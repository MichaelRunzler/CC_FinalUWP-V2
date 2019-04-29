#pragma once
#include "include.h"
#include "AppRef.h"

/// <summary>
/// Stores a serializable list of application references.
/// </summary>
class AppIndex
{
	/*
	 * BYTEFLAG MAP:
	 * All flags are prepended to 0xFF unless otherwise noted.
	 *
	 * 0xFE: End-of-entry
	 *		 Preceeded by: entry in set (AppRef bytestream)
	 *		 Followed by: next entry in set (AppRef bytestream) or basePath (UTF-16 MBS)
	 * 
	 * 0xFC: End-of-stream
	 *		 Preceeded by: basePath (UTF-16 MBS)
	 *		 Followed by: EOS (NULL)
	 *
	 */

private:
	static std::vector<AppRef> appList;

public:
	static void add(std::wstring& name, std::wstring& relPath);
	static void add(AppRef& toAdd);

	static AppRef getAt(const UINT index);
	static void setAt(const UINT index, AppRef toAdd);
	static void setOrAppendAt(const UINT index, AppRef toAdd);

	static bool remove(const UINT index);
	static bool remove(const std::wstring& name);
	static bool remove(const AppRef& toRemove);

	static void removeAll();

	static std::vector<AppRef>& getList();

	static std::vector<BYTE> serialize();
	static UINT deserialize(std::vector<BYTE>& data);
};

