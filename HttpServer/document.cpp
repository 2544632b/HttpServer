#include <iostream>
#include <fstream>
#include "document.h"
using namespace std;

document::document() {
    memset(file_buffer, NULL, sizeof(file_buffer));
}
void document::readFile(const string file_path) {
    ifstream fin(file_path, ios::binary);
    if(!fin.is_open()) {
        cout << "Error reading file" << endl;
    }
    while(!fin.eof()) {
        fin.read(file_buffer, sizeof(file_buffer));
        this->length++;
    }
    return;
}
char* document::getFile() {
    return file_buffer;
}
