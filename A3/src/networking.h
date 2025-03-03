
#pragma once
#include "common.h"
#include "sha256.h"

// container for hasher - array of size SHA256_HASH_SIZE (i.e. 32). Array contains elements of
//type uint8_t. typedef statement syntax: typedef old-type-name new-type-name
typedef uint8_t hashdata_t[SHA256_HASH_SIZE];

// container for assembling password with salt
typedef struct PasswordAndSalt {
    char password[PASSWORD_LEN];
    char salt[SALT_LEN];
} PasswordAndSalt_t;

// container for assembling request message headers
typedef struct RequestHeader {
    char username[USERNAME_LEN];
    hashdata_t salted_and_hashed;
    uint32_t length;
} RequestHeader_t;

// container for assembling request messages
typedef struct Request {
    RequestHeader_t header;
    char payload[PATH_LEN];
} Request_t;
