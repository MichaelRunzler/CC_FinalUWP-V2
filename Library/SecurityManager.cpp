#include "pch.h"
#include "SecurityManager.h"

BOOL SecurityManager::state = []() -> BOOL {return false; }();
std::wstring SecurityManager::passcode = []() -> std::wstring {return std::wstring(); }();

void SecurityManager::setPIN(std::wstring& code)
{
	passcode.clear();
	if (code.size() == 0) return;

	passcode = code;
}

BOOL SecurityManager::hasSetPin(){
	return passcode.size() != 0;
}

BOOL SecurityManager::checkState() {
	return state;
}

void SecurityManager::lock() {
	state = false;
}

BOOL SecurityManager::unlock(std::wstring& code)
{
	if (state == TRUE || passcode.size() == 0) {
		state = true;
		return true;
	}

	if (code.size() != passcode.size()) return false;

	std::wstring::iterator ptr = code.begin();
	for (WCHAR c : passcode) {
		if (c != *ptr) return false;
		ptr++;
	}

	state = true;
	return true;
}

std::vector<BYTE> SecurityManager::serialize()
{
	if (passcode.size() == 0) return std::vector<BYTE>();

	std::vector<BYTE> retV = std::vector<BYTE>();
	retV.resize(((passcode.size() + 2) * 2) + 1, 0x00);
	std::vector<BYTE>::iterator ptr = retV.begin();

	AppRef::toByteStream(passcode, ptr);
	AppRef::addFlag(ptr, 0xF7);

	*ptr = (BYTE)state;
	ptr++;
	AppRef::addFlag(ptr, 0xF6);
	
	return retV;
}

UINT SecurityManager::deserialize(std::vector<BYTE>& data)
{
	passcode.clear();

	if (data.size() == 0) {
		state = true;
		return 1;
	}

	std::vector<BYTE> buffer = std::vector<BYTE>();
	bool valid = false;
	for (auto ptr = data.begin(); ptr < data.end(); ptr++)
	{
		switch(*ptr)
		{
		case 0xF7:
			if(*(ptr + 1) == 0xFF){
				AppRef::fromBytes(buffer, &passcode);
				buffer.clear();
				ptr += 2;
			}
			break;
		case 0xF6:
			if(*(ptr + 1) == 0xFF){
				state = (BOOL) * (ptr - 1);
				ptr = data.end() - 1;
				buffer.clear();
				valid = true;
			}
			break;
		default:
			buffer.push_back(*ptr);
			break;
		}
	}

	return valid ? 0 : -1;
}
