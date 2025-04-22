#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")
std::string ip_adrs = "192.168.1.223";
int port_num = 15410;

// Ŭ���̾�Ʈ Ŭ������ ����
class Client {
public:
    void connectToServer(const std::string& ip, int port); //���� �����ϴ� �޼���
private:
    void receiveMessages(SOCKET clientSocket); // �����κ��� �޽����� �����ϴ� �޼���
    void sendMessages(SOCKET clientSocket); // ������ �޽����� �����ϴ� �޼���
};

// �����κ��� �޽����� �����ϴ� �޼���
void Client::receiveMessages(SOCKET clientSocket) {
    char buffer[1024];  // ������ �����͸� ������ ���� ����

    while (true) {
        // �� �������� ���� �ʱ�ȭ (���� ������ ����)
        ZeroMemory(buffer, sizeof(buffer));

        // �����κ��� �޽����� ���� (�ִ� sizeof(buffer) - 1 ����Ʈ)
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived > 0) {
            // ���ŵ� �޽����� �������� �� ����('\0')�� �߰��� ���ڿ��� ó��
            buffer[bytesReceived] = '\0';

            // ���ŵ� �޽����� �ֿܼ� ���
            std::cout << "\n[���� �޽���] " << buffer << "\n> ";
            std::cout.flush();  // '>' ������Ʈ�� �ٽ� ���
        }
        else if (bytesReceived == 0) {
            // ������ ���������� ������ ������ ���
            std::cout << "�������� ������ ����Ǿ����ϴ�.\n";
            break;  // ���� Ż��
        }
        else {
            // ���� ��Ȳ: recv() ����. ��: ���� ���� ����
            int err = WSAGetLastError();  // ���� �ڵ� Ȯ��

            if (err == WSAECONNRESET) {
                // ������ ������������ ������ ������ �� �߻��ϴ� ���� �ڵ� (10054)
                std::cout << "����) ������ ������������ ����Ǿ����ϴ�.\n";
            }
            else {
                // �ٸ� ������ ���� ����
                std::cerr << "�޽��� ���� ����: " << err << std::endl;
            }
            break;  // ���� ������ ���� ����
        }
    }
}

//������ �޽����� �����ϴ� �޼��� ����
void Client::sendMessages(SOCKET clientSocket) { //����ڰ� �Է��� �޽����� ����
    std::string message;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, message);
        if (message.empty()) continue;

        // �Է¹��� �޽����� ������ ����
        int result = send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0);
        if (result == SOCKET_ERROR) {
            std::cerr << "�޽��� ���� ����: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

// ������ �����ϴ� �޼��� ����
void Client::connectToServer(const std::string& ip, int port) {
    WSADATA wsaData; //winsock �ʱ�ȭ �����͸� ����
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup ����: " << WSAGetLastError() << std::endl;
        return;
    }

    //TCP ���� ����
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "���� ���� ����: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    //���� �ּ� ����ü ����
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    // IP �ּҸ� ���ڿ����� ���̳ʸ� ���·� ��ȯ
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    // ������ ����õ�
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "���� ���� ����: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    std::cout << "������ ����Ǿ����ϴ�! ä���� �����ϼ���.\n";

    // �޽��� ���� ������
    std::thread recvThread(&Client::receiveMessages, this, clientSocket);
    recvThread.detach();

    // �޽��� ���� ����
    sendMessages(clientSocket);

    // ���α׷� ���� �� ���� �ݱ� winsock ���ҽ� ����
    closesocket(clientSocket);
    WSACleanup();
}

// ���α׷� ������
int main() {
    Client client; // client ��ü ����
    client.connectToServer(ip_adrs, port_num);
    return 0;
}