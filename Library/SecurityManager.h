#pragma once
#include "include.h"

/// <summary>
/// Stores security and management state information for the app.
/// </summary>
class SecurityManager
{
	/*
	 * BYTEFLAG MAP:
	 * All flags are prepended to 0xFF unless otherwise noted.
	 *
	 * 0xF7: End-of-passcode
	 *		 Preceeded by: passcode (numeric UTF-16 MBS)
	 *		 Followed by: state (byte-coded bool flag)
	 *
	 * 0xF6: End-of-stream
	 *		 Preceeded by: state (byte-coded bool flag)
	 *		 Followed by: EOS (NULL)
	 *
	 */

private:
	static std::wstring passcode; // Ideally numeric-only, however, not specifically limited as such
	static bool state; // TRUE (1) is unlocked (Admin), FALSE (0) is locked (NoAdmin)

public:
	static void setPIN(const std::wstring& code);
	static bool hasSetPin();
	static bool checkState();

	static void lock();
	static bool unlock(std::wstring& code);

	static std::vector<BYTE> serialize();
	static UINT deserialize(std::vector<BYTE>& data);
};

