#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

#include <string>
#include <thread>
#include <winsock2.h>

class ClientHandler {
    int clientSocket;
    std::thread handlerThread;  // 클라이언트 연결을 위한 별도의 스레드

    void handleClient();  // 클라이언트와의 메시지 송수신 처리
    void sendMessage(const std::string& message);  // 클라이언트에게 메시지 전송
public:
	ClientHandler(int clientSocket);  // 생성자: 클라이언트 소켓을 초기화
    void start();  // 클라이언트와의 통신을 시작하는 함수
    void stop();   // 클라이언트와의 연결 종료
};

#endif
