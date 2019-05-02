#include "pch.h"
#include "ConfigRelay.h"

using namespace Platform;
using namespace Windows::Storage;

String^ FinalUWP::ConfigRelay::settingsID = "masterSettings";
String^ FinalUWP::ConfigRelay::securityID = "secPol";
String^ FinalUWP::ConfigRelay::applistID = "appStorage";

/// <summary>
/// Stores the current Security Manager and App Index states to the current
/// ApplicationDataContainer store.
/// </summary>
void FinalUWP::ConfigRelay::Save()
{
	// Load appdata settings repository

	ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;

	// Data values cannot be stored as binary streams in the settings
	// repository, so we must convert them to Unicode strings first.

	// Serialize states for the app index and security manager
	std::vector<BYTE> tmpA = AppIndex::serialize();
	std::vector<BYTE> tmpS = SecurityManager::serialize();

	// Initialize empty intermediary string containers for conversion
	std::wstring appStr = std::wstring();
	std::wstring secStr = std::wstring();

	// Convert each byte into a UTF-16 multibyte character representation
	// of its value via a simple expansion cast
	for (BYTE b : tmpA)
		appStr += (char)b;

	// Repeat for the other datastream
	for (BYTE b : tmpS)
		secStr += (char)b;

	// Store converted values as Platform::Strings into the settings repository under
	// their own subkeys
	ApplicationDataCompositeValue ^ comp = ref new ApplicationDataCompositeValue();
	comp->Insert(applistID, ref new String(appStr.c_str()));
	comp->Insert(securityID, ref new String(secStr.c_str()));

	// Push new keys to the repository
	localSettings->Values->Insert(settingsID, comp);
}

/// <summary>
/// Restores the SecurityManager and AppIndex states from the current
/// ApplicationDataContainer store if there are saved states. Otherwise,
/// resets their states to default.
/// </summary>
void FinalUWP::ConfigRelay::Restore()
{
	// Load appdata settings repository and attempt to load keys for security and app reference data

	ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
	ApplicationDataCompositeValue^ composite = safe_cast<ApplicationDataCompositeValue^>(localSettings->Values->Lookup(ConfigRelay::SETTINGS_ID));

	// Load empty keys into the settings stream if the master settings key is empty
	if (!composite) {
		composite = ref new ApplicationDataCompositeValue();
		composite->Insert(ConfigRelay::APPLIST_ID, ref new String());
		composite->Insert(ConfigRelay::SECURITY_ID, ref new String());
	}

	// TODO Check if a null value on one of the data strings causes a crash

	// Load app list data into a String.
	// The actual datastream is binary, but it has to be stored as a String due
	// to restrictions imposed by the ApplicationDataStore.
	String^ data = safe_cast<String^>(composite->Lookup(ConfigRelay::APPLIST_ID));
	const wchar_t* bytes = data->Data(); // Extract char array from string
	std::vector<BYTE> vec = std::vector<BYTE>(); // Initialize target bytestream container
	vec.resize(data->Length(), 0x00); // Presize vector container

	// Copy characters from the array to the buffer, truncating to 8 bits from 16.
	// The byte that's truncated should always be zero, since the data values going
	// in when saved are always < 256.
	std::vector<BYTE>::iterator iter = vec.begin();
	for (UINT i = 0; i < data->Length(); i++) {
		*iter = (BYTE)bytes[i];
		iter++;
	}

	// Pass bytestream to the app index for deserialization,
	// it will gracefully deal with any errors.
	AppIndex::deserialize(vec);

	// Repeat process for the security database

	data = safe_cast<String^>(composite->Lookup(ConfigRelay::SECURITY_ID));
	const wchar_t* bytesS = data->Data();
	vec.clear();
	vec.resize(data->Length(), 0x00);

	iter = vec.begin();
	for (UINT i = 0; i < data->Length(); i++) {
		*iter = (BYTE)bytesS[i];
		iter++;
	}

	SecurityManager::deserialize(vec);
}
