#include "pch.h"
#include "AppRef.h"


AppRef::AppRef() {
	AppRef(std::wstring(), std::wstring());
}

AppRef::AppRef(std::wstring& name, std::wstring& path)
{
	AppRef::appName = name;
	AppRef::absolutePath = path;
	AppRef::metadata = std::map<std::wstring, std::wstring>();
}

std::wstring AppRef::toString() {
	return appName + L"; " + absolutePath;
}

/// <summary>
/// Converts each metadata entry in the reference's metadata store
/// into an entry in the returned array.
/// </summary>
/// <param name="size">A pointer to an unsigned 64-bit container that will receive the size of the returned array.</param>
/// <returns type="wstring*"/>A pointer to an array of wstrings, where each entry corresponds to a metadata entry.</returns>
std::wstring* AppRef::metaToString(size_t* size)
{
	// Allocate buffer
	std::wstring* dest = new std::wstring[metadata.size()];

	// Combine each key/value metadata pair into a single string and write it to the buffer
	UINT i = 0;
	for(std::pair<std::wstring, std::wstring> p : metadata){
		dest[i] = p.first + L": " + p.second;
		i++;
	}

	// Update provided sizing pointer
	*size = metadata.size();

	return dest;
}

bool AppRef::operator==(const AppRef& o) {
	return o.appName == appName && o.absolutePath == absolutePath;
}

/// <summary>
/// Gets the number of bytes required to store the serialized representation of this
/// object in its current state.
/// </summary>
/// <returns type="size_t">The size of this object's serialized form in bytes.</returns>
SIZE_T AppRef::size()
{
	// 2 bytes per character of name and path, 3 flags (EoN, EoP, EoS) at 2 bytes each
	SIZE_T size = (appName.size() + absolutePath.size() + 3) * 2;
	for(auto ptr = metadata.begin(); ptr != metadata.end(); ptr++){
		// 2 bytes per character of key and value, 2 flags (EoK, EoV) at 2 bytes each
		size += (ptr->first.size() + ptr->second.size() + 2) * 2;
	}

	return size;
}

/// <summary>
/// Converts this object into a binary bytestream containing its state.
/// </summary>
/// <returns type="vector">The serialized form of this object. Cannot be zero-length or NULL.</returns>
std::vector<BYTE> AppRef::serialize()
{
	// Allocate and presize destination vector and obtain pointer to its start
	std::vector<BYTE> retV = std::vector<BYTE>();
	retV.resize(size(), 0x00);
	auto ptr = retV.begin();

	// Serialize name and path, add appropriate flags
	toByteStream(appName, ptr);
	addFlag(ptr, 0xFD);
	toByteStream(absolutePath, ptr);
	addFlag(ptr, 0xFA);

	// For each entry in the metadata store, serialize its key and value in sequence,
	// and add demarcation flags
	for(auto iter = metadata.begin(); iter != metadata.end(); iter++)
	{
		std::wstring key = iter->first;
		toByteStream(key, ptr);

		addFlag(ptr, 0xF9);

		std::wstring val = iter->second;
		toByteStream(val, ptr);

		addFlag(ptr, 0xF8);
	}

	// Add end-of-stream flag
	addFlag(ptr, 0xFB);

	return retV;
}

/// <summary>
/// Generates an AppRef object from a serialized state stream.
/// </summary>
/// <param name="data">The data to deserialize.</param>
/// <returns type="AppRef">The resultant AppRef value from deserialization. Cannot be NULL.</returns>
AppRef AppRef::deserialize(std::vector<BYTE>& data)
{
	// Initialize destination object. If the data is zero-length or NULL, this will be used as the return value.
	AppRef retV = AppRef();

	// Return a default-constructed AppRef if the incoming data is NULL
	if (!&data) return retV;

	// Cycle through each source byte
	std::vector<BYTE> buffer = std::vector<BYTE>();
	for(auto ptr = data.begin(); ptr < data.end(); ptr++)
	{
		// If the current byte is the end-of-name flag, deserialize the name and clear the buffer
		if (checkFlag(ptr, 0xFD)){
			fromBytes(buffer, &retV.appName);
			buffer.clear();
			ptr ++;
		// If the current byte is the end-of-path flag, deserialize it and clear the buffer
		}else if (checkFlag(ptr, 0xFA)) {
			fromBytes(buffer, &retV.absolutePath);
			buffer.clear();
			ptr ++;
		// If the current byte is the end-of-stream flag, the preceding bytes must be the metadata,
		// if there is any. Send it to its sub-routine and clear the buffer once done.
		// Ignore any further bytes in the stream if there are any.
		}else if (checkFlag(ptr, 0xFB)) {
			fromBytesM(buffer, &(retV.metadata));
			buffer.clear();
			ptr = data.end() - 1;
		}
		// Otheriwse, push the current byte to the buffer and continue
		else
			buffer.push_back(*ptr);
	}

	return retV;
}

