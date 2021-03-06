#include <iostream>
#include <ctime>
#include "status.h"
using namespace std;

string status::getStatus(string status_code) {
    if(strcmp(status_code.c_str(), "server_ok") == 0) {
        return "HTTP/1.1 200\n";
    }
    else if(strcmp(status_code.c_str(), "not_found") == 0) {
        return "HTTP/1.1 404\n";
    }
    else if(strcmp(status_code.c_str(), "forbidden") == 0) {
        return "HTTP/1.1 403\n";
    }
    return NULL;
}

string status::getServer(string server_type) {
    return "Server: "+server_type+"\n";
}

string status::getDate() {
    time_t serverDate = time(0);
    tm* getDate = gmtime(&serverDate);
    char* getDateChar = asctime(getDate);
    return "Date: "+string(getDateChar);
}

string status::getFileType(string file_type) {
    if(strcmp(file_type.c_str(), "html") == 0) {
        return "Content-Type: text/html\n";
    }
    else if (strcmp(file_type.c_str(), "css") == 0) {
        return "Content-Type: text/css\n";
    }
    else if (strcmp(file_type.c_str(), "javascript") || strcmp(file_type.c_str(), "js") == 0) {
        return "Content-Type: x-application/javascript\n";
    }
    else {
        return "Content-Type: text/binary\n";
    }
}

string status::getHttpLength(long length) {
    return "Content-Length: " + to_string(length) + "\n";
}

string status::getHeader(string status_code,
                         string server_type,
                         string file_type,
                         long length) {
    string status = status::getStatus(status_code)
    +status::getDate()
    +status::getServer(server_type)
    +status::getFileType(file_type)
    +status::getHttpLength(length)
    +"\r\n";
    return status;
}
