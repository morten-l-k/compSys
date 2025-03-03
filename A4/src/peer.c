#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#ifdef __APPLE__
#include "./endian.h"
#else
#include <endian.h>
#endif

#include "./peer.h"
#include "./sha256.h"


// Global variables to be used by both the server and client side of the peer.
// Some of these are not currently used but should be considered STRONG hints
PeerAddress_t *my_address;

pthread_mutex_t network_mutex = PTHREAD_MUTEX_INITIALIZER;
PeerAddress_t** network = NULL;
uint32_t peer_count = 0;

pthread_mutex_t retrieving_mutex = PTHREAD_MUTEX_INITIALIZER;
FilePath_t** retrieving_files = NULL;
uint32_t file_count = 0;


/*
 * Gets a sha256 hash of specified data, sourcedata. The hash itself is
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_data_sha(const char* sourcedata, hashdata_t hash, uint32_t data_size, 
    int hash_size)
{
  SHA256_CTX shactx;
  unsigned char shabuffer[hash_size];
  sha256_init(&shactx);
  sha256_update(&shactx, sourcedata, data_size);
  sha256_final(&shactx, shabuffer);

  for (int i=0; i<hash_size; i++)
  {
    hash[i] = shabuffer[i];
  }
}

/*
 * Gets a sha256 hash of specified data file, sourcefile. The hash itself is
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_file_sha(const char* sourcefile, hashdata_t hash, int size)
{
    int casc_file_size;

    FILE* fp = fopen(sourcefile, "rb");
    if (fp == 0)
    {
        printf("Failed to open source: %s\n", sourcefile);
        return;
    }

    fseek(fp, 0L, SEEK_END);
    casc_file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char buffer[casc_file_size];
    fread(buffer, casc_file_size, 1, fp);
    fclose(fp);

    get_data_sha(buffer, hash, casc_file_size, size);
}

/*
 * A simple min function, which apparently C doesn't have as standard
 */
uint32_t min(int a, int b)
{
    if (a < b) 
    {
        return a;
    }
    return b;
}

/*
 * Select a peer from the network at random, without picking the peer defined
 * in my_address
 */
void get_random_peer(PeerAddress_t* peer_address)
{ 
    PeerAddress_t** potential_peers = malloc(sizeof(PeerAddress_t*));
    uint32_t potential_count = 0; 
    // printf("PEER IN NETWORK: %s, %s, \n",network[0]->ip,my_address->ip);
    for (uint32_t i=0; i<peer_count; i++)
    {
        if (strcmp(network[i]->ip, my_address->ip) != 0 
                || strcmp(network[i]->port, my_address->port) != 0 )
        {
            potential_peers = realloc(potential_peers, 
                (potential_count+1) * sizeof(PeerAddress_t*));
            potential_peers[potential_count] = network[i];
            potential_count++;
        }
    }

    if (potential_count == 0)
    {
        printf("No peers to connect to. You probably have not implemented "
            "registering with the network yet.\n");
    }

    uint32_t random_peer_index = rand() % potential_count;

    memcpy(peer_address->ip, potential_peers[random_peer_index]->ip, IP_LEN);
    memcpy(peer_address->port, potential_peers[random_peer_index]->port, 
        PORT_LEN);

    free(potential_peers);

    printf("Selected random peer: %s:%s\n", 
        peer_address->ip, peer_address->port);
}

/*
 * Send a request message to another peer on the network. Unless this is 
 * specifically an 'inform' message as described in the assignment handout, a 
 * reply will always be expected.
 */
