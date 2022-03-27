#include <iostream>
#include <thread>
#include <fstream>
#include <mutex>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
#pragma warning(disable : 4996)

#include "status.h"

using namespace std;

#define port 8388
#define addr "192.168.1.9"

class HttpServer {
private:
    SSL_CTX* ctx = nullptr;
    bool enable_ssl = NULL;
    SSL* ssl = nullptr;

    SOCKET socket_server_f;

    char buffer_pool[4096];
    SOCKADDR_IN client_addr;
    int length = sizeof(SOCKADDR);

public:
    HttpServer(bool enable_ssl) {
        WORD w_req = MAKEWORD(2, 2);
        WSADATA wsadata;

        if (WSAStartup(w_req, &wsadata) == 0) {
            cout << "WSAData initalized" << endl;
        }

        this->socket_server_f = socket(AF_INET, SOCK_STREAM, 0);

        if (enable_ssl == true) {
            this->enable_ssl = enable_ssl;
            SSL_library_init();
            OpenSSL_add_all_algorithms();
            SSL_load_error_strings();
            ctx = SSL_CTX_new(SSLv23_server_method());
            if (ctx == NULL) {
                ERR_print_errors_fp(stdout);
                exit(-1);
            }

            SSL_CTX_use_certificate_file(ctx, "", SSL_FILETYPE_PEM);    // Enter the orgniaze CA here, tell the browser your site can be trust first.
            SSL_CTX_use_PrivateKey_file(ctx, "", SSL_FILETYPE_PEM);     // Enter your server private key, encrypt your session with customer browser.

            if (!SSL_CTX_check_private_key(ctx)) {
                ERR_print_errors_fp(stdout);    // OUTPUT some error here.
                exit(-1);
            }
        }

        SOCKADDR_IN server_socketaddr;
        server_socketaddr.sin_family = AF_INET;
        server_socketaddr.sin_addr.S_un.S_addr = inet_addr(addr);
        server_socketaddr.sin_port = htons(port);
        bind(this->socket_server_f, (SOCKADDR*)&server_socketaddr, sizeof(SOCKADDR));
    }

    ~HttpServer() {
        if (this->enable_ssl == true) {
            SSL_CTX_free(ctx);
        }
        closesocket(socket_server_f);
        WSACleanup();
    }

    bool create_listen(int queues) {
        cout << "Server is listening on " << (string)addr << ":" << port << endl;
        listen(this->socket_server_f, queues);
        return true;
    }

    bool content_back() {
        SOCKET conn = accept(this->socket_server_f, (SOCKADDR*)&this->client_addr, &this->length);

        if (this->enable_ssl == true) {
            this->ssl = SSL_new(ctx);
            SSL_set_fd(this->ssl, conn);

            SSL_accept(this->ssl);
        }

        memset(buffer_pool, NULL, sizeof(buffer_pool));
        long len = recv(conn, buffer_pool, sizeof(buffer_pool), 0);

        //string custom_browser_address = inet_ntoa(client_addr.sin_addr);
        status status;
        string header = status.getHeader("server_ok", "test", "html", 999);

        document document;
        document.readFile("ServerDocuments/index.html");

        if (this->enable_ssl == true) {
            int ssl_len = SSL_read(this->ssl, buffer_pool, sizeof(buffer_pool));

            if (ssl_len > 0) {
                SSL_write(this->ssl, header.c_str(), header.size());
                SSL_write(this->ssl, document.file_buffer, 999);
            }
        }

        if (this->enable_ssl == false) {
            if (len > 0) {
                // Safari need send for a section(actually just strictly head rule).
                send(conn, header.c_str(), header.size(), 0);
                send(conn, document.file_buffer, 999, 0);
            }
        }

        if (this->enable_ssl == true) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }

        closesocket(conn);

        return true;
    }

    thread content_back_thread() {
        return thread(&HttpServer::content_back, this);
    }
};

int main(void) {
    HttpServer HttpServer(false);
    if (HttpServer.create_listen(50) == false) {
        cout << "Error creating listener." << endl;
    }

    for (int i = 0; i < 100; i++) {
        HttpServer.content_back_thread().detach();
    }
    Sleep(100000);
    return NULL;
}
