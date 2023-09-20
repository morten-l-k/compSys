#include <stdio.h> //standard in out
#include <unistd.h>
#include <stdlib.h> //standard library
#include <string.h> //string
#include <errno.h>  //errors
#include <stdbool.h> //include boolean data type

//Function declaration
bool is_empty(FILE* arg);
bool is_ascii(FILE* arg);
bool is_utf(FILE* arg);
bool is_iso(FILE* arg);

enum file_type {
    EMPTY,
    ASCII,
    UTF,
    ISO,
    DATA
};

/**
 * @brief Function opens file on path and returns pointer to filestream
 * 
 * @param arg character array
 * @return FILE* 
 */
FILE* open_file(char arg[]) {
    char* mode = "r";
    FILE* fp = fopen(arg,mode);
    return fp;
}

/**
 * @brief Functions takes a file stream pointer and loops through different file formats
 * 
 * @param arg pointer of FILE type
 * @return enum file_type 
 */
enum file_type find_format(FILE* arg) {
    if (is_empty(arg)) {
        return EMPTY;
    } else if (is_ascii(arg)) {
        return ASCII;
    } else if (is_iso(arg)) {
        return ISO;
    } else if (is_utf(arg)) {
        return UTF;
    } else {
        return DATA;
    }
}

/**
 * @brief Takes a pointer of FILE type and check if there is a character in file
 * 
 * @param arg 
 * @return true 
 * @return false 
 */
bool is_empty(FILE* arg) {
    if (fgetc(arg) == EOF) {
        return true;
    } else {
        fseek(arg, 0, SEEK_SET);
        return false;
    }   
}

/**
 * @brief Takes a pointer of FILE type and checks if characters are within set of ascii characters
 * 
 * @param arg 
 * @return true 
 * @return false 
 */
bool is_ascii(FILE* arg) {
    bool returnvalue = true; 
    int c; 
    while((c = fgetc(arg)) != EOF) {
        if((c >= 0x07 && c <= 0x0D) || c == 0x1B || (c >= 0x20 && c <= 0x7E)) {
            // Do nothing
            //printf("Here: %d\n",c);
        }
        else {
            returnvalue = false; 
            fseek(arg, 0, SEEK_SET);
            break; 
        }
    }
    return returnvalue;
}

/**
 * @brief Takes a pointer of FILE type and checks if characters in file is within range of iso
 * characters
 * 
 * @param arg 
 * @return true 
 * @return false 
 */
bool is_iso(FILE* arg) {
    int c;
    bool b = false;
    while ((c = fgetc(arg)) != EOF) {
        if((c < 7) || (c > 13 && c < 27) || (c > 27 && c < 32) || (c > 127 && c < 160) || (c > 255)){
            fseek(arg, 0, SEEK_SET);
            return false;
        } else {
            b = true;
        }
    }
    return b;
}

/**
 * @brief Takes a pointer of FILE type and checks if characters are within UTF-range
 * 
 * @param arg 
 * @return true 
 * @return false 
 */
bool is_utf(FILE*arg) {
    int c;
    bool b = false;
    while((c = fgetc(arg)) != EOF) {
        if ((c > 255 && c <= 1114111) || (c > 127 && c < 160)) {
            return true;
        } else {
            b = false;
        }
    }
    return b;
}

/**
 * @brief array of char pointers
 * 
 */
const char * const FILE_TYPE_STRINGS[] = {
  "empty",
  "ASCII text",
  "UTF",
  "ISO-8859 text",
  "data"
};

/**
 * @brief Main function for the program
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char* argv[]) {
    int retval = EXIT_SUCCESS;
    if (argc != 2) {
        printf("One argument required only\n");
        retval = EXIT_FAILURE;
    } else {
        if (access(argv[1],F_OK) == 0) { 
            FILE* fp = open_file(argv[1]);
            enum file_type file = find_format(fp);
            printf("%s: %s\n",argv[1],FILE_TYPE_STRINGS[file]);
            fclose(fp);
            retval = EXIT_SUCCESS;
        } else {
            printf("File does not exist");
            retval = EXIT_FAILURE;
        }
    }
    return retval;
}
