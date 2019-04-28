#pragma once
#include "include.h"
#include "AppRef.h"

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
	 * 0xFC: End-of-path
	 *		 Preceeded by: basePath (UTF-16 MBS)
	 *		 Followed by: EOS (NULL)
	 *
	 */

private:
	static std::wstring basePath;
	static std::vector<AppRef> appList;

public:
	static void setBasePath(std::wstring& path);

	static void add(std::wstring& name, std::wstring& relPath);
	static void add(AppRef& toAdd);

	static void remove(UINT index);
	static void remove(std::wstring& name);
	static void remove(AppRef& toRemove);

	static void removeAll();

	static std::vector<AppRef>& getList();

	static std::vector<BYTE> serialize();

	/*
	 * NAME: Deserialize
	 * PURPOSE: Deserializes a state set for the app index from a bytestream
	 * ARGUMENTS:
	 *		data (vector<BYTE>&): a vector of raw binary data to be used to restore the app index state
	 *
	 * RETURNS: A status code from the following list:
	 *			0: Deserialization was successful, all state fields restored
	 *			1: No data was provided, state set to default
	 *		   -1: Stream incomplete, partial state may have been set. Check before proceeding.
	 */
	static UINT deserialize(std::vector<BYTE>& data);
};

