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
// destructor for cleanup
int _clear_parsed_url(parsed_url_t*);

// start socket, get fd, send http request
void _start_socket(char*, char*, char*, const char*);