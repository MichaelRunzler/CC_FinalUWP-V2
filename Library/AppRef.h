#pragma once
#include "include.h"
#include <stdlib.h>

class AppRef
{
	/*
	 * BYTEFLAG MAP:
	 * All flags are prepended to 0xFF unless otherwise noted.
	 * 
	 * 0xFD: End-of-name
	 *		 Preceeded by: appName (UTF-16 MBS)
	 *		 Followed by: absolutePath (UTF-16 MBS)
	 * 
	 * 0xFA: End-of-path
	 *		 Preceeded by: absolutePath (UTF-16 MBS)
	 *		 Followed by: metadata (ordered map of paired UTF-16 MBS)
	 *					  or end-of-stream marker (2-byte binary flag)
	 * 
	 * 0xF9: End-of-key
	 *		 Preceeded by: key in map pair (UTF-16 MBS)
	 *		 Followed by: value in map pair (UTF-16 MBS)
	 * 
	 * 0xF8: End-of-pair
	 *		 Preceeded by: value in map pair (UTF-16 MBS)
	 *		 Followed by: next pair in map (UTF-16 MBS) or end-of-stream marker (2-byte binary flag)
	 * 
	 * 0xFB: End-of-stream
	 *		 Preceeded by: last value in metadata map (UTF-16 MBS member of key-value pair)
	 *					   or end-of-path (UTF-16 MBS)
	 *		 Followed by: EOS (NULL)
	 * 
	 */

public:
	std::wstring appName;
	std::wstring absolutePath;
	std::map<std::wstring, std::wstring> metadata;

	AppRef();
	AppRef(std::wstring name, std::wstring path);

	std::wstring toString();
	std::wstring* metaToString(size_t* size);

	SIZE_T size();

	bool operator==(const AppRef& o);

	std::vector<BYTE> serialize();
	static AppRef deserialize(std::vector<BYTE>& data);

	static void toByteStream(std::wstring& input, std::vector<BYTE>::iterator& dest);
	static void fromBytes(std::vector<BYTE>& src, std::wstring* dest);
	static void addFlag(std::vector<BYTE>::iterator& ptr, BYTE flag);
	static bool checkFlag(std::vector<BYTE>::iterator& ptr, BYTE flag);
	static void fromBytesM(std::vector<BYTE>& src, std::map<std::wstring, std::wstring>& dest);
};

