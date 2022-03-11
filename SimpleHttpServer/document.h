#ifndef document_h
#define document_h
#include <iostream>
using namespace std;

class document {
    friend class HttpServer;
private:
    char file_buffer[7162];
    int length = 0;
public:
    document();
    void readFile(const string file_path);
    char* getFile();
};

#endif /* document_h */
