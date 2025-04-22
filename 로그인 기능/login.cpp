#include "login.h"
#include <iostream>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

const std::string DB_PATH = "users.json";

// 회원가입 함수
bool register_user(const std::string& name, const std::string& password) {
    std::ifstream inFile(DB_PATH);
    json db;

    // 파일이 존재하면 읽고, 존재하지 않으면 빈 db 초기화
    if (inFile.is_open()) {
        inFile >> db;
        inFile.close();
    }
    else {
        db["users"] = json::array();  // 파일이 없으면 "users" 배열을 새로 생성
    }

    // 중복 검사
    for (auto& user : db["users"]) {
        if (user["name"] == name) {
            return false; // 이미 존재
        }
    }

    // 새 유저 추가
    db["users"].push_back({ {"name", name}, {"password", password} });

    // 파일에 저장
    std::ofstream outFile(DB_PATH);
    if (outFile.is_open()) {
        outFile << db.dump(4); // pretty print
        outFile.close();
        return true;
    }
    else {
        std::cerr << "파일을 열 수 없습니다: " << DB_PATH << std::endl;
        return false;
    }
}

// 로그인 함수
bool login_user(const std::string& name, const std::string& password) {
    std::ifstream inFile(DB_PATH);
    if (!inFile.is_open()) {
        std::cerr << "파일을 열 수 없습니다: " << DB_PATH << std::endl;
        return false;
    }

    json db;
    inFile >> db;
    inFile.close();

    for (auto& user : db["users"]) {
        if (user["name"] == name && user["password"] == password) {
            return true; // 로그인 성공
        }
    }
    return false; // 로그인 실패
}