/// <summary>
/// Converts a UTF-16 string (wstring) into a multibyte binary stream.
/// </summary>
/// <param name="input">The wide string to serialize.</param>
/// <param name="destPtr">A pointer to a position in a byte vector to receive the result of the deserialization.
/// The iterator will be incremented during the course of this method, such that it will point to the position
/// after the null-terminator character in the serialized multibyte sequence.</param>-
void AppRef::toByteStream(const std::wstring& input, std::vector<BYTE>::iterator& destPtr)
{
	// Buffer sufficient to hold 2 characters (16 bytes, one UTF-16 character)
	char buffer[2];

	// For every character in the input string:
	for(WCHAR c : input)
	{
		// Convert to multibyte sequence, store to buffer
		int count = 0;
		wctomb_s(&count, buffer, 4, c);

		// Pad buffer if the source character only converted to one byte
		if (count < 2) { 
			buffer[1] = buffer[0];
			count = 2;
		}

		// Copy buffer to the destination vector
		for(int i = 0; i < count; i++){
			*destPtr = buffer[i];
			destPtr++;
		}
	}
}

/// <summary>
/// Reconstructs a UTF-16 character sequence from a multibyte-encoded binary stream.
/// </summary>
/// <param name="src">The vector containing the bytes to deserialize.</param>
/// <param name="dest">A pointer to a wstring object which will have the result of the deserialization appended to it.</param>
void AppRef::fromBytes(const std::vector<BYTE>& src, std::wstring* dest)
{
	// Receptacle for processed characters
	WCHAR buffer = L' ';
	// Temporary buffer for byte pairs read from the vector stream
	char sample[2];
	// For each pair of bytes in the input stream:
	for(auto ptr = src.begin(); ptr < src.end(); ptr += 2)
	{
		// Copy pair to buffer
		sample[0] = *ptr;
		sample[1] = *(ptr + 1);

		// Convert to wide character, store to buffer character
		UINT res = std::mbtowc(&buffer, sample, 2);
		// Copy to destination string
		if (res > 0) *dest += buffer;
	}
}

/// <summary>
/// Reconstructs a key-value map of UTF-16 strings from a multibyte-encoded binary entry stream.
/// </summary>
/// <param name="src">The vector containing the bytes to deserialize.</param>
/// <param name="dest">A pointer which will be initialized and used as a destination for the deserialized map entries.</param>
void AppRef::fromBytesM(std::vector<BYTE>& src, std::map<std::wstring, std::wstring>* dest)
{
	// Initialize default map; this will be used as the default value in case of zero-length input
	*dest = std::map<std::wstring, std::wstring>();

	std::vector<BYTE> buffer = std::vector<BYTE>();
	std::wstring key = L"";
	std::wstring val = L"";

	// For each byte in the input stream:
	for(auto ptr = src.begin(); ptr < src.end(); ptr++)
	{
		// If the current byte is the end-of-key flag, deserialize the key and store it
		if(checkFlag(ptr, 0xF9))
		{
			fromBytes(buffer, &key);
			buffer.clear();
			ptr ++;
		// If the current byte is the end-of-pair flag, deserialize the value, store it,
		// add the key/value pair to the map, and clear the buffer and the temporary key/value
		// buffers.
		}else if(checkFlag(ptr, 0xF8))
		{
			fromBytes(buffer, &val);
			buffer.clear();
			ptr ++;
			(*dest).insert_or_assign(key, val);
			key.clear();
			val.clear();
		// Otherwise, push the current byte to the buffer
		}else
			buffer.push_back(*ptr);
	}
}

/// <summary>
/// Adds a two-byte binary flag to the provided vector.
/// The complete flag consists of the provided byte with 0xFF appended to it.
/// </summary>
/// <param name="ptr">A pointer to the position in a byte vector which will have the flags inserted into it. 
/// The iterator is incremented twice duing the course of this method, such that it will point at the position 
/// after the inserted flag.</param>
/// <param name="flag">A binary single-byte value that will be used as the first byte in the flag.</param>
void AppRef::addFlag(std::vector<BYTE>::iterator& ptr, const BYTE flag)
{
	*ptr = flag;
	ptr++;
	*ptr = 0xFF;
	ptr++;
}

/// <summary>
/// Checks to see if the reference bytes in the provided vector are equal to the provided byte flag
/// value.
/// </summary>
/// <param name="ptr">A pointer to the position in a byte vector which corresponds to the start of the byte
/// sequence to be checked. This iterator will not be incremented or decremented during the course of this
/// method, only dereferenced twice.</param>
/// <param name="flag">The first byte of the flag pair to check. The second compared byte is assumed to be 0xFF.</param>
/// <returns type="bool">TRUE (1) if the bytes at the pointer position and the pointer position +1 equal the byteflag sequence
/// produced by passing the "flag" parameter to addFlag(), FALSE (0) otherwise.
bool AppRef::checkFlag(std::vector<BYTE>::iterator& ptr, const BYTE flag){
	return (*ptr == flag && *(ptr + 1) == 0xFF);
}
