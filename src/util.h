#pragma once // only gets included once in final compilation

// common includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

/* STRUCTURES */
typedef struct {
    // https://example.com/api/data
    char* protocol;
    char* host_name;
    char* port;
    char* path;
} parsed_url_t;

// for passing to threads
struct request_args {
    int fd;
    char* path;
    char* host_name;
    const char* url;
};

/* FUNCTIONS */

// constructor for parsing url string 
parsed_url_t* _parse_url(const char*);
// destructor for cleanup
int _clear_parsed_url(parsed_url_t*);

// start socket, get fd, send http request
void _start_socket(char*, char*, char*, const char*);

// define http/https funcs in separate files
void _http_request(int, char*, char*, const char*);
void _https_request(int, char*, char*, const char*);