#pragma once
namespace FinalUWP 
{
	/// <summary>
	/// Provides cross-class access to app state storage and restoration functionality.
	/// </summary>
	public ref class ConfigRelay sealed
	{
	public:
		static property Platform::String^ SETTINGS_ID {
			Platform::String^ get(){ return settingsID; }
		}

		static property Platform::String^ SECURITY_ID {
			Platform::String^ get() { return securityID; }
		}

		static property Platform::String^ APPLIST_ID {
			Platform::String^ get() { return applistID; }
		}

		static void Save();
		static void Restore();

	private:
		static Platform::String^ settingsID;
		static Platform::String^ securityID;
		static Platform::String^ applistID;
	};
}
