#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "status.h"
using namespace std;

#define port 1025
#define addr "192.168.1.8"

class HttpServer {
private:
    /* Server settings */
    bool enable_ssl = NULL;
    int socket_server_f = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_socketaddr;
    
    /* SSL settings */
    SSL_CTX *ctx;
    SSL *ssl = nullptr;
    
    /* Client settings */
    char buffer_pool[4096];
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    
    mutex mutex;
    
public:
    HttpServer(bool enable_ssl) {
        if(enable_ssl == true) {
            this->enable_ssl = enable_ssl;
            SSL_library_init();
            OpenSSL_add_all_algorithms();
            SSL_load_error_strings();
            this->ctx = SSL_CTX_new(SSLv23_server_method());
            if(ctx == NULL) {
                ERR_print_errors_fp(stdout);
                exit(-1);
            }
            
            SSL_CTX_use_certificate_file(ctx, "", SSL_FILETYPE_PEM);    // Enter the org CA here, tell the browser your site can be trust first.
            SSL_CTX_use_PrivateKey_file(ctx, "", SSL_FILETYPE_PEM);     // Enter your server private key, encrypt your session with customer browser.
            
            if(!SSL_CTX_check_private_key(ctx)) {
                ERR_print_errors_fp(stdout);    // OUTPUT some error here.
                exit(-1);
            }
        }
        
        server_socketaddr.sin_family = AF_INET;
        server_socketaddr.sin_addr.s_addr = inet_addr(addr);
        server_socketaddr.sin_port = htons(port);
        
        try {
            if(::bind(this->socket_server_f, (struct sockaddr *)&server_socketaddr, sizeof(server_socketaddr)) == -1) {
                throw ::bind(this->socket_server_f, (struct sockaddr *)&server_socketaddr, sizeof(server_socketaddr));
            }
        } catch(int e) {
            cout << "Failed to bind the http server, ";
            cerr << e << endl;
            cout << "The server is already on this port?" << endl;
            exit(-1);
        }
    }
    
    ~HttpServer() {
        if(this->enable_ssl == true) {
            SSL_CTX_free(ctx);
        }
        close(this->socket_server_f);
        
    }
    
    bool create_listen(int queues) {
        try {
            cout << "Server is listening on " << (string)addr << ":" << port << endl;
            listen(this->socket_server_f, queues);
            return true;
        } catch(exception &e) {
            cout << "An error while creating listener, ";
            cerr << e.what() << endl;
            return false;
        }
    }
    
    bool content_back() {
        mutex.lock();
        
        int conn = ::accept(this->socket_server_f, (struct sockaddr *)&client_addr, &length);
        
        if(this->enable_ssl == true) {
            this->ssl = SSL_new(this->ctx);
            SSL_set_fd(this->ssl, conn);
        
            SSL_accept(this->ssl);
        }
        
        memset(buffer_pool, NULL, sizeof(buffer_pool));
        
        /*string address = inet_ntoa(client_addr.sin_addr);*/
        status status;
        string header = status.getHeader("server_ok", "test", "html", 999);

        document document;
        document.readFile("ServerDocuments/index.html");
        
        if(this->enable_ssl == true) {
            long ssl_len = SSL_read(ssl, buffer_pool, sizeof(buffer_pool));
            cout << buffer_pool << endl;
            if(ssl_len > 0) {
                SSL_write(ssl, header.c_str(), header.size());
                SSL_write(ssl, document.file_buffer, 999);
            }
        }
        
        if(this->enable_ssl == false) {
            long len = recv(conn, buffer_pool, sizeof(buffer_pool), 0);
            if(len > 0) {
                cout << buffer_pool << endl;
                send(conn, header.c_str(), header.size(), 0);
                send(conn, document.file_buffer, 999, 0);
            }
        }
        
        if(this->enable_ssl == true) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        
        close(conn);
        
        mutex.unlock();
        
        return true;
    }
    
    thread content_back_thread() {
        return thread(&HttpServer::content_back, this);
    }
};

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
