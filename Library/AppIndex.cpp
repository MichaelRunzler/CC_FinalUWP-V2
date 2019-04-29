#include "pch.h"
#include "AppIndex.h"

// Statically initialize the application index at runtime
std::vector<AppRef> AppIndex::appList = []() -> std::vector<AppRef> {return std::vector<AppRef>(); }();


void AppIndex::add(std::wstring& name, std::wstring& relPath) {
	add(AppRef(name, relPath));
}

void AppIndex::add(AppRef& toAdd){
	appList.push_back(toAdd);
}

/// <summary>
/// Gets the AppRef at the specified index.
/// </summary>
/// <param name="index">The index from which to retrieve the object.</param>
/// <returns type="AppRef">The AppRef at the specified index, or a default-constructed 
/// AppRef object if the request was out-of-bounds.</returns>
AppRef AppIndex::getAt(const UINT index)
{
	std::vector<AppRef>::iterator iter = appList.begin();
	if (iter + index < appList.end()) {
		iter += index;
		return *iter;
	}
	
	return AppRef();
}

/// <summary>
/// Inserts the provided reference into the specified index, overwriting the current
/// value at that index in the process.
/// If the specified value is out-of-range, no action is taken.
/// </summary>
/// <param name="index">The index at which to insert the provided value.</param>
/// <param name="toAdd">The AppRef to insert into the index.</param>
void AppIndex::setAt(const UINT index, AppRef toAdd)
{
	std::vector<AppRef>::iterator iter = appList.begin();
	if (iter + index < appList.end()) {
		iter += index;
		*iter = toAdd;
	}
}

/// <summary>
/// Inserts the provided reference into the specified index, overwriting the current
/// value at that index in the process.
/// If the specified value is out-of-range, the provided value is appended to the end of the index vector.
/// </summary>
/// <param name="index">The index at which to insert the provided value.</param>
/// <param name="toAdd">The AppRef to insert into or append to the index.</param>
void AppIndex::setOrAppendAt(const UINT index, AppRef toAdd)
{
	std::vector<AppRef>::iterator iter = appList.begin();
	if (iter + index < appList.end()) {
		iter += index;
		*iter = toAdd;
	}else {
		appList.push_back(toAdd);
	}
}

/// <summary>
/// Removes the entry at the specified index from the app index.
/// </summary>
/// <returns type="bool">A return code of zero (FALSE) means the deletion completed successfully.</returns>
bool AppIndex::remove(const UINT index)
{
	std::vector<AppRef>::iterator ptr = appList.begin();
	ptr += index;
	if (ptr <= appList.end()) {
		appList.erase(ptr);
		return FALSE;
	}

	return TRUE;
}

/// <summary>
/// Removes the entry with the specified name from the index, if it is found.
/// </summary>
/// <returns type="bool">A return code of zero (FALSE) means the deletion completed successfully.</returns>
bool AppIndex::remove(const std::wstring& name)
{
	std::vector<AppRef>::iterator ptr;
	for (ptr = appList.begin(); ptr < appList.end(); ptr++) {
		if (ptr->appName == name) {
			appList.erase(ptr);
			return FALSE;
		}
	}

	return TRUE;
}

/// <summary>
/// Removes the specified AppRef from the index, if it exists.
/// </summary>
/// <returns type="bool">A return code of zero (FALSE) means the deletion completed successfully.</returns>
bool AppIndex::remove(const AppRef& toRemove)
{
	std::vector<AppRef>::iterator ptr;
	for (ptr = appList.begin(); ptr < appList.end(); ptr++) {
		if (*ptr == toRemove) {
			appList.erase(ptr);
			return FALSE;
		}
	}

	return TRUE;
}

void AppIndex::removeAll() {
	appList.clear();
}

std::vector<AppRef>& AppIndex::getList(){
	return appList;
}

/// <summary>
/// Serializes the state of the index and all contained entries into a
/// binary bytestream. 
/// </summary>
/// <returns type="vector">The result of serializing the index. May be zero-length, but not null.</returns>
std::vector<BYTE> AppIndex::serialize()
{
	// Return an empty vector stream if no entries are present
	if (appList.size() == 0) return std::vector<BYTE>();

	// Presize the receiving vector based on the present entries
	size_t size = 0;
	for (auto ptr = appList.begin(); ptr < appList.end(); ptr++) {
		AppRef ar = *ptr;
		size += ar.size();
	}

	// Account for flags at the end of each entry and the final end-of-stream flags
	size += appList.size() * 2;
	size += 2;

	// Presize vector, fill with zeroes
	std::vector<BYTE> retV = std::vector<BYTE>();
	retV.resize(size, 0x00);

	// Cycle through each entry in the index, serialize it, and add it to the stream
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

	// Add end-of-stream demarcation bytes
	AppRef::addFlag(iter, 0xFC);

	return retV;
}

/// <summary>
/// Deserializes a binary datastream into a state for
/// the index and its properties. Subdelegates to the deserialize()
/// method of the AppRef class to deserialize each entry in the index.
/// </summary>
/// <param name="data">The datastream to deserialize.</param>
/// <returns type="UINT">The result code of the deserialization operation.
///  0: Deserialization was successful, all state fields restored
///  1: No data was provided, state set to default
/// -1: Stream incomplete, partial state may have been set.Check before proceeding.
/// </returns>
UINT AppIndex::deserialize(std::vector<BYTE>& data)
{
	// Clear app list and return if the incoming bytestream is empty or NULL
	if (!&data || data.size() == 0) {
		appList = std::vector<AppRef>();
		return 1;
	}

	std::vector<BYTE> buffer = std::vector<BYTE>();
	// Cycle through each byte in the dataset
	bool successful = false;
	for (auto ptr = data.begin(); ptr < data.end(); ptr++)
	{
		// When each end-of-entry demarcation byte sequence is hit, pass the buffer to the deserialization function
		// of the AppRef class, push the result to the list, then dump the buffer
		if (AppRef::checkFlag(ptr, 0xFE)) {
			appList.push_back(AppRef::deserialize(buffer));
			buffer.clear();
			ptr++;
		// When the end-of-stream mark sequence is hit, break the loop
		}else if (AppRef::checkFlag(ptr, 0xFC)) {
			buffer.clear();
			ptr = data.end() - 1;
			successful = true;
		// Push each new byte that isn't a flag to the buffer
		}else
			buffer.push_back(*ptr);
	}

	return successful ? 0 : -1;
}