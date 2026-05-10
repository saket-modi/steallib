#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "util.h"

char* _make_request(char* path, char* host_name) {
    char* request = malloc(sizeof(char) * 1024); // 1024 bytes, arbitrary
    snprintf(request, 1024,
    "GET %s HTTP/1.0\r\n" // HTTPS/1.1 uses chunked encoding, might deal with that later
    "Host: %s\r\n"
    "User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows; U; Windows NT 6.2; Win64; x64 Trident/5.0)\r\n"
    "Accept: */*\r\n"
    "Connection: close\r\n\r\n", 
    path, host_name);

    return request;
}

char* _get_file_loc(char* host_name) {
    char* file_loc = malloc(sizeof(char) * (strlen(host_name) + 15)); // ../tests/ is 9 chars but 15 for safety
    sprintf(file_loc, "./tests/%s.html", host_name);
    return file_loc;
}

void _clean_output(char* file_loc) {
    char* cleanup_call = malloc(sizeof(char) * (strlen(file_loc) + 19)); // "python cleanup.py " = 19
    
    sprintf(cleanup_call, "python src/cleanup.py %s", file_loc);

    // run script to clean up headers and format html
    system(cleanup_call);

    free(cleanup_call);
}

void _http_request(int fd, char* path, char* host_name) {
    // request
    char* request = _make_request(path, host_name);

    // sending logic
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
    char* file_loc = _get_file_loc(host_name);

    f_ptr = fopen(file_loc, "wb");
    if (!f_ptr) {
        fprintf(stderr, "error while opening file!");
        return;
    }

    while ((bytes_received = recv(fd, buffer, BUFSIZ, 0)) > 0) {
        fwrite(buffer, sizeof(char), bytes_received, f_ptr);
    }

    fclose(f_ptr);

    _clean_output(file_loc);

    free(request);
    free(file_loc);
}

void _https_request(int fd, char* path, char* host_name) {

    // temp solution to rerouting
    // char* temp_hn = malloc(sizeof(char) * (strlen(host_name) + 20));
    // sprintf(temp_hn, "www.%s", host_name);
    // host_name = temp_hn;

    /* ssl context setup */
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "ssl context could not be set!");
        return;
    }

    // verify the server's certificate; change the final argument to specify a different cert store apart from the one obtained by running "openssl version -d"
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

    if (!SSL_CTX_set_default_verify_paths(ctx)) {
        fprintf(stderr, "couldn't set the default certificate store!");
        return;
    }

    if (!SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION)) {
        fprintf(stderr, "couldn't set minimum TLS version to 1.2");
        return;
    }

    /* create ssl object and bio it to the socket fd */
    SSL* ssl_o = SSL_new(ctx);
    if (!ssl_o) {
        fprintf(stderr, "couldn't create new ssl object!");
        return;
    }

    BIO* bio;
    bio = BIO_new(BIO_s_socket());
    if (!bio) {
        fprintf(stderr, "could not declare bio!");
        return;
    }

    BIO_set_fd(bio, fd, BIO_NOCLOSE);

    // gives ownership of bio to the ssl_o object; only need to free ssl_o now, and bio will be freed alongside it
    SSL_set_bio(ssl_o, bio, bio);

    // a server may support multiple hosts, thus it is required to specify which host name we wish to connect to
    if (!SSL_set_tlsext_host_name(ssl_o, host_name)) {
        fprintf(stderr, "couldn't set host name for initial ClientHello!");
        return;
    }

    // now the app must verify the certificate returned by the server to belong to that host name
    if (!SSL_set1_host(ssl_o, host_name)) {
        fprintf(stderr, "couldn't set host name to verify from the server-returned certificate!");
        return;
    }

    // connect to the server
    if (SSL_connect(ssl_o) < 1) {
        fprintf(stderr, "failed to connect to the server!");

        // get more information abt the failure if it is from a verification error
        // X509_OK represents a successful cert verif
        if (SSL_get_verify_result(ssl_o) != X509_V_OK) {
            printf("Verify error: %s\n", X509_verify_cert_error_string(SSL_get_verify_result(ssl_o)));
        }

        return;
    }

    /* write http request to the server now */
    // request
    char* request = _make_request(path, host_name);

    // send request
    size_t written;

    // sends the whole request; no loop required unlike http
    if (!SSL_write_ex(ssl_o, request, strlen(request), &written)) {
        fprintf(stderr, "failed to write ssl request to server!");
    }

    // receive output and write it to a local file
    FILE* fptr;
    char* file_loc = _get_file_loc(host_name);
    fptr = fopen(file_loc, "wb");
    if (!fptr) {
        fprintf(stderr, "could not open file for writing!");
        return;
    }

    char buffer[BUFSIZ];
    size_t readbytes;

    while (SSL_read_ex(ssl_o, buffer, sizeof(buffer), &readbytes) > 0) {
        fwrite(buffer, sizeof(char), readbytes, fptr);
    }

    // SSL_read_ex may exit when there's a failure and data couldn't reach the client OR
    // when all the data has been sent and no more remains, so a check if necessary to ensure
    // that it's the latter case. SSL_ERROR_ZERO_RETURN is sent when the peer (server) finishes sending data
    if (SSL_get_error(ssl_o, 0) != SSL_ERROR_ZERO_RETURN) {
        fprintf(stderr, "failed reading remaining data!");
        return;
    }

    // shut down the connection, server already shut down above, client should shut down too
    int ret = SSL_shutdown(ssl_o);
    if (ret < 1) {
        fprintf(stderr, "error shutting down!");
        return;
    }

    _clean_output(file_loc);

    free(file_loc);
    SSL_free(ssl_o);
    SSL_CTX_free(ctx);
}