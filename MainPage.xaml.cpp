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
	InitializeComponent();

	appPickerDialog = ref new FileChooserDialog();
	appPickerDialog->Title = "Choose an Application:";
	appPickerDialog->PrimaryButtonText = "Add App";
	appPickerDialog->DefaultButton = ContentDialogButton::Secondary;

	PINEntryDialog = ref new PINPadDialog();
	PINEntryDialog->DefaultButton = ContentDialogButton::Primary;

	appSource = ref new Platform::Collections::Vector<String^>();
	metaSource = ref new Platform::Collections::Vector<String^>();

	AppList->ItemsSource = appSource;
	DetailList->ItemsSource = metaSource;

	BOOL* lch = &(this->hasLauncher);
	auto locateTask = concurrency::create_task(StorageFile::GetFileFromApplicationUriAsync(ref new Uri("ms-appx:///UWPProcessLauncher.exe")));
	locateTask.then([lch](StorageFile ^ exe) {
		if (exe) (*lch) = TRUE;
		else (*lch) = FALSE;
	});

	setAdminStateL(SecurityManager::checkState());
	setAdminStateD(100);
}

void FinalUWP::MainPage::setAdminStateL(BOOL state) {
	VisualStateManager::GoToState(this, state ? "Admin" : "NoAdmin", false);
}

void FinalUWP::MainPage::setAdminStateD(UINT delay)
{
	MainPage^ rpm = this;
	IBindableVector^* asv = &appSource;

	concurrency::task<void>([delay, rpm, asv]()
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));

			Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync
			(Windows::UI::Core::CoreDispatcherPriority::Low,
				ref new Windows::UI::Core::DispatchedHandler([rpm, asv]()
					{
						rpm->setAdminStateL(SecurityManager::checkState());

						for (AppRef r : AppIndex::getList())
							(*asv)->Append(ref new String(r.appName.c_str()));
					}));
		});
}

void FinalUWP::MainPage::notifyNoFile(UINT appIndex)
{
	AppRef ar = AppIndex::getList().at(appIndex);

	if (SecurityManager::checkState())
	{
		ContentDialog^ confirmDelete = ref new ContentDialog();
		confirmDelete->Title = "File not found";
		confirmDelete->PrimaryButtonText = "Yes";
		confirmDelete->CloseButtonText = "No";
		confirmDelete->Content = "The application executable referenced by the entry \"" + ref new String(ar.appName.c_str()) + "\" does not appear to exist. Would you like to remove the entry?";
		confirmDelete->DefaultButton = ContentDialogButton::Primary;

		auto task = concurrency::create_task(confirmDelete->ShowAsync());
		IBindableVector^* asv = &(this->appSource);

		task.then([ar, asv](ContentDialogResult res) mutable {
			if (res == ContentDialogResult::Primary) {
				AppIndex::remove(ar);

				unsigned int* idx = new unsigned int(0);
				if ((*asv)->IndexOf(ref new Platform::String(ar.appName.c_str()), idx))
					(*asv)->RemoveAt(*idx);

				ContentDialog^ confirmation = ref new ContentDialog();
				confirmation->Title = "Reference Deleted";
				confirmation->CloseButtonText = "OK";
				confirmation->Content = "Application reference deleted.";
				confirmation->DefaultButton = ContentDialogButton::Close;

				confirmation->ShowAsync();
			}
			});
	}
	else {
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
	if (AppList->SelectedIndex > 0 && AppList->SelectedIndex < AppList->Items->Size) {
		AppList->SelectedIndex -= 1;
	}
}


void FinalUWP::MainPage::MoveDown_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	if (AppList->SelectedIndex >= 0 && AppList->SelectedIndex < AppList->Items->Size - 1) {
		AppList->SelectedIndex += 1;
	}
}


