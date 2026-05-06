#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h> // for inet_ntop()
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/util.h"

#define BACKLOG_LIMIT 20

parsed_url_t* _parse_url(const char* url) {
    parsed_url_t *temp = malloc(sizeof(parsed_url_t));
    temp->protocol = malloc(10 * sizeof(char)); // https + '\0' requires minimum 6 bytes
    temp->host_name = malloc(strlen(url) * sizeof(char));
    temp->path = malloc(strlen(url) * sizeof(char));

    char* protocol_temp = temp->protocol;
    char* host_name_temp = temp->host_name;
    char* path_temp = temp->path;
    // url parts: protocol (http/https), host (example.com), path (/api/data)
    int cnt = 0;

    while (*url != '\0') {
        if (*url == ':') {
            cnt++;
            url += 3; // move beyond the "//" and on to the 'e' in 'example.com'
            continue;
        }
        if (*url == '/' && cnt < 2) {
            // enter block only if we are parsing the host name
            cnt++;
        }
        switch(cnt) {
            case 0:
                *(protocol_temp++) = *url;
                break;
            case 1:
                *(host_name_temp++) = *url;
                break;
            case 2:
                *(path_temp++) = *url;
        }
        url++;
    }
    *protocol_temp = '\0';
    *host_name_temp = '\0';
    *path_temp = '\0';
    temp->port = !strcmp(temp->protocol, "https") ? "443" : "80";
    return temp;
}

void _http_request(int fd, char* path, char* host_name) {
    char* request = malloc(sizeof(char) * 1024); // 10 bits, arbitrary choice
    snprintf(request, 1024,
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "User-Agent: Mozilla/5.0...\r\n"
    "Accept: */*\r\n"
    "Connection: close\r\n\r\n", 
    path, host_name);

    // sending logic
    char* temp = request;
    int len = strlen(request), bytes_sent;

    // ensure whole string is sent, even if it's in parts
    while (len > 0) {
        bytes_sent = send(fd, request, len, 0);
        request += bytes_sent;
        len -= bytes_sent;
    }

    // reception logic
    // write to a constant buffer while simultaneously streaming the content on to a local file (disk streaming)
    char buffer[BUFSIZ];
    int bytes_received;

    FILE* f_ptr;

    // file location: ../tests/host_name.html
    char* temp_hn = host_name;
    strcat(temp_hn, ".html"); // file name with extension
    char* file_loc = malloc(sizeof(char) * (strlen(temp) + 15)); // ../tests/ is 9 chars but 15 for safety
    strcat(file_loc, "../tests/");
    strcat(file_loc, temp_hn);

    f_ptr = fopen(file_loc, "wb");
    if (!f_ptr) {
        fprintf(stderr, "error while opening file!");
        return;
    }

    while ((bytes_received = recv(fd, buffer, BUFSIZ, 0)) > 0) {
        fwrite(buffer, sizeof(char), bytes_received, f_ptr);
    }

    fclose(f_ptr);

    char* cleanup_call = malloc(sizeof(char) * (strlen(file_loc) + 19)); // "python cleanup.py " = 19
    
    sprintf(cleanup_call, "python cleanup.py %s", file_loc);

    // run script to clean up headers and format html
    system(cleanup_call);

    free(request);
    free(cleanup_call);
    free(file_loc);
}

void _start_socket(char* host_name, char* port, char* path) {
    struct addrinfo hints, *res, *p;
    int status, socket_fd;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // getaddrinfo() -> returns 0 for success
    if ((status = getaddrinfo(host_name, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        void* addr;
        char* ipver;
        struct sockaddr_in *ipv4;
        struct sockaddr_in6 *ipv6;

        if (p->ai_family == AF_INET) {
            // ipv4
            ipv4 = (struct sockaddr_in*)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else {
            // ipv6
            ipv6 = (struct sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf(" %s: %s\n", ipver, ipstr);
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd != -1) break;
    }
    
    if ((status = connect(socket_fd, p->ai_addr, p->ai_addrlen)) == -1) {
        fprintf(stderr, "could not connect to host!");
        return;
    }
    freeaddrinfo(res);

    // after connection, the client can send or receive once it is accepted
    // by the server
    _http_request(socket_fd, path, host_name); 

    // close the socket after all ops are done
    close(socket_fd);
}

/* testing funcs */
int main() {
    const char* str = "http://example.com/";
    parsed_url_t* parsed = _parse_url(str);
    printf("Protocol: %s\nHost name: %s\nPort: %s\nPath: %s\n", parsed->protocol, parsed->host_name, parsed->port, parsed->path);
    _start_socket(parsed->host_name, parsed->port, parsed->path);

    free(parsed->protocol);
    free(parsed->host_name);
    free(parsed->path);
    free(parsed);
}