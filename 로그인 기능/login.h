#pragma once

#include <string>

bool register_user(const std::string& name, const std::string& password);
bool login_user(const std::string& name, const std::string& password);
