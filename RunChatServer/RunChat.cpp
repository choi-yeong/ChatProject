#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <csignal>
#include <ctime>
#include <map>
#include <windows.h>
std::map<SOCKET, std::string> client_names; // 클라이언트 소켓과 닉네임을 서로 통용한다.
#pragma comment(lib, "ws2_32.lib")

// 서버가 시도할 포트 번호 범위 설정
#define PORT_START 15410 // 시작 포트 번호
#define PORT_END   15500 // 종료 포트 번호

#define MAX_CLIENTS 8 // 서버가 허용할 최대 동시 접속 클라이언트 수

// 전역 변수 선언
std::vector<SOCKET> client_sockets; // 연결된 클라이언트 소켓 목록
std::mutex clients_mutex;           // 클라이언트 목록 접근을 보호하기 위한 mutex
volatile bool server_running = true; // 서버 실행 상태 플래그
SOCKET server_socket = INVALID_SOCKET; // 서버 소켓


// 모든 클라이언트에게 메시지를 전송하는 함수
void broadcast_message(const std::string& message, SOCKET sender_socket = INVALID_SOCKET) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (SOCKET client : client_sockets) {
        if (client != sender_socket) {
            int result = send(client, message.c_str(), static_cast<int>(message.length()), 0);
            if (result == SOCKET_ERROR) {
                std::cerr << "send 오류: " << WSAGetLastError() << std::endl;
            }
        }
    }
}

// 클라이언트 처리 함수 수정
void handle_client(SOCKET client_socket) {
    char buffer[1024];
    // 클라이언트로부터 첫 메시지를 닉네임으로 수신
    char name_buffer[1024];
    int name_bytes = recv(client_socket, name_buffer, sizeof(name_buffer) - 1, 0);
    if (name_bytes <= 0) {
        closesocket(client_socket);
        return;
    }
    name_buffer[name_bytes] = '\0';
    std::string nickname(name_buffer);

    // 맵에 소켓과 닉네임 저장
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        client_names[client_socket] = nickname;
        client_sockets.push_back(client_socket); // 클라이언트 소켓 추가
    }
    // 현재 접속 클라이언트 수 출력
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        std::cout << "현재 접속인원: " << client_sockets.size() << "명" << std::endl;
    }
    while (server_running) {
        ZeroMemory(buffer, sizeof(buffer)); // 버퍼 초기화
        int bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0); // 클라이언트로부터 데이터 수신

        if (bytes == SOCKET_ERROR) {
            int errCode = WSAGetLastError();
            if (errCode == WSAECONNRESET) {
                std::cout << client_socket << "님과 연결이 끊겼습니다." << std::endl;
            }
            else {
                std::cerr << "recv 오류: " << errCode << std::endl;
            }
            break;
        }
        std::string msg(buffer); // 아래는 콘솔에 메시지를 표시하는 것이다.
        std::cout << "[" << std::time(nullptr) << "]" << client_names[client_socket] << "(" << client_socket << ") : " << msg << std::endl;
        std::string full_message = client_names[client_socket] + ": " + msg;
        broadcast_message(full_message, client_socket); // 메시지 전송
    }

    // 클라이언트 연결 해제 처리
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
        client_names.erase(client_socket);
    }

    closesocket(client_socket); // 클라이언트 소켓 닫기
}


// Ctrl+C 등 종료 신호를 처리하는 함수
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n서버 종료 요청 수신 (Ctrl+C). 클라이언트들에게 공지 중..." << std::endl;
        server_running = false;

        broadcast_message("서버가 종료됩니다. 연결이 끊어집니다.\n");

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            for (SOCKET client : client_sockets) {
                closesocket(client);
            }
            client_sockets.clear();
        }

        if (server_socket != INVALID_SOCKET) {
            closesocket(server_socket);
        }

        WSACleanup(); // Winsock 리소스 정리
        std::exit(0);
    }
}

// 사용 가능한 포트를 찾아 서버 소켓 생성하는 함수
SOCKET create_server_socket(int& used_port) {
    SOCKET sock;
    sockaddr_in server_addr;

    for (int port = PORT_START; port <= PORT_END; ++port) {
        sock = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성
        if (sock == INVALID_SOCKET) {
            std::cerr << "소켓 생성 실패: " << WSAGetLastError() << std::endl;
            continue;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        // 바인딩 시도
        if (bind(sock, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            closesocket(sock);
            continue;
        }

        // 리스닝 상태로 전환
        if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "리스닝 실패: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            continue;
        }

        used_port = port; // 사용한 포트 저장
        return sock;      // 생성된 소켓 반환
    }

    return INVALID_SOCKET; // 실패 시 INVALID_SOCKET 반환
}

// 메인 함수 - 서버 초기화 및 클라이언트 수락 루프
int main() {
    signal(SIGINT, signal_handler); // Ctrl+C 시 종료 처리 등록

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 실패: " << WSAGetLastError() << std::endl;
        return 1;
    }

    int selected_port = 0;
    server_socket = create_server_socket(selected_port); // 서버 소켓 생성 및 포트 바인딩
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "사용 가능한 포트를 찾을 수 없습니다 (" << PORT_START << "~" << PORT_END << ")" << std::endl;
        WSACleanup();
        return 1;
    }

    std::cout << "채팅 서버가 포트 " << selected_port << "에서 대기 중입니다 (최대 " << MAX_CLIENTS << "명)." << std::endl;
    std::cout << "서버 종료: Ctrl+C를 누르세요." << std::endl;

    // 클라이언트 수락 루프
    while (server_running) {
        sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &addr_len);

        if (client_socket == INVALID_SOCKET) {
            if (!server_running) break;
            std::cerr << "클라이언트 연결 실패: " << WSAGetLastError() << std::endl;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);

            if (client_sockets.size() >= MAX_CLIENTS) {
                std::string msg = "서버에 접속할 수 없습니다. (최대 접속 인원 초과)\n";
                send(client_socket, msg.c_str(), static_cast<int>(msg.length()), 0);
                closesocket(client_socket);
                continue;
            }
        }

        std::cout << "새로운 유저 접속(" << client_socket << ")" << std::endl;

        std::thread t(handle_client, client_socket);
        t.detach();
    }

    // 서버 종료 처리
    if (server_socket != INVALID_SOCKET) {
        closesocket(server_socket);
    }
    WSACleanup();
    std::cout << "서버가 정상적으로 종료되었습니다." << std::endl;
    return 0;
}