//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace FinalUWP;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;


MainPage::MainPage()
{
	// Initialize bound vectors
	appSource = ref new Platform::Collections::Vector<String^>();
	metaSource = ref new Platform::Collections::Vector<String^>();

	InitializeComponent();
	
	// Initialize persistent dialogs
	appPickerDialog = ref new FileChooserDialog();
	appPickerDialog->Title = "Choose an Application:";
	appPickerDialog->PrimaryButtonText = "Add App";
	appPickerDialog->DefaultButton = ContentDialogButton::Secondary;

	PINEntryDialog = ref new PINPadDialog();
	PINEntryDialog->DefaultButton = ContentDialogButton::Primary;

	// Check if Win32 launcher subroutine EXE is present in the app package.
	bool* lch = &(this->hasLauncher);
	auto locateTask = concurrency::create_task(StorageFile::GetFileFromApplicationUriAsync(ref new Uri("ms-appx:///UWPProcessLauncher.exe")));
	locateTask.then([lch](StorageFile ^ exe) {
		if (exe) (*lch) = TRUE;
		else (*lch) = FALSE;
	});

	// Add initial batch of reference entries from the AppIndex, if there are any
	for (AppRef r : AppIndex::getList())
		appSource->Append(ref new String(r.appName.c_str()));

	// Set the initial lock state from the SecurityManager
	setAdminStateL(SecurityManager::checkState());
}

/// <summary>
/// Sets whether or not the admin controls on the main page are enabled.
/// Does not change the SecurityManager's state.
/// </summary>
/// <param name="state">TRUE enables admin controls, FALSE disables them.</param>
void FinalUWP::MainPage::setAdminStateL(bool state) {
	VisualStateManager::GoToState(this, state ? "Admin" : "NoAdmin", false);
}

/// <summary>
/// Notifies the user that the file referenced by the specified AppRef is not available.
/// If Admin controls are enabled, offers to remove the reference.
/// </summary>
/// <param name="appIndex">The index of the reference whose file is missing.</param>
void FinalUWP::MainPage::notifyNoFile(UINT appIndex)
{
	// Get target reference
	AppRef ar = AppIndex::getAt(appIndex);

	if (SecurityManager::checkState())
	{
		// If admin controls are enabled, offer to remove the reference automatically.
		ContentDialog^ confirmDelete = ref new ContentDialog();
		confirmDelete->Title = "File not found";
		confirmDelete->PrimaryButtonText = "Yes";
		confirmDelete->CloseButtonText = "No";
		confirmDelete->Content = "The application executable referenced by the entry \"" + ref new String(ar.appName.c_str()) + "\" does not appear to exist. Would you like to remove the entry?";
		confirmDelete->DefaultButton = ContentDialogButton::Primary;

		IBindableVector^* asv = &(this->appSource);
		UINT* idx = new UINT(appIndex);
		auto task = concurrency::create_task(confirmDelete->ShowAsync());
		task.then([ar, asv, idx](ContentDialogResult res) 
		{
			if (res == ContentDialogResult::Primary) 
			{
				// If the user has chosen to remove the reference, remove it from the front-facing list and
				// the backend index and notify the user of successful removal
				AppIndex::remove((*idx));
				(*asv)->RemoveAt(*idx);

				ContentDialog^ confirmation = ref new ContentDialog();
				confirmation->Title = "Reference Deleted";
				confirmation->CloseButtonText = "OK";
				confirmation->Content = "Application reference deleted.";
				confirmation->DefaultButton = ContentDialogButton::Close;

				confirmation->ShowAsync();
			}
		});
	}else {
		// If admin controls are disabled, notify the user that an admin will have to resolve the issue
		ContentDialog^ notFound = ref new ContentDialog();
		notFound->Title = "File not found";
		notFound->CloseButtonText = "OK";
		notFound->Content = "The application file for \"" + ref new String(ar.appName.c_str()) + "\" could not be found. Please contact your administrator for assistance.";
		notFound->DefaultButton = ContentDialogButton::Close;

		notFound->ShowAsync();
	}
}


void FinalUWP::MainPage::MoveUp_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	// Move the selection cursor up by one
	if (AppList->SelectedIndex > 0 && AppList->SelectedIndex < AppList->Items->Size) 
		AppList->SelectedIndex -= 1;
}


void FinalUWP::MainPage::MoveDown_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	// Move the selection cursor down by one
	if (AppList->SelectedIndex >= 0 && AppList->SelectedIndex < AppList->Items->Size - 1) 
		AppList->SelectedIndex += 1;
}