void send_message(PeerAddress_t peer_address, int command, char* request_body)
{
    fprintf(stdout, "Connecting to server at %s:%s to run command %d (%s)\n", 
        peer_address.ip, peer_address.port, command, request_body);

    compsys_helper_state_t state;
    char msg_buf[MAX_MSG_LEN];
    FILE* fp;

    // Setup the eventual output file path. This is being done early so if 
    // something does go wrong at this stage we can avoid all that pesky 
    // networking
    char output_file_path[strlen(request_body)+1];
    if (command == COMMAND_RETREIVE)
    {     
        strcpy(output_file_path, request_body);

        if (access(output_file_path, F_OK ) != 0 ) 
        {
            fp = fopen(output_file_path, "a");
            fclose(fp);
        }
    }

    // Setup connection
    int peer_socket = compsys_helper_open_clientfd(peer_address.ip, peer_address.port);
    compsys_helper_readinitb(&state, peer_socket);

    // Construct a request message and send it to the peer
    struct RequestHeader request_header;
    strncpy(request_header.ip, my_address->ip, IP_LEN);
    request_header.port = htonl(atoi(my_address->port));
    request_header.command = htonl(command);
    request_header.length = htonl(strlen(request_body));

    memcpy(msg_buf, &request_header, REQUEST_HEADER_LEN);
    memcpy(msg_buf+REQUEST_HEADER_LEN, request_body, strlen(request_body));

    compsys_helper_writen(peer_socket, msg_buf, REQUEST_HEADER_LEN+strlen(request_body));

    // We don't expect replies to inform messages so we're done here
    if (command == COMMAND_INFORM)
    {
        return;
    }

    // Read a reply
    compsys_helper_readnb(&state, msg_buf, REPLY_HEADER_LEN);

    // Extract the reply header 
    char reply_header[REPLY_HEADER_LEN];
    memcpy(reply_header, msg_buf, REPLY_HEADER_LEN);

    uint32_t reply_length = ntohl(*(uint32_t*)&reply_header[0]);
    uint32_t reply_status = ntohl(*(uint32_t*)&reply_header[4]);
    uint32_t this_block = ntohl(*(uint32_t*)&reply_header[8]);
    uint32_t block_count = ntohl(*(uint32_t*)&reply_header[12]);
    hashdata_t block_hash;
    memcpy(block_hash, &reply_header[16], SHA256_HASH_SIZE);
    hashdata_t total_hash;
    memcpy(total_hash, &reply_header[48], SHA256_HASH_SIZE);

    // Determine how many blocks we are about to recieve
    hashdata_t ref_hash;
    memcpy(ref_hash, &total_hash, SHA256_HASH_SIZE);
    uint32_t ref_count = block_count;

    // Loop until all blocks have been recieved
    for (uint32_t b=0; b<ref_count; b++)
    {
        // Don't need to re-read the first block
        if (b > 0)
        {
            // Read the response
            compsys_helper_readnb(&state, msg_buf, REPLY_HEADER_LEN);

            // Read header
            memcpy(reply_header, msg_buf, REPLY_HEADER_LEN);

            // Parse the attributes
            reply_length = ntohl(*(uint32_t*)&reply_header[0]);
            reply_status = ntohl(*(uint32_t*)&reply_header[4]);
            this_block = ntohl(*(uint32_t*)&reply_header[8]);
            block_count = ntohl(*(uint32_t*)&reply_header[12]);

            memcpy(block_hash, &reply_header[16], SHA256_HASH_SIZE);
            memcpy(total_hash, &reply_header[48], SHA256_HASH_SIZE);

            // Check we're getting consistent results
            if (ref_count != block_count)
            {
                fprintf(stdout, 
                    "Got inconsistent block counts between blocks\n");
                close(peer_socket);
                return;
            }

            for (int i=0; i<SHA256_HASH_SIZE; i++)
            {
                if (ref_hash[i] != total_hash[i])
                {
                    fprintf(stdout, 
                        "Got inconsistent total hashes between blocks\n");
                    close(peer_socket);
                    return;
                }
            }
        }

        // Check response status
        if (reply_status != STATUS_OK)
        {
            if (command == COMMAND_REGISTER && reply_status == STATUS_PEER_EXISTS)
            {
                printf("Peer already exists\n");
            }
            else
            {
                printf("Got unexpected status %d\n", reply_status);
                close(peer_socket);
                return;
            }
        }

        // Read the payload
        char payload[reply_length+1];
        compsys_helper_readnb(&state, msg_buf, reply_length);
        memcpy(payload, msg_buf, reply_length);
        payload[reply_length] = '\0';
        
        // Check the hash of the data is as expected
        hashdata_t payload_hash;
        get_data_sha(payload, payload_hash, reply_length, SHA256_HASH_SIZE);

        for (int i=0; i<SHA256_HASH_SIZE; i++)
        {
            if (payload_hash[i] != block_hash[i])
            {
                fprintf(stdout, "Payload hash does not match specified\n");
                close(peer_socket);
                return;
            }
        }

        // If we're trying to get a file, actually write that file
        if (command == COMMAND_RETREIVE)
        {
            // Check we can access the output file
            fp = fopen(output_file_path, "r+b");
            if (fp == 0)
            {
                printf("Failed to open destination: %s\n", output_file_path);
                close(peer_socket);
            }

            uint32_t offset = this_block * (MAX_MSG_LEN-REPLY_HEADER_LEN);
            fprintf(stdout, "Block num: %d/%d (offset: %d)\n", this_block+1, 
                block_count, offset);
            fprintf(stdout, "Writing from %d to %d\n", offset, 
                offset+reply_length);

            // Write data to the output file, at the appropriate place
            fseek(fp, offset, SEEK_SET);
            fputs(payload, fp);
            fclose(fp);
        }
    }

    // Confirm that our file is indeed correct
    if (command == COMMAND_RETREIVE)
    {
        fprintf(stdout, "Got data and wrote to %s\n", output_file_path);

        // Finally, check that the hash of all the data is as expected
        hashdata_t file_hash;
        get_file_sha(output_file_path, file_hash, SHA256_HASH_SIZE);

        for (int i=0; i<SHA256_HASH_SIZE; i++)
        {
            if (file_hash[i] != total_hash[i])
            {
                fprintf(stdout, "File hash does not match specified for %s\n", 
                    output_file_path);
                close(peer_socket);
                return;
            }
        }
    }

    // If we are registering with the network we should note the complete 
    // network reply
    char* reply_body = malloc(reply_length + 1);
    memset(reply_body, 0, reply_length + 1);
    memcpy(reply_body, msg_buf, reply_length);

    if (reply_status == STATUS_OK)
    {
        if (command == COMMAND_REGISTER) { 
            //Reallocating memory for network to contain one more pointer                   
            int peers_in_network = 1 + (reply_length / sizeof(NetworkAddress_t));
            network = realloc(network,(size_t)(peers_in_network * sizeof(PeerAddress_t*)));
            
            // for (int i = 0; i < peers_in_network; i++) {
            //     //Constructing new peer to be added to own network
            //     PeerAddress_t new_peer;
            //     memset(new_peer.ip,'\0',IP_LEN);
            //     memset(new_peer.port,'\0',PORT_LEN);

            //     memcpy(new_peer.ip,reply_body + (i * sizeof(NetworkAddress_t)), IP_LEN);

            //     uint32_t *tmp = \
            //         ntohl(*(uint32_t*)&reply_body[IP_LEN + (i * sizeof(NetworkAddress_t))]);
            //     sprintf(new_peer.port,"%d",tmp);

            //     pthread_mutex_lock(&network_mutex);
            //     //Allocating space for new peer to be added
            //     network[i + 1] = malloc(sizeof(PeerAddress_t));

            //     //Copying new peer to network
            //     memcpy(network[i + 1],&new_peer,sizeof(PeerAddress_t));
            //     peer_count++;
            //     pthread_mutex_unlock(&network_mutex);
            // }
            
            // Constructing new peer to be added to network
            PeerAddress_t new_peer;
            memset(new_peer.ip,'\0',IP_LEN);
            memset(new_peer.port,'\0',PORT_LEN);
            
            memcpy(new_peer.ip,reply_body,IP_LEN);

            //Converting port bytes from network to host order
            uint32_t *tmp = ntohl(*(uint32_t*)&reply_body[IP_LEN]);
            sprintf(new_peer.port,"%d",tmp);

            pthread_mutex_lock(&network_mutex);
            //Allocating space for new peer to be added
            network[peer_count] = malloc(sizeof(PeerAddress_t));

            //Copying new peer to network
            memcpy(network[peer_count],&new_peer,sizeof(PeerAddress_t));
            peer_count++;
            pthread_mutex_unlock(&network_mutex);
        }
    } 
    else
    {
        printf("Got response code: %d, %s\n", reply_status, reply_body);
    }
    free(reply_body);
    close(peer_socket);
}


