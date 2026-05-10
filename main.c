// COMPILE WITH: gcc main.c -Iinclude -Llib -lsteal -lssl -lcrypto -o steal_client
#include <stdio.h>
#include <steallib.h>

int main() {
    const char* str = "https://www.wikipedia.org/";
    parsed_url_t* parsed = _parse_url(str);
    printf("Protocol: %s\nHost name: %s\nPort: %s\nPath: %s\n", parsed->protocol, parsed->host_name, parsed->port, parsed->path);
    _start_socket(parsed->host_name, parsed->port, parsed->path);
    _clear_parsed_url(parsed);
}