void FinalUWP::MainPage::AddApp_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	appPickerDialog->Clear();
	auto res = concurrency::create_task(appPickerDialog->ShowAsync());
	FileChooserDialog^* fcp = &(this->appPickerDialog);
	IBindableVector^* asv = &(this->appSource);
	res.then([fcp, asv](ContentDialogResult r) {
		if (r == ContentDialogResult::Primary) {
			AppRef ar = AppRef(std::wstring((*fcp)->ChosenName->Data()), std::wstring((*fcp)->ChosenFile->Path->Data()));
			AppIndex::add(ar);
			(*asv)->Append(ref new String(ar.appName.c_str()));
		}
		});
}


void FinalUWP::MainPage::RemoveApp_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	if (AppList->SelectedIndex >= 0 && AppList->SelectedIndex < AppList->Items->Size)
	{
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
		res.then([selected, asv, idx](ContentDialogResult r)
			{
				if (r == ContentDialogResult::Primary) {
					for (AppRef ar : AppIndex::getList()) {
						if (std::wstring(selected->Data()) == ar.appName) {
							AppIndex::remove(ar);
							(*asv)->RemoveAt(idx);
							break;
						}
					}
				}
			});
	}
}


void FinalUWP::MainPage::Lock_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	if (SecurityManager::checkState())
	{
		if (!SecurityManager::hasSetPin())
		{
			ContentDialog^ noPIN = ref new ContentDialog();
			noPIN->Title = "No PIN Set";
			noPIN->Content = "You haven't set a PIN yet! Set one before locking the app down.";
			noPIN->CloseButtonText = "Cancel";
			noPIN->PrimaryButtonText = "Set PIN";
			noPIN->DefaultButton = ContentDialogButton::Primary;

			auto task = concurrency::create_task(noPIN->ShowAsync());

			MainPage^ rpm = this;
			task.then([&sender, e, rpm](ContentDialogResult r) {
				if (r == ContentDialogResult::Primary)
					rpm->PassCode_Click(sender, e);
				});
		}
		else {
			SecurityManager::lock();
			setAdminStateL(FALSE);
		}
	}
	else
	{
		if (SecurityManager::hasSetPin())
		{
			PINEntryDialog->PrimaryButtonText = "Confirm";
			PINEntryDialog->Clear();
			auto task = concurrency::create_task(PINEntryDialog->ShowAsync());

			MainPage^ rpm = this;
			PINPadDialog^* pdu = &(this->PINEntryDialog);
			task.then([rpm, pdu](ContentDialogResult r)
				{
					if (r == ContentDialogResult::Primary && !(*pdu)->Entered->IsEmpty()) {
						if (SecurityManager::unlock(std::wstring((*pdu)->Entered->Data()))) rpm->setAdminStateL(TRUE);
						else {
							ContentDialog^ incorrect = ref new ContentDialog();
							incorrect->Title = "Incorrect PIN";
							incorrect->Content = "You have entered an incorrect PIN. Please contact your administrator if you have forgotten your administrative PIN.";
							incorrect->CloseButtonText = "OK";
							incorrect->ShowAsync();
						}
					}
				});
		}
		else {
			SecurityManager::unlock(std::wstring());
			setAdminStateL(TRUE);
		}
	}
}