/*
 * Function to act as thread for all required client interactions. This thread 
 * will be run concurrently with the server_thread but is finite in nature.
 * 
 * This is just to register with a network, then download two files from a 
 * random peer on that network. As in A3, you are allowed to use a more 
 * user-friendly setup with user interaction for what files to retrieve if 
 * preferred, this is merely presented as a convienient setup for meeting the 
 * assignment tasks
 */ 
void* client_thread(void* thread_args)
{
    struct PeerAddress *peer_address = thread_args;

    // Register the given user
    send_message(*peer_address, COMMAND_REGISTER, "\0");

    // Update peer_address with random peer from network
    get_random_peer(peer_address);


    // Retrieve the smaller file, that doesn't not require support for blocks
    send_message(*peer_address, COMMAND_RETREIVE, "tiny.txt");

    // Update peer_address with random peer from network
    get_random_peer(peer_address);

    // Retrieve the larger file, that requires support for blocked messages
    send_message(*peer_address, COMMAND_RETREIVE, "hamlet.txt");

    return NULL;
}

/*
 * Handle any 'register' type requests, as defined in the asignment text. This
 * should always generate a response.
 */
void handle_register(int connfd, char* client_ip, int client_port_int)
{    
    //SEND NETWORK LIST TO PEER (1/2)
    //Creating reply header for response

    ReplyHeader_t reply;
    reply.length = htobe32(peer_count * sizeof(NetworkAddress_t));
    reply.status = htobe32(1); //htobe32 converts from host/little-endian to big-endian/network order
    reply.this_block = 1;
    reply.block_count = reply.length < (MAX_MSG_LEN - REPLY_HEADER_LEN) ? 1 : 2;
    reply.block_hash;
    reply.total_hash;


    NetworkAddress_t reply_body[peer_count];



    //Creating reply body for response
    for (u_int32_t i = 0; i < peer_count; i++) {
        NetworkAddress_t peer_i;

        //Aquiring lock so other thread can't access network list
        pthread_mutex_lock(&network_mutex);
        memcpy(&peer_i.ip,network[i]->ip,IP_LEN);
        char *endptr;
        uint32_t u_port = htobe32(strtol(&network[i]->port,&endptr,10));
        pthread_mutex_unlock(&network_mutex);

        memcpy(&peer_i.port,&u_port,sizeof(uint32_t));

        memcpy(&reply_body[i].ip,&peer_i.ip,IP_LEN);
        reply_body[i].port = peer_i.port;
    }


    hashdata_t hash;
    
    //Converting reply_body to char* array
    char body_src[ntohl(reply.length)];
    memcpy(&body_src,&reply_body,ntohl(reply.length));

    get_data_sha(body_src,hash,sizeof(body_src),SHA256_HASH_SIZE);
    
    memcpy(&reply.block_hash,&hash,sizeof(hashdata_t));
    memcpy(&reply.total_hash,&hash,sizeof(hashdata_t));

    //Assemble reply header and reply body
    char* total_reply[REPLY_HEADER_LEN + ntohl(reply.length)];
    memcpy(total_reply,&reply,REPLY_HEADER_LEN);
    memcpy(total_reply + REPLY_HEADER_LEN,&reply_body,ntohl(reply.length));
    
    //Send reply to client
    compsys_helper_writen(connfd,total_reply,ntohl(reply.length) + REPLY_HEADER_LEN);


    //ADD PEER TO NETWORK (2/2)
    //Reallocating new memory to update own network list
    network = realloc(network,(size_t)(peer_count+1) * sizeof(PeerAddress_t*));
    network[peer_count] = malloc(sizeof(PeerAddress_t));

    PeerAddress_t new_peer;
    memcpy(new_peer.ip,client_ip,IP_LEN);
    sprintf(new_peer.port,"%d",client_port_int);

    //Copying new_peer to network
    pthread_mutex_lock(&network_mutex);
    memcpy(network[peer_count],&new_peer,sizeof(PeerAddress_t));
    peer_count++;
    pthread_mutex_unlock(&network_mutex);
}

