#include "pch.h"
#include "SecurityManager.h"

// Statically initialize state values at runtime
bool SecurityManager::state = []() -> bool {return false; }();
std::wstring SecurityManager::passcode = []() -> std::wstring {return std::wstring(); }();

/// <summary>
/// Sets the current password or PIN.
/// Passwords cannot be set or reset unless the manager is unlocked.
/// </summary>
/// <param name="code">The passcode to set.</param>
void SecurityManager::setPIN(const std::wstring& code)
{
	// Ensure admin access before proceeding
	if (!checkState()) return;

	// If the input string is empty or NULL, simply clear the code
	passcode.clear();
	if (!&code || code.size() == 0) return;

	passcode = code;
}

/// <summary>
/// Checks if the manager has a PIN currently set.
/// </summary>
/// <returns type="bool">TRUE (1) if there is a non-empty PIN set, FALSE (0) if the set PIN is empty.</returns>
bool SecurityManager::hasSetPin(){
	return passcode.size() != 0;
}

/// <summary>
/// Checks what state the security manager is currently in.
/// </summary>
/// <returns type="bool">TRUE (1) if the manager is unlocked, FALSE (0) if it is locked.</returns>
bool SecurityManager::checkState() {
	return state;
}

/// <summary>
/// Locks the manager. If it is already locked, no action is taken.
/// </summary>
void SecurityManager::lock() {
	state = false;
}

/// <summary>
/// Attempts to use the provided PIN code to unlock the manager.
/// If the manager is already unlocked, no action is taken.
/// If the manager has no code set, the manager will unlock regardless of what PIN
/// is provided.
/// </summary>
/// <param name="code">The code to test against the currently set code.</param>
/// <returns type="bool">Returns TRUE (1) if the manager is already unlocked, has no code set, or the code matched the set PIN.
/// If the call returns TRUE, the manager is guaranteed to be unlocked when the call completes. Returns FALSE (0) if the provided
/// PIN did not match the set PIN, and there is a non-empty PIN set. If the call returns FALSE, the manager is guaranteed to have 
/// retained its original state when the call completes. </returns>
bool SecurityManager::unlock(std::wstring& code)
{
	// Default to the unlocked state if no PIN is set or the manager is already unlocked.
	if (state == true || !hasSetPin()) {
		state = true;
		return true;
	}

	// If the code is NULL, or its size does not match the size of the set PIN,
	// it is guaranteed not to match, so we can return FALSE by default.
	if (!&code || code.size() != passcode.size()) return false;

	// If the sizes are the same, run through each character and compare it.
	// If all characters match, unlock and return TRUE.
	std::wstring::iterator ptr = code.begin();
	for (WCHAR c : passcode) {
		if (c != *ptr) return false;
		ptr++;
	}

	state = true;
	return true;
}

/// <summary>
/// Converts this object into a binary bytestream representing its current state.
/// </summary>
/// <returns type="vector">The binary representation of this object's current state. Cannot be NULL, may be zero-length.</returns>
std::vector<BYTE> SecurityManager::serialize()
{
	// Initialize and presize destination vector
	std::vector<BYTE> retV = std::vector<BYTE>();
	retV.resize(((passcode.size() + 2) * 2) + 1, 0x00);
	std::vector<BYTE>::iterator ptr = retV.begin();

	// Serialize passcode if there is one, add marker
	AppRef::toByteStream(passcode, ptr);
	AppRef::addFlag(ptr, 0xF7);

	// Add state flag and end-of-stream bytes
	*ptr = (BYTE)state;
	ptr++;
	AppRef::addFlag(ptr, 0xF6);
	
	return retV;
}

/// <summary>
/// Restores the state of this object from a binary bytestream.
/// </summary>
/// <param name="data">A vector containing the binary data to deserialize.</param>
/// <returns type="UINT">The result of the deserialization operation.
///  0: The operation completed successfully, and the state was restored.
///  1: The incoming datastream was NULL or zero-length, the default state was loaded.
/// -1: The datastream was incomplete or corrupt, a partial state may have been restored.</returns>
UINT SecurityManager::deserialize(std::vector<BYTE>& data)
{
	passcode.clear();

	// Load default state and return if the incoming datastream is NULL or zero-length
	if (!&data || data.size() == 0) {
		state = true;
		return 1;
	}

	std::vector<BYTE> buffer = std::vector<BYTE>();
	// Cycle through each byte in the datastream
	bool valid = false;
	for (auto ptr = data.begin(); ptr < data.end(); ptr++)
	{
		// If the current byte is the end-of-passcode flag, deserialize it and clear the buffer
		if (AppRef::checkFlag(ptr, 0xF7)) 
		{
			AppRef::fromBytes(buffer, &passcode);
			buffer.clear();
			ptr++;
		// If the current byte is the end-of-stream flag, the preceding byte must be the state bit.
		// Check it, set the lock state, clear the buffer, and set the validity flag.
		}else if (AppRef::checkFlag(ptr, 0xF6)) 
		{
			state = (bool) * (ptr - 1);
			ptr = data.end() - 1;
			buffer.clear();
			valid = true;
		// Otherwise, push the current byte to the buffer
		}else
			buffer.push_back(*ptr);
	}

	return valid ? 0 : -1;
}
