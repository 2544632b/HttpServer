#include <iostream>
#pragma warning(disable : 4996)
using namespace std;

class status {
public:
	static string getStatus(string status_code);
	static string getServer(string server_type);
	static string getDate();
	static string getFileType(string file_type);
};
// Access control next