/*
 * Handle 'inform' type message as defined by the assignment text. These will 
 * never generate a response, even in the case of errors.
 */
void handle_inform(char* request)
{
    
    //Reallocating memory for network to contain one more pointer       
    network = realloc(network,(size_t)((peer_count+1) * sizeof(PeerAddress_t*)));

    //Constructing new peer to be added to network
    PeerAddress_t new_peer;
    memset(new_peer.ip,'\0',IP_LEN);
    memset(new_peer.port,'\0',PORT_LEN);
    
    memcpy(new_peer.ip,request,IP_LEN);

    //Converting port bytes from network to host order
    uint32_t *tmp = ntohl(*(uint32_t*)&request[IP_LEN]);
    sprintf(new_peer.port,"%d",tmp);

    //Allocating space for new peer to be added
    network[peer_count] = malloc(sizeof(PeerAddress_t));

    //Updating peer count
    pthread_mutex_lock(&network_mutex);
    memcpy(network[peer_count],&new_peer,sizeof(PeerAddress_t));
    peer_count++;
    pthread_mutex_unlock(&network_mutex);
}

/*
 * Handle 'retrieve' type messages as defined by the assignment text. This will
 * always generate a response
 */
void handle_retreive(int connfd, char* request)
{

    char msg_buf[MAX_MSG_LEN];
    FILE* fp;

    if (access(request, F_OK) != 0)
    {
        char* response = "Could not retrive file";
        send_message_reponse(connfd, STATUS_BAD_REQUEST, response);
        return;    
    }
    //Vi læser request fra client
    fp = fopen(request, "rb");
    if (fp == NULL) {
        perror("Error reading file");
        return;
    }
    //Læs filens størrelse
    fseek(fp,0,SEEK_END);
    long file_size = ftell(fp);
    fseek(fp,0,SEEK_SET);
    //Alloker memory
    char* response = malloc(file_size);
    if (response == NULL)
    {
        perror("Error allocating memory");
        fclose(fp);
        return;
    }
    
    size_t read_bytes = fread(response,1,file_size,fp);
    if (read_bytes != file_size)
    {
        perror("Error reading file");
        fclose(fp);
        free(response);
        return;
    }

    send_message_reponse(connfd,STATUS_OK, response);
    fclose(fp);
    free(response);
}

