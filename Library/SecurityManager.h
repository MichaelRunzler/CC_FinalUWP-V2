#pragma once
#include "include.h"

class SecurityManager
{
	/*
	 * BYTEFLAG MAP:
	 * All flags are prepended to 0xFF unless otherwise noted.
	 *
	 * 0xF7: End-of-passcode
	 *		 Preceeded by: passcode (numeric UTF-16 MBS)
	 *		 Followed by: state (byte-coded BOOL flag)
	 *
	 * 0xF6: End-of-stream
	 *		 Preceeded by: state (byte-coded BOOL flag)
	 *		 Followed by: EOS (NULL)
	 *
	 */

private:
	static std::wstring passcode;
	static BOOL state;

public:
	static void setPIN(std::wstring& code);
	static BOOL hasSetPin();
	static BOOL checkState();

	static void lock();
	static BOOL unlock(std::wstring& code);

	static std::vector<BYTE> serialize();
	static UINT deserialize(std::vector<BYTE>& data);
};

