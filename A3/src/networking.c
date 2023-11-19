#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#ifdef __APPLE__
#include "./endian.h"
#else
#include <endian.h>
#endif

#include "./networking.h"
#include "./sha256.h"
#include "networking.h"

char server_ip[IP_LEN];
char server_port[PORT_LEN];
char my_ip[IP_LEN];
char my_port[PORT_LEN];

//Define different response_header sections:
#define resp_len        4
#define resp_statCo     4
#define resp_blockNr    4
#define resp_blockCnt   4
#define resp_blockHsh   SHA256_HASH_SIZE
#define resp_totHsh     SHA256_HASH_SIZE



int c;

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
 * Combine a password and salt together and hash the result to form the 
 * 'signature'. The result should be written to the 'hash' variable. Note that 
 * as handed out, this function is never called. You will need to decide where 
 * it is sensible to do so.
 */
void get_signature(char* password, char* salt, hashdata_t* hash) {
    //Salting password
    char salted_password[strlen(password) + strlen(salt)];
    memcpy(salted_password,password,strlen(password));
    memcpy(salted_password + strlen(password),salt,strlen(salt));
    salted_password[strlen(password) + strlen(salt)] = '\0';

    //Hashing salted_password
    get_data_sha(salted_password,*hash,strlen(salted_password),SHA256_HASH_SIZE);
}

/*
 * Register a new user with a server by sending the username and signature to 
 * the server
 */
void register_user(char* username, char* password, char* user_salt) {
    //Creating socket and also making client request connection to server with compsy_helper
    int clientfd = compsys_helper_open_clientfd(server_ip,server_port);
    if (clientfd < 0) {
        fprintf(stderr, "Failed to connect to the server.\n");
        return;
    }

    //Getting signature by salting and hashing password
    hashdata_t hash; //Signature will be stored in hash
    get_signature(password,user_salt,&hash);
    
    //Copying data to struct RequestHeader_t
    RequestHeader_t request_header;    
    //Setting username
    memcpy(&(request_header.username),username, USERNAME_LEN);
   
    //Setting salted_and_hashed password
    memcpy(&(request_header.salted_and_hashed), hash, SHA256_HASH_SIZE);
    //setting request_header.length to 0, complying with protocol in terms of registration of new user
    request_header.length = 0;
    

    //Sending request_header to server 
    compsys_helper_writen(clientfd, &request_header, sizeof(request_header));
    
    //Declaring response buffer
    char resbuf[MAXBUF];
    
    //Reading response from server 
    compsys_helper_readn(clientfd, resbuf, sizeof(resbuf));
    
    // Declaring variables for each datasection as per protocol:
    char length[resp_len];
    char statusCode[resp_statCo];
    char blockNr[resp_blockNr];
    char blockCnt[resp_blockCnt];
    char blockHsh[resp_blockHsh];
    char totalHsh[resp_totHsh];
    

    // Initializing the variables above
    memcpy(length, resbuf, resp_len);
    memcpy(statusCode, resbuf + resp_len, resp_statCo);
    memcpy(blockNr, resbuf + resp_len + resp_statCo, resp_blockNr);
    memcpy(blockCnt, resbuf + resp_len + resp_statCo + resp_blockNr, resp_blockCnt);
    memcpy(blockHsh, resbuf + resp_len + resp_statCo + resp_blockNr + resp_blockCnt, resp_blockHsh);
    memcpy(totalHsh, resbuf + resp_len + resp_statCo + resp_blockNr + resp_blockCnt + resp_blockHsh, resp_totHsh);
    

    // The response itself
    uint32_t len;
    memcpy(&len, length, sizeof(uint32_t));
    len = OSSwapBigToHostConstInt32(len);
    char response[len];
    memcpy(response, resbuf + resp_len + resp_statCo + resp_blockNr + resp_blockCnt + resp_blockHsh + resp_totHsh, len);
    response[len] = '\0';

    // Ensuring with checksum that the response message has not been tampered with
    hashdata_t thisHash;
    get_data_sha(response, thisHash, len, SHA256_HASH_SIZE);    
    assert(memcmp(thisHash, totalHsh, SHA256_HASH_SIZE) == 0);


    //Checking the status code:
    // First converte:
    uint32_t status;
    memcpy(&status, statusCode, sizeof(uint32_t));
    status = OSSwapBigToHostInt32(status);
    // Then check:
    if(status == 1){
        printf("Got response: %s \n", response);
    }
    else {
        printf("Got unexpected status code: %d \n", status);
    }
    close(clientfd);
}

