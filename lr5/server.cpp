#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "54000" 

std::vector<SOCKET> clients;
std::mutex clients_mutex;

void broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto client : clients) {
        send(client, message.c_str(), message.size() + 1, 0);
    }
}

void handleClient(SOCKET clientSocket) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
            closesocket(clientSocket);
            std::cout << "Клиент отключился." << std::endl;
            break;
        }
        // Рассылаем полученное сообщение всем клиентам
        broadcastMessage(std::string(buffer));
        std::cout << "Сообщение получено и разослано: " << buffer << std::endl;
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    // Инициализация Winsock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsOk = WSAStartup(ver, &wsData);
    if (wsOk != 0) {
        std::cerr << "Не удалось инициализировать Winsock. Код ошибки: " << wsOk << std::endl;
        return -1;
    }

    // Создание адреса для сервера
    struct addrinfo hints, *res;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;     // Для bind

    int addrRes = getaddrinfo(NULL, PORT, &hints, &res);
    if (addrRes != 0) {
        std::cerr << "getaddrinfo не удалось. Код ошибки: " << addrRes << std::endl;
        WSACleanup();
        return -1;
    }

    // Создание сокета
    SOCKET listenSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Не удалось создать сокет. Код ошибки: " << WSAGetLastError() << std::endl;
        freeaddrinfo(res);
        WSACleanup();
        return -1;
    }

    // Привязка сокета
    bind(listenSocket, res->ai_addr, (int)res->ai_addrlen);
    freeaddrinfo(res);

    // Начинаем слушать
    listen(listenSocket, SOMAXCONN);
    std::cout << "Сервер запущен и слушает на порту " << PORT << std::endl;

    // Основной цикл для принятия клиентов
    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Не удалось принять клиента. Код ошибки: " << WSAGetLastError() << std::endl;
            continue;
        }

        // Добавляем клиента в список
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(clientSocket);
        }

        std::cout << "Клиент подключился." << std::endl;

        // Запускаем поток для обработки клиента
        std::thread(handleClient, clientSocket).detach();
    }

    // Закрытие сокета и очистка Winsock (на самом деле, этот код никогда не выполнится)
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