void FinalUWP::MainPage::AddApp_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	// Clear the current state of the persistent dialog if there is one
	appPickerDialog->Clear();

	FileChooserDialog^* fcp = &(this->appPickerDialog);
	IBindableVector^* asv = &(this->appSource);

	auto res = concurrency::create_task(appPickerDialog->ShowAsync());
	res.then([fcp, asv](ContentDialogResult r) 
	{
		// If the user did not cancel the dialog, use the result to construct a new AppRef and add it to the index
		// and the forward-facing list
		if (r == ContentDialogResult::Primary) {
			AppIndex::add(std::wstring((*fcp)->ChosenName->Data()), std::wstring((*fcp)->ChosenFile->Path->Data()));
			(*asv)->Append((*fcp)->ChosenName);
		}
	});
}


void FinalUWP::MainPage::RemoveApp_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	// Check bounds of selection first
	if (AppList->SelectedIndex < 0 || AppList->SelectedIndex >= AppList->Items->Size)
		return;

	// Confirm that the user wishes to delete the entry
	ContentDialog^ confirmDelete = ref new ContentDialog();
	confirmDelete->Title = "Confirm Removal";
	confirmDelete->Content = "Are you sure you wish to remove this application reference? No files will be deleted on disk.";
	confirmDelete->PrimaryButtonText = "Confirm";
	confirmDelete->CloseButtonText = "Cancel";
	confirmDelete->DefaultButton = ContentDialogButton::Close;

	String^ selected = (String^)AppList->SelectedItem;
	UINT idx = AppList->SelectedIndex;
	IBindableVector^* asv = &(this->appSource);

	auto res = concurrency::create_task(confirmDelete->ShowAsync());
	res.then([selected, asv, idx](ContentDialogResult r){
		// If the user has chosen to delete the entry, remove it from the index and list
		if (r == ContentDialogResult::Primary) {
			AppIndex::remove(idx);
			(*asv)->RemoveAt(idx);
		}
	});
}


void FinalUWP::MainPage::Lock_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	if (SecurityManager::checkState())
	{
		if (!SecurityManager::hasSetPin())
		{
			// If no PIN has been set, ask that the user set one before locking the app
			ContentDialog^ noPIN = ref new ContentDialog();
			noPIN->Title = "No PIN Set";
			noPIN->Content = "You haven't set a PIN yet! Set one before locking the app down.";
			noPIN->CloseButtonText = "Cancel";
			noPIN->PrimaryButtonText = "Set PIN";
			noPIN->DefaultButton = ContentDialogButton::Primary;

			auto task = concurrency::create_task(noPIN->ShowAsync());

			MainPage^ rpm = this;
			task.then([&sender, e, rpm](ContentDialogResult r) {
				// Call the passcode set method by reference
				if (r == ContentDialogResult::Primary)
					rpm->PassCode_Click(sender, e);
			});
		}else {
			// Lock the app if a PIN is set
			SecurityManager::lock();
			setAdminStateL(FALSE);
		}
	}else{
		if (SecurityManager::hasSetPin())
		{
			// Request the PIN from the user
			PINEntryDialog->PrimaryButtonText = "Confirm";
			PINEntryDialog->Clear();
			auto task = concurrency::create_task(PINEntryDialog->ShowAsync());

			MainPage^ rpm = this;
			PINPadDialog^* pdu = &(this->PINEntryDialog);
			task.then([rpm, pdu](ContentDialogResult r)
			{
				// If the user entered a PIN, try to unlock the SecurityManager
				if (r == ContentDialogResult::Primary && !(*pdu)->Entered->IsEmpty()) 
				{
					if (SecurityManager::unlock(std::wstring((*pdu)->Entered->Data()))) rpm->setAdminStateL(TRUE);
					else 
					{
						// If the Manager fails to unlock, notify the user
						ContentDialog^ incorrect = ref new ContentDialog();
						incorrect->Title = "Incorrect PIN";
						incorrect->Content = "You have entered an incorrect PIN. Please contact your administrator if you have forgotten your administrative PIN.";
						incorrect->CloseButtonText = "OK";
						incorrect->ShowAsync();
					}
				}
			});
		}else {
			// If no PIN is set, just unlock without prompting for a PIN
			SecurityManager::unlock(std::wstring());
			setAdminStateL(TRUE);
		}
	}
}


