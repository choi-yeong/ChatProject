#include "ClientHandler.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>

ClientHandler::ClientHandler(int clientSocket)
    : clientSocket(clientSocket) {
}

void ClientHandler::start() {
    handlerThread = std::thread(&ClientHandler::handleClient, this);  // 클라이언트 처리 스레드 시작
}

void ClientHandler::stop() {
    if (handlerThread.joinable()) {
        handlerThread.join();  // 스레드가 종료될 때까지 기다림
    }
    closesocket(clientSocket);  // 소켓 닫기
}

void ClientHandler::handleClient() {
    char buffer[1024];
    int bytesReceived;
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesReceived] = '\0';  // 받은 데이터 종료 문자로 처리
        std::string message(buffer);
        std::cout << "수신됨: " << message << std::endl;

        // 메시지에 대한 처리 (예: 클라이언트에게 다시 메시지 보내기)
        sendMessage("문자 수신: " + message);
    }

    // 연결이 끊어지면 소켓 종료
    if (bytesReceived == 0) {
        std::cout << "사용자 접속 종료." << std::endl;
    }
    else {
        std::cerr << "수신 실패!" << std::endl;
    }

    stop();  // 연결 종료
}

void ClientHandler::sendMessage(const std::string& message) {
    send(clientSocket, message.c_str(), message.size(), 0);  // 메시지 전송
}