//Message to clinet
void send_message_reponse(int connfd, int status, char *data) {
    compsys_helper_state_t state;

    // Setup connection
    compsys_helper_readinitb(&state, connfd);

    // Beregn hvor mange blokke skal sendes
    int block_count = sizeof(data)/MAX_MSG_LEN+1; //Vi +1, da vi runder ned.
    for (size_t i = 0; i < block_count; i++)
    {
        int from_index = i * MAX_MSG_LEN;
        int to_index = sizeof(data)-from_index < MAX_MSG_LEN
            ? sizeof(data)%MAX_MSG_LEN
            : (i+1) * MAX_MSG_LEN;
 
        // Construct a request message and send it to the peer
        struct ReplyHeader reply_header;
        reply_header.length = to_index-from_index;
        reply_header.status = htonl(status);
        reply_header.this_block = i; // Hvilekn block er vi kommet til
        reply_header.block_count = block_count;
        compsys_helper_writen(connfd, data, REQUEST_HEADER_LEN+ reply_header.length);

    }
}

/*
 * Handler for all server requests. This will call the relevent function based 
 * on the parsed command code
 */
void handle_server_request(int connfd)
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code
    compsys_helper_state_t state;

    //Associate state buffer with descriptor
    compsys_helper_readinitb(&state,connfd);

    //Reading request to buffer
    char msg_buf[MAX_MSG_LEN];
    compsys_helper_readnb(&state,&msg_buf,REQUEST_HEADER_LEN);

    //Unpacking buffered data into variables
    char request_header[REQUEST_HEADER_LEN];

    memcpy(&request_header,msg_buf,REQUEST_HEADER_LEN);

    char IP_address[16];
    memcpy(&IP_address,&request_header,IP_LEN);
    
    uint32_t port = ntohl(*(uint32_t*)&request_header[16]);
    uint32_t command_code = ntohl(*(uint32_t*)&request_header[20]);
    uint32_t length = ntohl(*(uint32_t*)&request_header[24]);
    
    if (command_code == COMMAND_REGISTER) {
        handle_register(connfd,&IP_address,port);
    }

    if (command_code == COMMAND_RETREIVE) {
        char* request = malloc(length);
        memcpy(request, &state.compsys_helper_buf[REQUEST_HEADER_LEN], length);
        handle_retreive(connfd, request);
       
    }

    if(command_code == COMMAND_INFORM) {
        char* request = malloc(sizeof(PeerAddress_t));
        memcpy(request, &state.compsys_helper_buf[REQUEST_HEADER_LEN], sizeof(PeerAddress_t));
        handle_inform(request);
    }
    }

