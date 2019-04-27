//
// App.xaml.cpp
// Implementation of the App class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace FinalUWP;

using namespace Platform;
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

static String^ const SECURITY_FILE = "secPol.bin";
static String^ const APPLIST_FILE = "appStorage.bin";

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
void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e)
{
    auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

    // Do not repeat app initialization when the Window already has content,
    // just ensure that the window is active
    if (rootFrame == nullptr)
    {
        // Create a Frame to act as the navigation context and associate it with
        // a SuspensionManager key
        rootFrame = ref new Frame();

        rootFrame->NavigationFailed += ref new Windows::UI::Xaml::Navigation::NavigationFailedEventHandler(this, &App::OnNavigationFailed);

		StorageFolder^ local = ApplicationData::Current->LocalFolder;

		readData(local, SECURITY_FILE, [](std::vector<BYTE>* rt) {
			if (rt->size()) SecurityManager::deserialize(*rt);
		});

		readData(local, APPLIST_FILE, [](std::vector<BYTE> * rt) {
			if (rt->size()) AppIndex::deserialize(*rt);
		});

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
            // Ensure the current window is active
            Window::Current->Activate();
        }
    }
    else
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

	writeData(ApplicationData::Current->LocalFolder, APPLIST_FILE, AppIndex::serialize(), []() {
	});

	writeData(ApplicationData::Current->LocalFolder, SECURITY_FILE, SecurityManager::serialize(), []() {
	});
}

template <typename T>
void FinalUWP::App::writeData(StorageFolder^ parent, String^ name, std::vector<BYTE>& data, T&& lambda)
{
	// Create file (overwrite if existing)
	auto task = concurrency::create_task(parent->CreateFileAsync(name, CreationCollisionOption::ReplaceExisting));

	task.then([](StorageFile ^ file) -> concurrency::task<IRandomAccessStream^>
	{
		// Obtain handle to file's data stream
		return concurrency::create_task(file->OpenAsync(FileAccessMode::ReadWrite));
	}).then([data, &lambda](IRandomAccessStream^ stream)
	{
		DataWriter^ dw = ref new DataWriter(stream); // Create data writer object, attach to stream

		// Copy data from vector to write buffer
		for (auto iter = data.begin(); iter < data.end(); iter++) dw->WriteByte(*iter);

		// Commit stream buffer to disk
		concurrency::create_task(dw->StoreAsync()).then([dw, stream](UINT l) 
		{
			// Close the writer object and its associated stream
			dw->DetachStream();
			delete dw;
			delete stream;
		}).then([&lambda](concurrency::task<void> t) {
			lambda();
		});
	});
}

template <typename T>
void FinalUWP::App::readData(StorageFolder^ parent, String^ name, UINT start, UINT len, T&& lambda)
{
	// Obtain handle to existing file if there is one, create a new file if not
	concurrency::create_task(parent->CreateFileAsync(name, CreationCollisionOption::OpenIfExists))
	.then([](StorageFile ^ file) -> concurrency::task<IRandomAccessStream^>
	{
		// If the specified name is a directory, the operation fails
		if ((file->Attributes & FileAttributes::Directory) == FileAttributes::Directory) {
			SetLastError(ERROR_DIRECTORY_NOT_SUPPORTED);
			throw 1;
		}

		// Obtain handle to file's data stream
		return concurrency::create_task(file->OpenAsync(FileAccessMode::Read));
	}).then([start, len, &lambda](IRandomAccessStream^ stream)
	{
		// Ensure that the stream is at the correct offset before reading
		stream->Seek(start);

		// If the stream cannot be read, the operation fails
		if (!stream->CanRead) {
			SetLastError(ERROR_ACCESS_DENIED);
			throw -1;
		}

		DataReader^ dr = ref new DataReader(stream); // Create data writer object, attach to stream
		UINT size = len == 0 || len > stream->Size ? stream->Size : len; // Determine number of bytes to be read
		concurrency::create_task(dr->LoadAsync(size)).then([dr, stream, size, &lambda](UINT l)
		{
			std::vector<BYTE>* dest = new std::vector<BYTE>();
			// If the stream size is 0, the file is empty (perhaps because it has just been created by this routine),
			// return with the current buffer intact.
			if (l > 0)
			{
				// Copy data from stream buffer to destination vector
				dest->resize(size); // Preallocate vector capacity for faster copy
									 // Also guarantees that reallocation will not invalidate active pointers
				for (auto ptr = dest->begin(); ptr < dest->end(); ptr++) *ptr = dr->ReadByte();
			}

			// Close reader and associated stream
			dr->DetachStream();
			delete dr;
			delete stream;
			lambda(dest);
		});
	});


}

template <typename T>
void FinalUWP::App::readData(StorageFolder^ parent, String^ name, T&& lambda){
	readData(parent, name, 0, 0, lambda);
}

/// <summary>
/// Invoked when Navigation to a certain page fails
/// </summary>
/// <param name="sender">The Frame which failed navigation</param>
/// <param name="e">Details about the navigation failure</param>
void App::OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e)
{
    throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}