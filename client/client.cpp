#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")
std::string ip_adrs = "192.168.1.223";
int port_num = 15410;

// 클라이언트 클래스를 정의
class Client {
public:
    void connectToServer(const std::string& ip, int port); //서버 연결하는 메서드
private:
    void receiveMessages(SOCKET clientSocket); // 서버로부터 메시지를 수신하는 메서드
    void sendMessages(SOCKET clientSocket); // 서버로 메시지를 전송하는 메서드
};

// 서버로부터 메시지를 수신하는 메서드
void Client::receiveMessages(SOCKET clientSocket) {
    char buffer[1024];  // 수신할 데이터를 저장할 버퍼 선언

    while (true) {
        // 매 루프마다 버퍼 초기화 (기존 데이터 제거)
        ZeroMemory(buffer, sizeof(buffer));

        // 서버로부터 메시지를 수신 (최대 sizeof(buffer) - 1 바이트)
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived > 0) {
            // 수신된 메시지의 마지막에 널 문자('\0')를 추가해 문자열로 처리
            buffer[bytesReceived] = '\0';

            // 수신된 메시지를 콘솔에 출력
            std::cout << "\n[서버 메시지] " << buffer << "\n> ";
            std::cout.flush();  // '>' 프롬프트를 다시 출력
        }
        else if (bytesReceived == 0) {
            // 서버가 정상적으로 연결을 종료한 경우
            std::cout << "서버와의 연결이 종료되었습니다.\n";
            break;  // 루프 탈출
        }
        else {
            // 예외 상황: recv() 실패. 예: 서버 강제 종료
            int err = WSAGetLastError();  // 오류 코드 확인

            if (err == WSAECONNRESET) {
                // 서버가 비정상적으로 연결을 끊었을 때 발생하는 오류 코드 (10054)
                std::cout << "오류) 서버가 비정상적으로 종료되었습니다.\n";
            }
            else {
                // 다른 이유로 인한 오류
                std::cerr << "메시지 수신 실패: " << err << std::endl;
            }
            break;  // 연결 문제로 루프 종료
        }
    }
}

//서버로 메시지를 전송하는 메서드 구현
void Client::sendMessages(SOCKET clientSocket) { //사용자가 입력한 메시지를 저장
    std::string message;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, message);
        if (message.empty()) continue;

        // 입력받은 메시지를 서버로 전송
        int result = send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0);
        if (result == SOCKET_ERROR) {
            std::cerr << "메시지 전송 실패: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

// 서버에 연결하는 메서드 구현
void Client::connectToServer(const std::string& ip, int port) {
    WSADATA wsaData; //winsock 초기화 데이터를 저장
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 실패: " << WSAGetLastError() << std::endl;
        return;
    }

    //TCP 소켓 생성
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "소켓 생성 실패: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    //서버 주소 구조체 설정
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    // IP 주소를 문자열에서 바이너리 형태로 변환
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    // 서버에 연결시도
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "서버 연결 실패: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    std::cout << "서버에 연결되었습니다! 채팅을 시작하세요.\n";

    // 메시지 수신 스레드
    std::thread recvThread(&Client::receiveMessages, this, clientSocket);
    recvThread.detach();

    // 메시지 전송 루프
    sendMessages(clientSocket);

    // 프로그램 종료 전 소켓 닫기 winsock 리소스 정리
    closesocket(clientSocket);
    WSACleanup();
}

// 프로그램 진입점
int main() {
    Client client; // client 객체 생성
    client.connectToServer(ip_adrs, port_num);
    return 0;
}