void FinalUWP::MainPage::PassCode_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
	PINEntryDialog->PrimaryButtonText = "Set";
	PINEntryDialog->Clear();
	auto task = concurrency::create_task(PINEntryDialog->ShowAsync());

	PINPadDialog^* pdu = &(this->PINEntryDialog);
	task.then([pdu](ContentDialogResult r) {
		if (r == ContentDialogResult::Primary && !(*pdu)->Entered->IsEmpty())
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
	ContentDialog^ confirmation = ref new ContentDialog();
	confirmation->Title = "Are you sure?";
	confirmation->Content = "This will wipe all administrator credentials, delete all app references, and restore settings to defaults. Are you sure you wish to continue?";
	confirmation->CloseButtonText = "No";
	confirmation->PrimaryButtonText = "Yes";
	confirmation->DefaultButton = ContentDialogButton::Close;
	auto task = concurrency::create_task(confirmation->ShowAsync());

	IBindableVector^* asv = &(this->appSource);
	IBindableVector^* amv = &(this->metaSource);

	task.then([asv, amv](ContentDialogResult r)
		{
			if (r == ContentDialogResult::Primary) {
				(*asv)->Clear();
				(*amv)->Clear();
				SecurityManager::setPIN(std::wstring());
				AppIndex::getList().clear();

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
	if (AppList->SelectedIndex >= 0 && AppList->SelectedIndex < AppList->Items->Size)
		this->Frame->Navigate(TypeName(FinalUWP::MetaEdit::typeid), AppList->SelectedIndex);
}


void FinalUWP::MainPage::AppList_KeyUp(Platform::Object^ sender, KeyRoutedEventArgs^ e)
{
	if (e->Key == Windows::System::VirtualKey::Enter || e->Key == Windows::System::VirtualKey::Space) 
		Launch_Click(sender, e);
}

void FinalUWP::MainPage::AppList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (AppList->SelectedIndex >= 0 && AppList->SelectedIndex < AppList->Items->Size)
	{
		String^ selected = (String^)AppList->SelectedItem;
		for (AppRef ar : AppIndex::getList())
		{
			if (std::wstring(selected->Data()) == ar.appName)
			{
				size_t* metaSize = new size_t(0);
				std::wstring* meta = ar.metaToString(metaSize);
				metaSource->Clear();
				for (size_t i = 0; i < *metaSize; i++)
					metaSource->Append(ref new String(meta[i].c_str()));

				return;
			}
		}
	}

	metaSource->Clear();
}


void FinalUWP::MainPage::Launch_Click(Platform::Object ^ sender, RoutedEventArgs ^ e)
{
	if (AppList->SelectedIndex < 0 || AppList->SelectedIndex >= AppList->Items->Size) return;

	BOOL canLaunch = TRUE;
	String^ errorText = "";

	if (!Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent("Windows.ApplicationModel.FullTrustAppContract", 1, 0)) {
		canLaunch = FALSE;
		errorText = "It appears that you are running this application on a system which does not support the Windows 10 Full-Trust Process API, such as Windows 10 S. Apps will not launch until this is resolved.";
	}

	String^ selected = (String^)AppList->SelectedItem;
	AppRef ar;
	for (AppRef a : AppIndex::getList()) {
		if (std::wstring(selected->Data()) == a.appName)
			ar = a;
	}

	if (!&ar) return;
	else if (!hasLauncher) {
		canLaunch = FALSE;
		errorText = "The master application launcher executable (UWPProcessLauncher.exe) appears to be missing from the application package. Please perform a clean re-install of the AppX package.";
	}

	if (!canLaunch) {
		ContentDialog^ unavail = ref new ContentDialog();
		unavail->Title = "Unable to Launch App";
		unavail->CloseButtonText = "OK";
		unavail->Content = SecurityManager::checkState() ? errorText : "An error has occurred. Please contact your administrator for details. We apologize for the inconvenience.";
		unavail->DefaultButton = ContentDialogButton::Close;
		unavail->ShowAsync();
		return;
	}

	std::wstring absPath = ar.absolutePath;

	auto folderTask = concurrency::create_task(KnownFolders::DocumentsLibrary->CreateFolderAsync("ARK Software", CreationCollisionOption::OpenIfExists));
	folderTask.then([absPath](StorageFolder ^ result) 
	{
		if (result) 
		{
			auto createTask = concurrency::create_task(result->CreateFileAsync("processName.tpc", CreationCollisionOption::ReplaceExisting));
			createTask.then([absPath](StorageFile ^ result) 
			{
				if (result) 
				{
					auto writeTask = concurrency::create_task(FileIO::WriteTextAsync(result, ref new String(absPath.c_str())));
					writeTask.then([]() 
					{
						Windows::ApplicationModel::FullTrustProcessLauncher::LaunchFullTrustProcessForCurrentAppAsync("FileAccess");
					});
				}
			});
		}
	});
}
