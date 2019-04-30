//
// FileChooserDialog.xaml.cpp
// Implementation of the FileChooserDialog class
//

#include "pch.h"
#include "FileChooserDialog.xaml.h"

using namespace FinalUWP;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;


FinalUWP::FileChooserDialog::FileChooserDialog()
{
	InitializeComponent();
}


void FinalUWP::FileChooserDialog::ContentDialog_PrimaryButtonClick(ContentDialog^ sender, ContentDialogButtonClickEventArgs^ args)
{
	// Abort close and show error if one or both fields are empty

	if(&chosen == NULL || FilePath->Text->IsEmpty()){
		args->Cancel = TRUE;
		ErrorDisplay->Text = "Please select a file.";
		ErrorDisplay->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}else if(AppName->Text->IsEmpty()){
		args->Cancel = TRUE;
		ErrorDisplay->Text = "Please set a name.";
		ErrorDisplay->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
}

void FinalUWP::FileChooserDialog::ContentDialog_SecondaryButtonClick(ContentDialog^ sender, ContentDialogButtonClickEventArgs^ args)
{
	// Clear state before exiting
	this->Clear();
}


void FinalUWP::FileChooserDialog::ChangePath_Click(Object^ sender, RoutedEventArgs^ e)
{
	// Load and display the file picker
	auto picker = ref new FileOpenPicker();
	picker->ViewMode = PickerViewMode::List;
	picker->SuggestedStartLocation = PickerLocationId::ComputerFolder;
	picker->FileTypeFilter->Append(".jar");
	picker->FileTypeFilter->Append(".exe");
	picker->FileTypeFilter->Append(".jnlp");

	// Get pointers to class fields so that they may be passed as copy-mode lambda captures
	StorageFile^* rcf = &chosen;
	TextBox^* rcb = &FilePath;
	TextBox^* rcn = &AppName;

	auto task = concurrency::create_task(picker->PickSingleFileAsync());
	task.then([rcf, rcb, rcn](StorageFile^ file){
		// Only set values if the user did not click Cancel
		if (file) {
			*rcf = file;
			(*rcb)->Text = file->Name;
			(*rcn)->Text = file->DisplayName;
		}
	});
}


/// <summary>
/// Clears all fields and stored data in this dialog,
/// resetting it as if it were reinitialized.
/// Implementation of Clearable::Clear()
/// </summary>
void FinalUWP::FileChooserDialog::Clear()
{
	ErrorDisplay->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	FilePath->Text = "";
	AppName->Text = "";
	// Must be cleared-by-pointer due to how ref classes work
	StorageFile^* tmp = &(this->chosen);
	(*tmp) = nullptr;
}
