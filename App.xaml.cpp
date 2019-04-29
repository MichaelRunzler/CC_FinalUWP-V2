//
// App.xaml.cpp
// Implementation of the App class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace FinalUWP;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

// Constants for local settings keys

static String^ const SETTINGS_ID = "masterSettings";
static String^ const SECURITY_ID = "secPol";
static String^ const APPLIST_ID = "appStorage";

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
    InitializeComponent();
    Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
}

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(LaunchActivatedEventArgs^ e)
{
    auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

    // Do not repeat app initialization when the Window already has content,
    // just ensure that the window is active
    if (rootFrame == nullptr)
    {
        // Create a Frame to act as the navigation context and associate it with
        // a SuspensionManager key
        rootFrame = ref new Frame();

        rootFrame->NavigationFailed += ref new NavigationFailedEventHandler(this, &App::OnNavigationFailed);

		//
		// APP STATE LOADER
		//

		// Load appdata settings repository and attempt to load keys for security and app reference data

		ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
		ApplicationDataCompositeValue^ composite = safe_cast<ApplicationDataCompositeValue^>(localSettings->Values->Lookup(SETTINGS_ID));

		// Load empty keys into the settings stream if the master settings key is empty
		if (!composite) {
			composite = ref new ApplicationDataCompositeValue();
			composite->Insert(APPLIST_ID, ref new String());
			composite->Insert(SECURITY_ID, ref new String());
		}

		// TODO Check if a null value on one of the data strings causes a crash

		// Load app list data into a String.
		// The actual datastream is binary, but it has to be stored as a String due
		// to restrictions imposed by the ApplicationDataStore.
		String^ data = safe_cast<String^>(composite->Lookup(APPLIST_ID));
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

		data = safe_cast<String^>(composite->Lookup(SECURITY_ID));
		const wchar_t* bytesS = data->Data();
		vec.clear();
		vec.resize(data->Length(), 0x00);

		iter = vec.begin();
		for (UINT i = 0; i < data->Length(); i++) {
			*iter = (BYTE)bytesS[i];
			iter++;
		}

		SecurityManager::deserialize(vec);

		//
		// END STATE LOADER
		//

        if (e->PrelaunchActivated == false)
        {
            if (rootFrame->Content == nullptr)
            {
                // When the navigation stack isn't restored navigate to the first page,
                // configuring the new page by passing required information as a navigation
                // parameter
                rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
            }
            // Place the frame in the current Window
            Window::Current->Content = rootFrame;

			//Windows::UI::ViewManagement::ApplicationView::PreferredLaunchWindowingMode = Windows::UI::ViewManagement::ApplicationViewWindowingMode::FullScreen;

            // Ensure the current window is active
            Window::Current->Activate();
        }
    } else
    {
        if (e->PrelaunchActivated == false)
        {
            if (rootFrame->Content == nullptr)
            {
                // When the navigation stack isn't restored navigate to the first page,
                // configuring the new page by passing required information as a navigation
                // parameter
                rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
            }
            // Ensure the current window is active
            Window::Current->Activate();
        }
    }
}


/// <summary>
/// Invoked when application execution is being suspended.  Application state is saved
/// without knowing whether the application will be terminated or resumed with the contents
/// of memory still intact.
/// </summary>
/// <param name="sender">The source of the suspend request.</param>
/// <param name="e">Details about the suspend request.</param>
void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
    (void) sender;  // Unused parameter
    (void) e;   // Unused parameter

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
	ApplicationDataCompositeValue^ comp = ref new ApplicationDataCompositeValue();
	comp->Insert(APPLIST_ID, ref new String(appStr.c_str()));
	comp->Insert(SECURITY_ID, ref new String(secStr.c_str()));

	// Push new keys to the repository
	localSettings->Values->Insert(SETTINGS_ID, comp);
}


/// <summary>
/// Invoked when Navigation to a certain page fails
/// </summary>
/// <param name="sender">The Frame which failed navigation</param>
/// <param name="e">Details about the navigation failure</param>
void App::OnNavigationFailed(Platform::Object ^sender, NavigationFailedEventArgs ^e)
{
    throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}