/*
 * Get a file from the server by sending the username and signature, along with
 * a file path. Note that this function should be able to deal with both small 
 * and large files. 
 */
void get_file(char* username, char* password, char* salt, char* to_get)
{

    int clientfd = compsys_helper_open_clientfd(server_ip,server_port);
    if (clientfd < 0) {
        fprintf(stderr, "Failed to connect to the server.\n");
        return;
    }

    //Getting signature by salting and hashing password
    hashdata_t hash; //Signature will be stored in hash
    get_signature(password,&salt,&hash);
    
    //Copying data to struct RequestHeader_t
    RequestHeader_t request_header;    
    //Setting username
    memcpy(&(request_header.username),username, USERNAME_LEN);
   
    //Setting salted_and_hashed password
    memcpy(&(request_header.salted_and_hashed), hash, SHA256_HASH_SIZE);

    request_header.length = strlen(to_get);
    Request_t request;
    request.header = request_header;
    memcpy(&(request.payload), to_get, PATH_LEN);

    printf("\n\n%s\n\n",request.payload);
    //Sending request_header to server 
    compsys_helper_writen(clientfd, &request, sizeof(request));

    //Declaring response buffer
    char resbuf[MAXBUF];
    
    //Reading response from server 
    compsys_helper_readn(clientfd, resbuf, sizeof(resbuf));
    
    // Declaring variables for each datasection as per protocol:
    char length[resp_len];
    char statusCode[resp_statCo];
    char blockNr[resp_blockNr];
    char blockCnt[resp_blockCnt];
    char blockHsh[resp_blockHsh];
    char totalHsh[resp_totHsh];
    

    // Initializing the variables above
    memcpy(length, resbuf, resp_len);
    memcpy(statusCode, resbuf + resp_len, resp_statCo);
    memcpy(blockNr, resbuf + resp_len + resp_statCo, resp_blockNr);
    memcpy(blockCnt, resbuf + resp_len + resp_statCo + resp_blockNr, resp_blockCnt);
    memcpy(blockHsh, resbuf + resp_len + resp_statCo + resp_blockNr + resp_blockCnt, resp_blockHsh);
    memcpy(totalHsh, resbuf + resp_len + resp_statCo + resp_blockNr + resp_blockCnt + resp_blockHsh, resp_totHsh);
    

    // The response itself
    uint32_t len;
    memcpy(&len, length, sizeof(uint32_t));
    len = OSSwapBigToHostConstInt32(len);
    char response[len];
    memcpy(response, resbuf + resp_len + resp_statCo + resp_blockNr + resp_blockCnt + resp_blockHsh + resp_totHsh, len);
    response[len] = '\0';
    printf(response);
}

void generate_salt(char salt[], int size) {
    for (int i=0; i<SALT_LEN; i++)
    {
        salt[i] = 'a' + (arc4random()% 26);
    }
}



void save_to_disc(char* username, char* salt) {
    // Create folder
    const char* folderName = "user_salts/";
    if (folderName != 0) {
        if (mkdir(folderName, 0777) != 0 && errno != EEXIST) {
        perror("Error creating folder");
        return;
        }
    }
    //Hvis folder eksistere, lig filerne ind
    char file_path[256];
    strcpy(file_path, folderName);
    strcat(file_path, username);
    strcat(file_path, ".salt");
    FILE *fptr;
    // create file with username as filename

    fptr = fopen(file_path, "w");
    if (fptr == NULL) {
        perror("Error creating file");
        return;
    }
    printf("SALT: %s \n",salt);
    fprintf(fptr, "%s\n", salt);
    fclose(fptr);
}
void find_file(char* username, char* salt, int buff_size) {
    // DIR* dir = opendir(directory);
    // if (dir == NULL) {
    //     perror("Could not open directory");
    //     return NULL;
    // }


    char file_path[256];
    strcpy(file_path, "user_salts/");
    strcat(file_path, username);
    strcat(file_path, ".salt");

    FILE *file = fopen(file_path, "r");
    if(file == NULL) {
        return;
    }

    fgets(salt, buff_size, file);
    // free(file_path);
    // free(file);
    fclose(file);
}



