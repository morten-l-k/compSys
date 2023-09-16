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

enum file_type find_format(FILE* arg) {
    if (is_empty(arg)) {
        return EMPTY;
    } else if (is_ascii(arg)) {
        return ASCII;
    } else if (is_utf(arg)) {
        return UTF;
    } else if (is_iso(arg)) {
        return ISO;
    } else {
        return DATA;
    }
}

bool is_empty(FILE* arg) {
    if (fgetc(arg) == EOF) {
        return true;
    } else {
        return false;
    }   
}

bool is_ascii(FILE* arg) {
    int c;
    bool b = false;
    while ((c = fgetc(arg)) != EOF) {
        if((c < 7) || (c > 13 && c < 27) || (c > 27 && c < 32) || (c > 126)) {
            return false;
        } else {
            b = true;
        }
    }
    return b;
}
//TODO
//Problemer med iso-metode. Returnerer -1 ved fgetc(). Som om filformatet kun returnerer -1
bool is_iso(FILE* arg) {
    int c;
    bool b = false;
    while ((c = fgetc(arg)) != EOF) {
        if((c < 7) || (c > 13 && c < 27) || (c > 27 && c < 32) || (c > 127 && c < 160) || (c > 255)){
            return false;
        } else {
            b = true;
        }
    }
    return b;
}

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

const char * const FILE_TYPE_STRINGS[] = {
  "empty",
  "ASCII text",
  "UTF",
  "ISO",
  "data"
};

int main(int argc, char* argv[]) {
    int retval = EXIT_SUCCESS;
    if (argc != 2) {
        printf("One argument required only\n");
        retval = 1;
    } else {
        if (access(argv[1],F_OK) == 0) { 
            FILE* fp = open_file(argv[1]);
            enum file_type file = find_format(fp);
            printf("%s: %s\n",argv[1],FILE_TYPE_STRINGS[file]);
            fclose(fp);
            retval = 0;
        } else {
            printf("File does not exist");
            retval = 1;
        }
    }
    return retval;
}
