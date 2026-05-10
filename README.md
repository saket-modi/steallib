## Running locally
- **Windows:** This repository was developed on Windows using Cygwin for 64-bit machines to access UNIX/POSIX compliant headers such as <code><unistd.h></code> and <code><sys/socket.h></code>. It won't compile natively on Windows using MSYS2 or MinGW.
    - Install Cygwin with the following packages (packages can be selected during the installation process via the setup):
    - <code>gcc-core</code>, <code>gcc-g++</code>, <code>cygwin-devel</code>, <code>libssl-devel</code>
- **Unix/Linux-based:** The code should compile natively on POSIX compliant systems. All libraries used are present in the C stdlib.

## Sample usage
```cpp
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
```