#include "Server.hpp"

int main() {
    Server server(12345);
    server.start();  // 서버 시작
    return 0;
}
