#include "pch.h"
#include "AppRef.h"


AppRef::AppRef() {
	AppRef(L"", L"");
}

AppRef::AppRef(const std::wstring name,const std::wstring path)
{
	AppRef::appName = name;
	AppRef::absolutePath = path;
	AppRef::metadata = std::map<std::wstring, std::wstring>();
}

std::wstring AppRef::toString() {
	return appName + L"; " + absolutePath;
}

std::wstring* AppRef::metaToString(size_t* size)
{
	std::wstring* dest = new std::wstring[metadata.size()];

	UINT i = 0;
	for(std::pair<std::wstring, std::wstring> p : metadata){
		dest[i] = p.first + L": " + p.second;
		i++;
	}

	*size = metadata.size();

	return dest;
}

bool AppRef::operator==(const AppRef& o) {
	return o.appName == appName && o.absolutePath == absolutePath;
}

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

std::vector<BYTE> AppRef::serialize()
{
	std::vector<BYTE> retV = std::vector<BYTE>();
	retV.resize(size(), 0x00);
	auto ptr = retV.begin();

	toByteStream(appName, ptr);
	addFlag(ptr, 0xFD);
	toByteStream(absolutePath, ptr);
	addFlag(ptr, 0xFA);

	for(auto iter = metadata.begin(); iter != metadata.end(); iter++)
	{
		std::wstring key = iter->first;
		toByteStream(key, ptr);

		addFlag(ptr, 0xF9);

		std::wstring val = iter->second;
		toByteStream(val, ptr);

		addFlag(ptr, 0xF8);
	}

	addFlag(ptr, 0xFB);

	return retV;
}

AppRef AppRef::deserialize(std::vector<BYTE>& data)
{
	AppRef retV = AppRef();
	std::vector<BYTE> buffer = std::vector<BYTE>();
	for(auto ptr = data.begin(); ptr < data.end(); ptr++)
	{
		if (checkFlag(ptr, 0xFD)){
			fromBytes(buffer, &retV.appName);
			buffer.clear();
			ptr ++;
		}else if (checkFlag(ptr, 0xFA)) {
			fromBytes(buffer, &retV.absolutePath);
			buffer.clear();
			ptr ++;
		}else if (checkFlag(ptr, 0xFB)) {
			fromBytesM(buffer, retV.metadata);
			buffer.clear();
			ptr = data.end() - 1;
		}
		else
			buffer.push_back(*ptr);
	}

	return retV;
}

void AppRef::toByteStream(std::wstring& input, std::vector<BYTE>::iterator& destPtr)
{
	// Intentionally oversized buffer
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

void AppRef::fromBytes(std::vector<BYTE>& src, std::wstring* dest)
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

void AppRef::fromBytesM(std::vector<BYTE>& src, std::map<std::wstring, std::wstring>& dest)
{
	dest = std::map<std::wstring, std::wstring>();

	std::vector<BYTE> buffer = std::vector<BYTE>();
	std::wstring key = L"";
	std::wstring val = L"";
	for(auto ptr = src.begin(); ptr < src.end(); ptr++)
	{
		if(checkFlag(ptr, 0xF9))
		{
			fromBytes(buffer, &key);
			buffer.clear();
			ptr ++;
		}else if(checkFlag(ptr, 0xF8))
		{
			fromBytes(buffer, &val);
			buffer.clear();
			ptr ++;
			dest.insert_or_assign(key, val);
			key.clear();
			val.clear();
		}else
			buffer.push_back(*ptr);
	}
}

void AppRef::addFlag(std::vector<BYTE>::iterator& ptr, BYTE flag)
{
	*ptr = flag;
	ptr++;
	*ptr = 0xFF;
	ptr++;
}

bool AppRef::checkFlag(std::vector<BYTE>::iterator& ptr, BYTE flag){
	return (*ptr == flag && *(ptr + 1) == 0xFF);
}
