## Running Locally
- **Windows:** This repository was developed on Windows using Cygwin for 64-bit machines to access UNIX/POSIX compliant headers such as <code><unistd.h></code> and <code><sys/socket.h></code>. It won't compile natively on Windows using MSYS2 or MinGW.
    - Install Cygwin with the following packages (packages can be selected during the installation process via the setup):
    - <code>gcc-core</code>, <code>gcc-g++</code>, <code>cygwin-devel</code>
- **Unix/Linux-based:** The code should compile natively on POSIX compliant systems. All libraries used are present in the C stdlib.