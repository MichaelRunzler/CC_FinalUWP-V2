
#include "UWPProcessLauncher.h"

using namespace std;

/*
 * The main method executes most of the program logic within a shell method.
 * It does this so that it can capture the return code and write it to a file
 * if it has been requested to do so.
 */
int main(int argc, char* argv[])
{
	// Run main program logic, capture exit code
	int result = process(argc, argv);

	// If the '/log' option with a valid file was given,
	// log the result code to the given file
	if (lpResult != L"") 
	{
		ofstream fout;
		fout.open(lpResult, ios::trunc);
		fout << result;
		fout.flush();
		fout.close();
	}
	
	// Pass the result code back up to the rest of the Win32 API
	// as normal
	return result;
}

int process(const int argc, char* argv[])
{
	// Argument constants
	const wstring ARG_NAME =    L"/name";    // Launch the executable with the specified name
	const wstring ARG_FILE =    L"/file";    // Read additional execution or backup information from the file with this name
	const wstring ARG_PARAMS =  L"/params";  // Pass these parameters to the executed file if in execution mode
	const wstring ARG_RESCODE = L"/log";     // Save exit status code data to this file
	const wstring ARG_BACKUP =  L"/backup";  // Backup a copy of the file specified in the second line of the file specified by
										     // the /file argument to the file specified in the first line of that file
	const wstring ARG_RESTORE = L"/restore"; // Same as above, but in reverse
	const wstring ARG_DELAY =   L"/delay";   // Delays the selected operation by the specified number of milliseconds

	// State flags
	STARTUPINFO si = { sizeof(STARTUPINFO), 0 };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = { 0 };
	wstring lpName = L""; // Name of the file from which the target name will be read
	LPWSTR lpBackup = NULL; // Source file that backup information will be read from, or restore
							  // information will be saved to
	byte backup = 0; // Will initiate a backup operation if nonzero
	LPWSTR lpTarget = NULL; // The file to execute if in execution mode,
							// or the file that backup data will be read from or restore information
							// will be saved to if in restore mode
	LPWSTR lpArgs = NULL; // Arguments to be passed to the executed file if in execution mode
	LPWSTR lpDir = NULL;
	int delay = 0;

	// Print short message to the console detailing what this program is in case someone starts it up by itself
	cout << "ARK Software Win64 Process Launcher/Backup Utility for the Universal Windows Platform" << endl;
	cout << "Distributed with the ARK Application Launcher Platform (UWP)" << endl << endl;
	cout << "This program is basically useless without the main app." << endl << "Feel free to run it, but don't expect anything exciting." << endl;
	cout << "Subroutine version 2.12, main app version 2.00" << endl;
	cout << "(c) 2018-2019 Ethan R. Scott/ARK Software IRC." << endl;
	cout << "Property of Yosemite Community College District." << endl << endl;

	// Run through argument list and check for any known args
	LPWSTR result = NULL; // Cache variable for extracted arguments
	for (int i = 0; i < argc; i++)
	{
		// Get current argument from the array, convert to UTF-16
		LPWSTR arg = charToWCHAR(argv[i], strlen(argv[i]));

		// /backup and /restore have no data passed to them, so we just check if they exist and move on
		if (arg == ARG_BACKUP) {
			backup = 1;
		}
		else if (arg == ARG_RESTORE) {
			backup = 2;
		}
		// For all arguments that have data passed to them, check if they exist,
		// and if they do, extract their data and store it in an appropriate form
		// to one of the state variables
		else if ((result = checkArg(argv, argc, i, ARG_NAME)) != NULL) {
			lpTarget = result;
			i++;
		}
		else if ((result = checkArg(argv, argc, i, ARG_FILE)) != NULL) {
			lpName = expandEnvironmentVariables(result);
			i++;
		}
		else if ((result = checkArg(argv, argc, i, ARG_RESCODE)) != NULL) {
			lpResult = expandEnvironmentVariables(result);
			i++;
		}
		else if ((result = checkArg(argv, argc, i, ARG_PARAMS)) != NULL) {
			lpArgs = result;
			i++;
		}
		else if ((result = checkArg(argv, argc, i, ARG_DELAY)) != NULL) {
			if (isdigit(result[0])) {
				delay = atoi(argv[i + 1]);
				i++;
			}
		}
	}

	// Either a target name or a file specifying a target name is required for all
	// operations. If neither one was given, return straight away with an error code of -1.
	if (lpName == L"" && !lpTarget) return -1;

	// Lookup file for target if no target argument was given
	if (!lpTarget)
	{
		// Open a stream to the specified file
		ifstream fin;
		fin.open(lpName);

		// Return with an error if the file does not exist or could not be opened
		if (fin.fail()) return 1;

		// Read the target from the first line in the name file
		string tmpTarget = "";
		getline(fin, tmpTarget);
		lpTarget = charToWCHAR(tmpTarget.c_str(), strlen(tmpTarget.c_str()));

		// Re-use cache string for working path of target
		tmpTarget = tmpTarget.substr(0, tmpTarget.find_last_of('\\'));
		lpDir = charToWCHAR(tmpTarget.c_str(), strlen(tmpTarget.c_str()));

		// If a backup or restore operation was specified, read the target for that
		// operation from the second line in the name file
		if (backup) {
			getline(fin, tmpTarget);
			lpBackup = charToWCHAR(tmpTarget.c_str(), strlen(tmpTarget.c_str()));
		}

		// Release the lock on the name file
		fin.close();
	}

	// Delay if requested
	if (delay > 0) {
		cout << "Delaying execution for " << delay << "ms..." << endl;
		this_thread::sleep_for(chrono::milliseconds(delay));
	}

	// If no backup operation is requested, launch target file
	if (!backup) 
	{
		// In this case, the HINSTANCE handle is actually a result code, so we store it
		// and check it for a result.
		HINSTANCE h = ShellExecuteW(NULL, L"open", lpTarget, lpArgs, lpDir, SW_SHOW);

		// For some reason, the 'success' result code is 42, all others except 0 are failure codes.
		// Treat 42 as 0 (HRESULT_SUCCESS), all others, push as nonzero exit codes to the shell method.
		if ((int)h == 0 || (int)h == 42) return 0;
		else return (int)h;
	}else 
	{
		ofstream fout;
		ifstream fin;

		// Open a read stream to the backup source (or restore source)
		fin.open(backup == 1 ? lpBackup : lpTarget, ios::binary);
		// If the read stream fails, return BEFORE opening the write stream.
		// The write stream is in TRUNC mode, so if we opened both (and it turned out that
		// the source file was inaccessible) the target would be wiped and no new data would
		// be available to replace it.
		if (fin.fail()) return 1; 

		// Open a write stream to the backup target (or restore target)
		fout.open(backup == 1 ? lpTarget : lpBackup, ios::binary | ios::trunc);
		// If the output stream fails to initialize (for example, if the target is already locked),
		// abort copy.
		if (fout.fail()) return 1;

		// Run a buffer-to-buffer copy from the input stream to the output stream
		fout << fin.rdbuf();

		// Flush and close the output stream, release the lock on the input file
		fout.flush();
		fout.close();
		fin.close();
	}

	return 0;
}