int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    } 

    // Read in configuration options. Should include a client_directory, 
    // client_ip, client_port, server_ip, and server_port
    char buffer[128];
    fprintf(stderr, "Got config path at: %s\n", argv[1]);
    FILE* fp = fopen(argv[1], "r");
    while (fgets(buffer, 128, fp)) {
        if (starts_with(buffer, CLIENT_IP)) {
            memcpy(my_ip, &buffer[strlen(CLIENT_IP)], 
                strcspn(buffer, "\r\n")-strlen(CLIENT_IP));
            if (!is_valid_ip(my_ip)) {
                fprintf(stderr, ">> Invalid client IP: %s\n", my_ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, CLIENT_PORT)) {
            memcpy(my_port, &buffer[strlen(CLIENT_PORT)], 
                strcspn(buffer, "\r\n")-strlen(CLIENT_PORT));
            if (!is_valid_port(my_port)) {
                fprintf(stderr, ">> Invalid client port: %s\n", my_port);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, SERVER_IP)) {
            memcpy(server_ip, &buffer[strlen(SERVER_IP)], 
                strcspn(buffer, "\r\n")-strlen(SERVER_IP));
            if (!is_valid_ip(server_ip)) {
                fprintf(stderr, ">> Invalid server IP: %s\n", server_ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, SERVER_PORT)) {
            memcpy(server_port, &buffer[strlen(SERVER_PORT)], 
                strcspn(buffer, "\r\n")-strlen(SERVER_PORT));
            if (!is_valid_port(server_port)) {
                fprintf(stderr, ">> Invalid server port: %s\n", server_port);
                exit(EXIT_FAILURE);
            }
        }        
    }
    fclose(fp);

    fprintf(stdout, "Client at: %s:%s\n", my_ip, my_port);
    fprintf(stdout, "Server at: %s:%s\n", server_ip, server_port);

    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
    char user_salt[SALT_LEN+1];
    
    fprintf(stdout, "Enter a username to proceed: ");
    scanf("%16s", username);
    while ((c = getchar()) != '\n' && c != EOF);
    printf("Username before: %lu \n", sizeof(username));
    // Clean up username string as otherwise some extra chars can sneak in.
    for (int i=strlen(username); i<USERNAME_LEN; i++)
    {
        username[i] = '\0';
    }

    printf("Username after: %lu \n", sizeof(username));
 
    fprintf(stdout, "Enter your password to proceed: ");
    scanf("%16s", password);
    while ((c = getchar()) != '\n' && c != EOF);
    // Clean up password string as otherwise some extra chars can sneak in.
    for (int i=strlen(password); i<PASSWORD_LEN; i++)
    {
        password[i] = '\0';
    }
    
    find_file(username, user_salt, SALT_LEN);
    if (user_salt[0] == '\0') {//user doesnt exist, create salt and save the user salt file, false means the user salt already exists on disc.
        generate_salt(user_salt, SALT_LEN);
        save_to_disc(username,user_salt);
    }
    // Users should call this script with a single argument describing what 
    // config to use
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    } 


    // Note that a random salt should be used, but you may find it easier to
    // repeatedly test the same user credentials by using the hard coded value
    // below instead, and commenting out this randomly generating section.


    fprintf(stdout, "Using salt: %s\n", user_salt);


    // The following function calls have been added as a structure to a 
    // potential solution demonstrating the core functionality. Feel free to 
    // add, remove or otherwise edit. Note that if you are creating a system 
    // for user-interaction the following lines will almost certainly need to 
    // be removed/altered.

    // Register the given user. As handed out, this line will run every time 
    // this client starts, and so should be removed if user interaction is 
    // added
    register_user(&username, &password, &user_salt);

    // Retrieve the smaller file, that doesn't not require support for blocks. 
    // As handed out, this line will run every time this client starts, and so 
    // should be removed if user interaction is added

    // get_file(username, password, user_salt, "tiny.txt");
    // printf("get_file_small\n");


    // Retrieve the larger file, that requires support for blocked messages. As
    // handed out, this line will run every time this client starts, and so 
    // should be removed if user interaction is added
    
    //get_file(username, password, user_salt, "hamlet.txt");

    exit(EXIT_SUCCESS);
}
