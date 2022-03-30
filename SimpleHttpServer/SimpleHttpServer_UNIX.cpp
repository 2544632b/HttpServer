#include "HttpServer.h"

int main(void) {
    HttpServer HttpServer(false);
    if (HttpServer.create_listen(2000) == false) {
        cout << "Error creating listener." << endl;
    }
    
    for(int i = 0; i < 100; i++) {
        HttpServer.content_back_thread().detach();
    }
    sleep(100000);
    
    return NULL;
}