/*
 * Checks if the argument specified by 'flag' is equal to the argument at argv[i]. If it is,
 * and the end of the argument buffer has not yet been reached, retrieve the data passed to that argument
 * and return it as a UTF-16 string pointer. Otherwise, nullptr is returned.
 */
LPWSTR checkArg(char** argv, const int argc, int i, const wstring flag)
{
	if (charToWCHAR(argv[i], strlen(argv[i])) == flag && i + 1 < argc) return charToWCHAR(argv[i + 1], strlen(argv[i + 1]));
	else return nullptr;
}

/*
 * Converts an ASCII char*[] to a 64-bit pointer to a UTF-16 string.
 * Will cause memory access violations if the length specified by
 * 'inLen' is not the actual length of 'input'.
 */
LPWSTR charToWCHAR(const char* input, int inLen)
{
	const char* cs = input; // Get a const pointer to the input buffer
	int size = MultiByteToWideChar(CP_UTF8, 0, cs, -1, NULL, 0); // Presize a destination buffer for the MBTOWC conversion
	wchar_t* output = new wchar_t[size];
	MultiByteToWideChar(CP_UTF8, 0, cs, -1, output, size); // Run conversion
	return output; // Return the resulting wchar_t*[] as a LPWSTR
}

/*
 * Copied from a Stack Overflow post, modified to use Unicode.
 * @see https://stackoverflow.com/questions/1902681/expand-file-names-that-have-environment-variables-in-their-path
 */

// Update the input string
void autoExpandEnvironmentVariables(wstring& text) 
{
	static wregex env(L"\\%(.*?)\\%"); // Search for any pair of %s and the data between them
	wsmatch match;
	while (regex_search(text, match, env)) // While there are still matches for this sequence:
	{
		wchar_t** st = new wchar_t*[0]; // Allocate a dummy buffer
		size_t* num = new size_t(0); // Allocate a pointer for size return
		_wdupenv_s(st, num, match[1].str().c_str()); // Grab a UTF-16 environment variable using the allocated buffers

		// If no variable was found that matched the extracted string, use an empty UTF-16 string
		// as a default replacement (with the result being that the environment variable would be
		// removed instead of being expanded).
		wstring var(((*num) == 0 ? L"" : *st));
		// Insert the expanded variable back into the string
		text.replace(match[0].first, match[0].second, var);
	}
}

// Leave input alone and return new string
wstring expandEnvironmentVariables(const wstring& input) {
	wstring text = input;
	autoExpandEnvironmentVariables(text);
	return text;
}
