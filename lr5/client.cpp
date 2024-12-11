#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "54000" // Порт сервера
#define SERVER_IP "127.0.0.1" // IP сервера

bool running = true;

void receiveMessages(SOCKET sock) {
    char buffer[1024];
    while (running) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << buffer << std::endl;
        }
        else if (bytesReceived == 0) {
            std::cout << "Соединение закрыто сервером." << std::endl;
            running = false;
            break;
        }
        else {
            std::cerr << "Ошибка при получении данных. Код ошибки: " << WSAGetLastError() << std::endl;
            running = false;
            break;
        }
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsOk = WSAStartup(ver, &wsData);
    if (wsOk != 0) {
        std::cerr << "Не удалось инициализировать Winsock. Код ошибки: " << wsOk << std::endl;
        return -1;
    }


    struct addrinfo hints, *res;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_protocol = IPPROTO_TCP;

    int addrRes = getaddrinfo(SERVER_IP, PORT, &hints, &res);
    if (addrRes != 0) {
        std::cerr << "getaddrinfo не удалось получить. Код ошибки: " << addrRes << std::endl;
        WSACleanup();
        return -1;
    }

    SOCKET sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Не удалось создать сокет. Код ошибки: " << WSAGetLastError() << std::endl;
        freeaddrinfo(res);
        WSACleanup();
        return -1;
    }

    int connResult = connect(sock, res->ai_addr, (int)res->ai_addrlen);
    if (connResult == SOCKET_ERROR) {
        std::cerr << "Не удалось подключиться к серверу. Код ошибки: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        freeaddrinfo(res);
        WSACleanup();
        return -1;
    }

    freeaddrinfo(res);
    std::cout << "Подключено к серверу." << std::endl;

    std::thread recvThread(receiveMessages, sock);

    while (running) {
        std::string msg;
        std::getline(std::cin, msg);
        if (msg.empty()) continue;
        if (msg == "/quit") {
            running = false;
            break;
        }
        // std::cout << "Popitka";
        send(sock, msg.c_str(), msg.size() + 1, 0);
    }

    closesocket(sock);
    recvThread.join();
    WSACleanup();
    return 0;
}