void FinalUWP::MainPage::PassCode_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	// This button should only be accessible in Admin mode, but check anyway
	if (!SecurityManager::checkState()) 
	{
		// Notify the user that they do not have permission to execute this action
		ContentDialog^ noPerm = ref new ContentDialog();
		noPerm->Title = "Permission Denied";
		noPerm->Content = "You do not have permission to perform this action.";
		noPerm->CloseButtonText = "OK";
		noPerm->DefaultButton = ContentDialogButton::Close;
		noPerm->ShowAsync();
		return;
	}

	// Set up a few properties on the entry dialog and clear it
	PINEntryDialog->PrimaryButtonText = "Set";
	PINEntryDialog->Clear();
	PINPadDialog^* pdu = &(this->PINEntryDialog);

	auto task = concurrency::create_task(PINEntryDialog->ShowAsync());
	task.then([pdu](ContentDialogResult r) 
	{
		// If the user entered a PIN, set it and notify the user
		if (r == ContentDialogResult::Primary)
		{
			SecurityManager::setPIN(std::wstring((*pdu)->Entered->Data()));

			ContentDialog^ confirmation = ref new ContentDialog();
			confirmation->Title = "PIN Set!";
			confirmation->Content = "You have successfully set your administrative PIN. Be careful - if lost, your PIN cannot be recovered without resetting all stored app settings.";
			confirmation->CloseButtonText = "OK";
			confirmation->ShowAsync();
		}
	});
}


void FinalUWP::MainPage::Reset_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	// Confirm that the user wishes to wipe the settings store
	ContentDialog^ confirmation = ref new ContentDialog();
	confirmation->Title = "Are you sure?";
	confirmation->Content = "This will wipe all administrator credentials, delete all app references, and restore settings to defaults. Are you sure you wish to continue?";
	confirmation->CloseButtonText = "No";
	confirmation->PrimaryButtonText = "Yes";
	confirmation->DefaultButton = ContentDialogButton::Close;

	IBindableVector^* asv = &(this->appSource);
	IBindableVector^* amv = &(this->metaSource);

	auto task = concurrency::create_task(confirmation->ShowAsync());
	task.then([asv, amv](ContentDialogResult r)
	{
		// If the user confirmed:
		if (r == ContentDialogResult::Primary) 
		{
			// Wipe the front-facing metadata and AppRef lists
			(*asv)->Clear();
			(*amv)->Clear();

			// Clear the PIN in the SecurityManager
			SecurityManager::setPIN(std::wstring());

			// Clear the AppIndex
			AppIndex::removeAll();

			// Notify the user of successful deletion
			ContentDialog^ deleted = ref new ContentDialog();
			deleted->Title = "Reset Complete";
			deleted->Content = "All settings have been reset to their defaults. You may wish to restart the application to ensure a complete reset.";
			deleted->CloseButtonText = "OK";
			deleted->ShowAsync();
		}
	});
}


void FinalUWP::MainPage::EditMetadata_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	// Check bounds on the selection in the app list, then navigate the metadata editor with the selected
	// index as a passed argument
	if (AppList->SelectedIndex >= 0 && AppList->SelectedIndex < AppList->Items->Size)
		this->Frame->Navigate(TypeName(FinalUWP::MetaEdit::typeid), AppList->SelectedIndex);
}


void FinalUWP::MainPage::AppList_KeyUp(Platform::Object^ sender, KeyRoutedEventArgs^ e)
{
	// If the user pressed Space or Enter on an AppRef in the list, launch it as if they clicked Launch
	if (e->Key == Windows::System::VirtualKey::Enter || e->Key == Windows::System::VirtualKey::Space) 
		Launch_Click(sender, e);
}


void FinalUWP::MainPage::AppList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	// Check bounds. If the check fails, clear the metadata display and return.
	if (AppList->SelectedIndex < 0 && AppList->SelectedIndex >= AppList->Items->Size) {
		metaSource->Clear();
		return;
	}

	// Update the metadata list with the current data for the selected index
	size_t* metaSize = new size_t(0);
	std::wstring* meta = AppIndex::getAt(AppList->SelectedIndex).metaToString(metaSize);
	metaSource->Clear();
	for (size_t i = 0; i < *metaSize; i++)
		metaSource->Append(ref new String(meta[i].c_str()));
}


