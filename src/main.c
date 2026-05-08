#include <stdio.h>
#include <stdlib.h>
#include "../include/util.h"

int main() {
    const char* str = "https://www.wikipedia.org/";
    parsed_url_t* parsed = _parse_url(str);
    printf("Protocol: %s\nHost name: %s\nPort: %s\nPath: %s\n", parsed->protocol, parsed->host_name, parsed->port, parsed->path);
    _start_socket(parsed->host_name, parsed->port, parsed->path);

    free(parsed->protocol);
    free(parsed->host_name);
    free(parsed->path);
    free(parsed);
}