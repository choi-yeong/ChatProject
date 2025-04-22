#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

#include <string>
#include <thread>
#include <winsock2.h>

class ClientHandler {
    int clientSocket;
    std::thread handlerThread;  // Ŭ���̾�Ʈ ������ ���� ������ ������

    void handleClient();  // Ŭ���̾�Ʈ���� �޽��� �ۼ��� ó��
    void sendMessage(const std::string& message);  // Ŭ���̾�Ʈ���� �޽��� ����
public:
	ClientHandler(int clientSocket);  // ������: Ŭ���̾�Ʈ ������ �ʱ�ȭ
    void start();  // Ŭ���̾�Ʈ���� ����� �����ϴ� �Լ�
    void stop();   // Ŭ���̾�Ʈ���� ���� ����
};

#endif
