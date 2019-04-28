
#include "UWPProcessLauncher.h"

using namespace std;

int main(int argc, char* argv[])
{
	int result = process(argc, argv);

	if (result != 0 && lpResult != L"") 
	{
		ofstream fout;
		fout.open(lpResult);
		fout << result;
		fout.flush();
		fout.close();
	}
	
	return result;
}

int process(const int argc, char* argv[])
{
	STARTUPINFO si = { sizeof(STARTUPINFO), 0 };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = { 0 };
	wstring lpName = L"";
	LPWSTR lpTarget = NULL;
	LPWSTR lpArgs = NULL;

	const wstring ARG_NAME = L"/name";
	const wstring ARG_FILE = L"/file";
	const wstring ARG_PARAMS = L"/params";
	const wstring ARG_RESCODE = L"/log";

	for (int i = 0; i < argc; i++)
	{
		LPWSTR arg = charToWCHAR(argv[i], strlen(argv[i]));

		if (arg == ARG_NAME) {
			if (i + 1 < argc) {
				lpName = charToWCHAR(argv[i + 1], strlen(argv[i + 1]));
				i++;
			}
		}
		else if (arg == ARG_FILE) {
			if (i + 1 < argc) {
				lpName = expandEnvironmentVariables(charToWCHAR(argv[i + 1], strlen(argv[i + 1])));
				i++;
			}
		}
		else if (arg == ARG_RESCODE) {
			if (i + 1 < argc) {
				lpResult = expandEnvironmentVariables(charToWCHAR(argv[i + 1], strlen(argv[i + 1])));
				i++;
			}
		}
		else if (arg == ARG_PARAMS) {
			if (i + 1 < argc) {
				lpArgs = charToWCHAR(argv[i + 1], strlen(argv[i + 1]));
				i++;
			}
		}
	}

	if (lpName == L"") return -1;

	ifstream fin;
	fin.open(lpName);

	string tmpTarget = "";
	getline(fin, tmpTarget);
	fin.close();
	lpTarget = charToWCHAR(tmpTarget.c_str(), strlen(tmpTarget.c_str()));

	HINSTANCE h = ShellExecuteW(NULL, L"open", lpTarget, lpArgs, NULL, SW_SHOW);

	if ((int)h != 42) return (int)h;
	else return 0;
}

LPWSTR charToWCHAR(const char* input, int inLen)
{
	const char* cs = input;
	int size = MultiByteToWideChar(CP_UTF8, 0, cs, -1, NULL, 0);
	wchar_t* output = new wchar_t[size];
	MultiByteToWideChar(CP_UTF8, 0, cs, -1, output, size);
	return output;
}

/*
 * Copied from a Stack Overflow post, modified to use Unicode.
 * @see https://stackoverflow.com/questions/1902681/expand-file-names-that-have-environment-variables-in-their-path
 */

// Update the input string
void autoExpandEnvironmentVariables(wstring& text) 
{
	static wregex env(L"\\%(.*?)\\%");
	wsmatch match;
	while (regex_search(text, match, env)) 
	{
		wchar_t** st = new wchar_t*[0];
		size_t* num = new size_t(0);
		_wdupenv_s(st, num, match[1].str().c_str());

		wstring var((*num) == 0 ? L"" : *st);
		text.replace(match[0].first, match[0].second, var);
	}
}

// Leave input alone and return new string
wstring expandEnvironmentVariables(const wstring & input) {
	wstring text = input;
	autoExpandEnvironmentVariables(text);
	return text;
}
