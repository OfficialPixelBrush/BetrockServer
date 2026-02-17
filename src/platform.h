#pragma once

// Platform-specific socket and system includes
#ifdef _WIN32
    // Windows socket headers
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <io.h>
    #pragma comment(lib, "ws2_32.lib")
    
    // Define missing POSIX macros for Windows
    #define SHUT_RDWR SD_BOTH
    typedef int socklen_t;
    
    // Windows uses SOCKET type, Unix uses int
    typedef SOCKET SocketFd;
    
    // errno mapping for Windows
    #define SOCKET_ERROR_CODE WSAGetLastError()
    
    // Redefine socket operations for Windows
    inline int close_socket(SocketFd fd) {
        return closesocket(fd);
    }
    
    inline int read_socket(SocketFd fd, void* buf, int len) {
        return recv(fd, (char*)buf, len, 0);
    }
    
    inline int write_socket(SocketFd fd, const void* buf, int len) {
        return send(fd, (const char*)buf, len, 0);
    }
    
    // Storage for WSA startup
    class WinsockInit {
    public:
        WinsockInit() {
            WSADATA wsa_data;
            int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
            if (result != 0) {
                throw std::runtime_error("WSAStartup failed");
            }
        }
        
        ~WinsockInit() {
            WSACleanup();
        }
    };
    
#else
    // Unix/Linux socket headers
    #include <unistd.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    
    // Unix uses int for file descriptors
    typedef int SocketFd;
    
    // errno mapping for Unix
    #define SOCKET_ERROR_CODE errno
    
    // Redefine socket operations for Unix (basically no-ops)
    inline int close_socket(SocketFd fd) {
        return close(fd);
    }
    
    inline int read_socket(SocketFd fd, void* buf, int len) {
        return read(fd, buf, len);
    }
    
    inline int write_socket(SocketFd fd, const void* buf, int len) {
        return write(fd, buf, len);
    }
    
    // Dummy Windows init for Unix
    class WinsockInit {
    public:
        WinsockInit() {}
        ~WinsockInit() {}
    };
    
#endif

// Common includes for both platforms
#include <iostream>
#include <cstring>
#include <stdexcept>
