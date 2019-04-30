#pragma once

#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <locale>
#include <codecvt>
#include <thread>
#include <chrono>
#include <Windows.h>

/*
 * A Win32 (technically Unicode Win64) subroutine application designed to allow 
 * breaking out of the UWP sandbox environment by allowing execution of arbitrary
 * files on the filesystem.
 * Also includes a config backup/restore engine, since that feature is also not possible
 * from within the UWP sandbox environemnt.
 *
 * //// ARGUMENTS ARE DETAILED IN UWPProcessLauncher.cpp
 * //// RETURN CODES ////
 *  0: Operations completed successfully.
 *  1: A critical file IO error was encountered.
 * -1: One or more critical arguments are missing.
 *  2-65536: A nonzero return code was produced by a call to ShellExecuteW.
 *           See that function's documentation for more details.
 */

// Must be declared outside of the methods because it's used in main() and process()
std::wstring lpResult = std::wstring();

LPWSTR checkArg(char** argv, const int argc, int i, const std::wstring flag);
int process(const int argc, char* argv[]);
LPWSTR charToWCHAR(const char* input, int inLen);
void autoExpandEnvironmentVariables(std::wstring& text);
std::wstring expandEnvironmentVariables(const std::wstring& input);