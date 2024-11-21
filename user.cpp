#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h> // Для работы с IP-адресами
#define PORT 7432

using namespace std;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    // Создание сокета
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Socket creation error." << endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Преобразование IPv4 и IPv6 адресов из текстового представления в двоичное
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address / Address not supported." << endl;
        return -1;
    }

    // Подключение к серверу
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection failed." << endl;
        return -1;
    }

    string query;
    while (true) {
        cout << "Enter your query (or 'exit' to quit): ";
        getline(cin, query);
        if (query == "exit") {
            break;
        }

        send(sock, query.c_str(), query.size(), 0);
        
        char buffer[1024] = {0};
        int bytes_read = read(sock, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            cout << "Response from server: " << buffer << endl;
        }
    }

    close(sock);
    return 0;
}
