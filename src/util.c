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
    if (port == "80")
        _http_request(socket_fd, path, host_name);
    else if (port == "443")
        _https_request(socket_fd, path, host_name);

    // close the socket after all ops are done
    close(socket_fd);
}