void FinalUWP::MainPage::Launch_Click(Platform::Object ^ sender, RoutedEventArgs ^ e)
{
	// Ensure that selection is within bounds
	if (AppList->SelectedIndex < 0 || AppList->SelectedIndex >= AppList->Items->Size) return;

	// Retrieve reference to selected AppRef. If the path is empty, it's likely that the selection
	// was out-of-bounds for the index, return without error.
	AppRef ar = AppIndex::getAt(AppList->SelectedIndex);
	if (ar.absolutePath == std::wstring()) return;

	bool canLaunch = TRUE;
	String^ errorText = "";

	// If the FullTrustAppLauncher is disabled or the Win32 launcher subroutine EXE is missing, set the error flags
	if (!Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent("Windows.ApplicationModel.FullTrustAppContract", 1, 0)) {
		canLaunch = FALSE;
		errorText = "It appears that you are running this application on a system which does not support the Windows 10 Full-Trust Process API, such as Windows 10 S. Apps will not launch until this is resolved.";
	}else if (!hasLauncher) {
		canLaunch = FALSE;
		errorText = "The master application launcher executable (UWPProcessLauncher.exe) appears to be missing from the application package. Please perform a clean re-install of the AppX package.";
	}

	// If one or more critical requirements are not met, notify the user (dependent on state) and return
	if (!canLaunch) 
	{
		ContentDialog^ unavail = ref new ContentDialog();
		unavail->Title = "Unable to Launch App";
		unavail->CloseButtonText = "OK";
		unavail->Content = SecurityManager::checkState() ? errorText : "An error has occurred. Please contact your administrator for details. We apologize for the inconvenience.";
		unavail->DefaultButton = ContentDialogButton::Close;
		unavail->ShowAsync();
		return;
	}

	// Collect information to be passed to lambda task chain
	std::wstring absPath = ar.absolutePath;
	UINT* sel = new UINT(AppList->SelectedIndex);
	MainPage^ rpm = this;

	// Initialize task chain.
	/// STEEL YOURSELVES, MY BROTHERS
	// Create or open the cross-thread communications root folder in the user's Documents library
	auto folderTask = concurrency::create_task(KnownFolders::DocumentsLibrary->CreateFolderAsync("ARK Software", CreationCollisionOption::OpenIfExists));
	folderTask.then([absPath, rpm, sel](StorageFolder ^ result) 
	{
		// Only proceed if the folder was created/opened successfully
		if (result) 
		{
			// Create/overwrite the process-name storage file
			auto createTask = concurrency::create_task(result->CreateFileAsync("processName.tpc", CreationCollisionOption::ReplaceExisting));
			createTask.then([absPath, result, rpm, sel](StorageFile ^ resultF) 
			{
				// Only proceed if the file was created successfully
				if (resultF) 
				{
					// Write the process name data to the file
					auto writeTask = concurrency::create_task(FileIO::WriteTextAsync(resultF, ref new String(absPath.c_str())));
					writeTask.then([]() -> concurrency::task<void> {
						// Activate the Win32 subroutine process via the FullTrustProcessLauncher component
						return concurrency::create_task(Windows::ApplicationModel::FullTrustProcessLauncher::LaunchFullTrustProcessForCurrentAppAsync("FileAccess"));
					}).then([result]() -> concurrency::task<StorageFile^> {
						// Create the target file for the subroutine to write its exit code to
						return concurrency::create_task(result->CreateFileAsync("exitCode.tpc", CreationCollisionOption::OpenIfExists));
					}).then([rpm, sel, resultF](StorageFile^ resCode) 
					{
						// Sleep for 250ms to allow the subroutine time to finish its work and write its exit code
						std::this_thread::sleep_for(std::chrono::milliseconds(250));
						// Read the exit code from the target file
						concurrency::create_task(FileIO::ReadTextAsync(resCode)).then([rpm, sel, resCode, resultF](String ^ code){
							// If the launcher wrote a code, check it
							if (code)
								// If the code is 0x02 (FILE_NOT_FOUND), the reference is probably not valid anymore, give the user
								// the option to remove it
								if (code->Length() > 0 && code == "2")
									rpm->notifyNoFile(*sel);

							// Remove the result code and process name files since they are no longer needed
							resCode->DeleteAsync();
							resultF->DeleteAsync();
							/// Hi, Joe. Enjoying the asyncness? ;)
						});
					});
				}
			});
		}
	});
}


void FinalUWP::MainPage::ExportSettings_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// TODO Set up flags for backup, run Launch_ONClicked, modify to write source/target files to TPC, launch FTPL with Backup arg set,
	// modify UWPProcessLauncher to read source/target and copy with delay after main app exits
}


void FinalUWP::MainPage::ImportSettings_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//TODO Same as above but in reverse with Restore arg set
}
