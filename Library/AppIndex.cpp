#include "pch.h"
#include "AppIndex.h"

std::wstring AppIndex::basePath = []() -> std::wstring {return L"";}();
std::vector<AppRef> AppIndex::appList = []() -> std::vector<AppRef> {return std::vector<AppRef>(); }();
UINT AppIndex::runningPID = []() -> UINT {return -1; }();


void AppIndex::setBasePath(std::wstring& path) {
	basePath = path;
}

void AppIndex::add(std::wstring& name, std::wstring& relPath) {
	add(AppRef(name, basePath + L'\\' + relPath));
}

void AppIndex::add(AppRef& toAdd){
	appList.push_back(toAdd);
}

void AppIndex::remove(UINT index)
{
	std::vector<AppRef>::iterator ptr = appList.begin();
	ptr += index;
	if (ptr > appList.end())
		SetLastError(ERROR_NOT_FOUND);
	else
		appList.erase(ptr);
}

void AppIndex::remove(std::wstring& name)
{
	std::vector<AppRef>::iterator ptr;
	for (ptr = appList.begin(); ptr < appList.end(); ptr++) {
		if (ptr->appName == name) {
			appList.erase(ptr);
			return;
		}
	}

	SetLastError(ERROR_NOT_FOUND);
}

void AppIndex::remove(AppRef& toRemove)
{
	std::vector<AppRef>::iterator ptr;
	for (ptr = appList.begin(); ptr < appList.end(); ptr++) {
		if (*ptr == toRemove) {
			appList.erase(ptr);
			return;
		}
	}

	SetLastError(ERROR_NOT_FOUND);
}

void AppIndex::removeAll() {
	appList.clear();
}

std::vector<AppRef>& AppIndex::getList(){
	return appList;
}

std::vector<BYTE> AppIndex::serialize()
{
	// Return an empty vector stream if no entries are present and no base path is set
	if (appList.size() == 0 && basePath.size() == 0) return std::vector<BYTE>();

	// Presize the receiving vector based on the present entries
	size_t size = 0;
	for (auto ptr = appList.begin(); ptr < appList.end(); ptr++) {
		AppRef ar = *ptr;
		size += ar.size();
	}

	size += basePath.size() + 2;
	size += appList.size() * 2;

	// Presize vector, fill with zeroes
	std::vector<BYTE> retV = std::vector<BYTE>();
	retV.resize(size, 0x00);

	// Cycle through each entry in the index, serialize it, add it to the stream
	std::vector<BYTE>::iterator iter = retV.begin();
	for (auto ptr = appList.begin(); ptr < appList.end(); ptr++) {
		AppRef ar = *ptr;
		for(BYTE b : ar.serialize()){
			*iter = b;
			iter++;
		}

		// Add entry demarcation bytes
		AppRef::addFlag(iter, 0xFE);
	}

	// Serialize base path characters, add to stream along with end-of-stream demarcation bytes
	AppRef::toByteStream(basePath, iter);
	AppRef::addFlag(iter, 0xFC);

	return retV;
}

UINT AppIndex::deserialize(std::vector<BYTE>& data)
{
	// Clear app list and return if the incoming bytestream is empty
	if (data.size() == 0) {
		appList = std::vector<AppRef>();
		return 1;
	}

	// Cycle through each byte in the dataset
	std::vector<BYTE> buffer = std::vector<BYTE>();
	bool successful = false;
	for (auto ptr = data.begin(); ptr < data.end(); ptr++)
	{
		switch (*ptr)
		{
		// When the end-of-entry demarcation byte sequence is hit, pass the buffer to the deserialization function
		// of the AppRef class, push the result to the list, then dump the buffer
		case 0xFE:
			if(*(ptr + 1) == 0xFF){
				appList.push_back(AppRef::deserialize(buffer));
				buffer.clear();
				ptr ++;
			}
			break;
		// When the end-of-stream mark sequence is hit, serialize the last contents of the buffer as a string,
		// copy it to the base path, then break the loop
		case 0xFC:
			if (*(ptr + 1) == 0xFF) {
				AppRef::fromBytes(buffer, &basePath);
				buffer.clear();
				ptr = data.end() - 1;
				successful = true;
			}
			break;
		// Push each new byte that isn't a flag to the buffer
		default:
			buffer.push_back(*ptr);
			break;
		}
	}

	return successful ? 0 : -1;
}