#pragma once

#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <locale>
#include <codecvt>
#include <Windows.h>

std::wstring lpResult = std::wstring();

int process(const int argc, char* argv[]);
LPWSTR charToWCHAR(const char* input, int inLen);
void autoExpandEnvironmentVariables(std::wstring& text);
std::wstring expandEnvironmentVariables(const std::wstring& input);