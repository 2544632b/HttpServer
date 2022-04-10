#include <iostream>
using namespace std;

class status {
public:
    static string getStatus(string status_code);
    static string getServer(string server_type);
    static string getDate();
    static string getFileType(string file_type);
    static string getHttpLength(long length);
    static string getHeader(string status_code, string server_type, string file_type, long length);
};
