#include "Server.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include<winsock2.h>


Server::Server(int port) { //���� ���� ���� Ŭ����
	// ���� ���� �ʱ�ȭ
	this->port = port;  // �ʱ�ȭ ��Ʈ ��ȣ
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0) {
		std::cerr << "���� ���� ����!" << std::endl;
		exit(EXIT_FAILURE);
	}
}
void Server::start() { // ���� ����
	// Bind and listen
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) > 0) {
		perror("���ε� ����");
		exit(EXIT_FAILURE);
	}
	listen(serverSocket, 5);
	running = true;
	std::cout << "���� ���۵� ��Ʈ��ȣ : " << port << std::endl;
}

void Server::acceptClient() { //Ŭ���̾�Ʈ ���� ����
	// Accept a client connection
	int clientSocket = accept(serverSocket, NULL, NULL);
	if (clientSocket >= 0) {
		clientSockets.push_back(clientSocket);
		std::cout << "����� ����� : " << clientSocket << std::endl;
	}
}
void Server::broadcast(const std::string& message) { //��� Ŭ���̾�Ʈ�� �� �� �ְ� ��ü ä�� ����
	// Send a message to all connected clients
	for (int clientSocket : clientSockets) {
		send(clientSocket, message.c_str(), message.size(), 0);
	}
}

void Server::stop() { //���� ����
	// Close all client sockets
	for (int clientSocket : clientSockets) {
		close(clientSocket);
	}
	clientSockets.clear();
	close(serverSocket);
	running = false;
	std::cout << "���� �����." << std::endl;
}

void Server::sendMessage(int clientSocket, const std::string& message) { // �Ѹ��� Ŭ���̾�Ʈ���� �޽��� ����.
	// Send a message to a specific client
	send(clientSocket, message.c_str(), message.size(), 0);
}