#include "ClientHandler.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>

ClientHandler::ClientHandler(int clientSocket)
    : clientSocket(clientSocket) {
}

void ClientHandler::start() {
    handlerThread = std::thread(&ClientHandler::handleClient, this);  // Ŭ���̾�Ʈ ó�� ������ ����
}

void ClientHandler::stop() {
    if (handlerThread.joinable()) {
        handlerThread.join();  // �����尡 ����� ������ ��ٸ�
    }
    closesocket(clientSocket);  // ���� �ݱ�
}

void ClientHandler::handleClient() {
    char buffer[1024];
    int bytesReceived;
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesReceived] = '\0';  // ���� ������ ���� ���ڷ� ó��
        std::string message(buffer);
        std::cout << "���ŵ�: " << message << std::endl;

        // �޽����� ���� ó�� (��: Ŭ���̾�Ʈ���� �ٽ� �޽��� ������)
        sendMessage("���� ����: " + message);
    }

    // ������ �������� ���� ����
    if (bytesReceived == 0) {
        std::cout << "����� ���� ����." << std::endl;
    }
    else {
        std::cerr << "���� ����!" << std::endl;
    }

    stop();  // ���� ����
}

void ClientHandler::sendMessage(const std::string& message) {
    send(clientSocket, message.c_str(), message.size(), 0);  // �޽��� ����
}
