#include <stdio.h> //standard in out
#include <unistd.h>
#include <stdlib.h>  //standard library
#include <string.h>  //string
#include <errno.h>   //errors
#include <stdbool.h> //include boolean data type

// Function declaration
bool is_empty(int c);
bool is_ascii(int c);
bool is_utf(int c);
bool is_iso(int c);

enum file_type
{
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
FILE *open_file(char arg[])
{
    char *mode = "r";
    FILE *fp = fopen(arg, mode);
    return fp;
}

/**
 * @brief Functions takes a file stream pointer and loops through different file formats
 *
 * @param arg pointer of FILE type
 * @return enum file_type
 */
enum file_type find_format(FILE *arg)
{
    int c = fgetc(arg);
    if(is_empty(c))
        return EMPTY;
    bool isAscii = true;
    bool isUtf = true;
    bool isISO = true;

    while (c != EOF)
    {
        if(isAscii)
            isAscii = is_ascii(c);
        if(isUtf)
            isUtf = is_utf(c);
        if(isISO)
            isISO = is_iso(c);
        c = fgetc(arg);
    }

    if (isAscii)
    {
        return ASCII;
    }
    else if (isUtf)
    {
        return UTF;
    }
    else if (isISO)
    {
        return ISO;
    }
    else
    {
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
bool is_empty(int c)
{
    return c == EOF;
}

/**
 * @brief Takes a pointer of FILE type and checks if characters are within set of ascii characters
 *
 * @param arg
 * @return true
 * @return false
 */
bool is_ascii(int c)
{
    return ((c >= 0x07 && c <= 0x0D) || c == 0x1B || (c >= 0x20 && c <= 0x7E));
}

/**
 * @brief Takes a pointer of FILE type and checks if characters in file is within range of iso
 * characters
 *
 * @param arg
 * @return true
 * @return false
 */
bool is_iso(int c)
{
    return !((c < 7) || (c > 13 && c < 27) || (c > 27 && c < 32) || (c > 127 && c < 160) || (c > 255));
}

/**
 * @brief Takes a pointer of FILE type and checks if characters are within UTF-range
 *
 * @param arg
 * @return true
 * @return false
 */
bool is_utf(int c)
{
    return ((c > 255 && c <= 1114111) || (c > 127 && c < 160));
}

/**
 * @brief array of char pointers
 *
 */
const char *const FILE_TYPE_STRINGS[] = {
    "empty",
    "ASCII text",
    "UTF",
    "ISO-8859 text",
    "data"};

/**
 * @brief Main function for the program
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    int retval = EXIT_SUCCESS;
    if (argc != 2)
    {
        printf("One argument required only\n");
        retval = EXIT_FAILURE;
    }
    else
    {
        if (access(argv[1], F_OK) == 0)
        {
            FILE *fp = open_file(argv[1]);
            enum file_type file = find_format(fp);
            printf("%s: %s\n", argv[1], FILE_TYPE_STRINGS[file]);
            fclose(fp);
            retval = EXIT_SUCCESS;
        }
        else
        {
            printf("File does not exist");
            retval = EXIT_FAILURE;
        }
    }
    return retval;
}