/*
 * Function to act as basis for running the server thread. This thread will be
 * run concurrently with the client thread, but is infinite in nature.
 */
void* server_thread()
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code 
        
    int socket_fd = compsys_helper_open_listenfd(my_address->port);    
    
    if (socket_fd < 0) {
        fprintf(stderr, "Error: %d in server_thread\n", errno);
    }

    while (1) {
        //Accept redirects clients to a new socket, so new clients can keep conneting to main socket
        struct sockaddr *new_address;
        struct socklen_t *sock_len;

        printf("Waiting for peer to connect\n");
        int connfd = accept(socket_fd,new_address,sock_len);

        if(connfd < 0) {
            fprintf(stderr, "Error: %d in server_thread when creating new socket\n", errno);
        }
        printf("HERE1\n");
        handle_server_request(connfd);
    }
}


int main(int argc, char **argv)
{
    // Initialise with known junk values, so we can test if these were actually
    // present in the config or not
    struct PeerAddress peer_address;
    memset(peer_address.ip, '\0', IP_LEN);
    memset(peer_address.port, '\0', PORT_LEN);
    memcpy(peer_address.ip, "x", 1);
    memcpy(peer_address.port, "x", 1);

    // Users should call this script with a single argument describing what 
    // config to use
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    } 

    my_address = (PeerAddress_t*)malloc(sizeof(PeerAddress_t));
    memset(my_address->ip, '\0', IP_LEN);
    memset(my_address->port, '\0', PORT_LEN);

    // Read in configuration options. Should include a client_ip, client_port, 
    // server_ip, and server_port
    char buffer[128];
    fprintf(stderr, "Got config path at: %s\n", argv[1]);
    FILE* fp = fopen(argv[1], "r");
    while (fgets(buffer, 128, fp)) {
        if (starts_with(buffer, MY_IP)) {
            memcpy(&my_address->ip, &buffer[strlen(MY_IP)], 
                strcspn(buffer, "\r\n")-strlen(MY_IP));
            if (!is_valid_ip(my_address->ip)) {
                fprintf(stderr, ">> Invalid client IP: %s\n", my_address->ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, MY_PORT)) {
            memcpy(&my_address->port, &buffer[strlen(MY_PORT)], 
                strcspn(buffer, "\r\n")-strlen(MY_PORT));
            if (!is_valid_port(my_address->port)) {
                fprintf(stderr, ">> Invalid client port: %s\n", 
                    my_address->port);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, PEER_IP)) {
            memcpy(peer_address.ip, &buffer[strlen(PEER_IP)], 
                strcspn(buffer, "\r\n")-strlen(PEER_IP));
            if (!is_valid_ip(peer_address.ip)) {
                fprintf(stderr, ">> Invalid peer IP: %s\n", peer_address.ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, PEER_PORT)) {
            memcpy(peer_address.port, &buffer[strlen(PEER_PORT)], 
                strcspn(buffer, "\r\n")-strlen(PEER_PORT));
            if (!is_valid_port(peer_address.port)) {
                fprintf(stderr, ">> Invalid peer port: %s\n", 
                    peer_address.port);
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(fp);

    retrieving_files = malloc(file_count * sizeof(FilePath_t*));
    srand(time(0));

    network = malloc(sizeof(PeerAddress_t*));
    network[0] = my_address;
    peer_count = 1;

    // Setup the client and server threads 
    pthread_t client_thread_id;
    pthread_t server_thread_id;
    if (peer_address.ip[0] != 'x' && peer_address.port[0] != 'x')
    {   
        pthread_create(&client_thread_id, NULL, client_thread, &peer_address);
    } 
    pthread_create(&server_thread_id, NULL, server_thread, NULL);

    // Start the threads. Note that the client is only started if a peer is 
    // provided in the config. If none is we will assume this peer is the first
    // on the network and so cannot act as a client.
    if (peer_address.ip[0] != 'x' && peer_address.port[0] != 'x')
    {
        pthread_join(client_thread_id, NULL);
    }
    pthread_join(server_thread_id, NULL);

    exit(EXIT_SUCCESS);
}