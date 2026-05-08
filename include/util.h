/* STRUCTURES */
typedef struct {
    // https://example.com/api/data
    char* protocol;
    char* host_name;
    char* port;
    char* path;
} parsed_url_t;

/* FUNCTIONS */

// constructor for parsing url string 
parsed_url_t* _parse_url(const char*);

// start socket, get fd, send http request
void _start_socket(char*, char*, char*);

// define http/https funcs in separate files
void _http_request(int, char*, char*);
void _https_request(int, char*, char*);