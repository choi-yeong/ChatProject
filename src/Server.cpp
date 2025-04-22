#include "Server.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include<winsock2.h>


Server::Server(int port) { //서버 소켓 생성 클래스
	// 서버 소켓 초기화
	this->port = port;  // 초기화 포트 번호
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0) {
		std::cerr << "소켓 생성 실패!" << std::endl;
		exit(EXIT_FAILURE);
	}
}
void Server::start() { // 서버 시작
	// Bind and listen
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) > 0) {
		perror("바인딩 실패");
		exit(EXIT_FAILURE);
	}
	listen(serverSocket, 5);
	running = true;
	std::cout << "서버 시작됨 포트번호 : " << port << std::endl;
}

void Server::acceptClient() { //클라이언트 연결 수락
	// Accept a client connection
	int clientSocket = accept(serverSocket, NULL, NULL);
	if (clientSocket >= 0) {
		clientSockets.push_back(clientSocket);
		std::cout << "사용자 연결됨 : " << clientSocket << std::endl;
	}
}
void Server::broadcast(const std::string& message) { //모든 클라이언트가 볼 수 있게 전체 채팅 제공
	// Send a message to all connected clients
	for (int clientSocket : clientSockets) {
		send(clientSocket, message.c_str(), message.size(), 0);
	}
}

void Server::stop() { //서버 종료
	// Close all client sockets
	for (int clientSocket : clientSockets) {
		close(clientSocket);
	}
	clientSockets.clear();
	close(serverSocket);
	running = false;
	std::cout << "서버 종료됨." << std::endl;
}

void Server::sendMessage(int clientSocket, const std::string& message) { // 한명의 클라이언트에게 메시지 전송.
	// Send a message to a specific client
	send(clientSocket, message.c_str(), message.size(), 0);
}