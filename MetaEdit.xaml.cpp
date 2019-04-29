//
// MetaEdit.xaml.cpp
// Implementation of the BlankPage class
//

#include "pch.h"
#include "MetaEdit.xaml.h"

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
using namespace Windows::UI::Xaml::Interop;

MetaEdit::MetaEdit()
{
	// Initialize source binding for meta element list before component initialization
	metaSource = ref new Platform::Collections::Vector<MetaPair^>();
	InitializeComponent();

	// Initialize path editor dialog
	pathEditor = ref new FileChooserDialog();
	pathEditor->Title = "Edit Application Path...";
	pathEditor->PrimaryButtonText = "Commit Changes";
	pathEditor->DefaultButton = ContentDialogButton::Primary;

	editingIndex = -1;
	modifiedPath = std::wstring();
}

void FinalUWP::MetaEdit::OnNavigatedTo(NavigationEventArgs^ e)
{
	// If no parameter has been provided, clear state and navigate back to main page.
	if (!e->Parameter) {
		editingIndex = -1;
		this->Frame->Navigate(TypeName(FinalUWP::MainPage::typeid));
		return;
	}

	// Derive parameter from navigation arguments, clear previous state if there is one
	editingIndex = (int)(e->Parameter);
	metaSource->Clear();

	// Check bounds of provided index, grab it if it is valid
	AppRef ar = AppIndex::getAt(editingIndex);
	if(ar.absolutePath == std::wstring()) Cancel_Click(nullptr, ref new RoutedEventArgs());
	
	// Display metadata if there is any
	if (ar.metadata.size() != 0) {
		for (std::pair<std::wstring, std::wstring> p : ar.metadata)
			metaSource->Append(ref new MetaPair(ref new String(p.first.c_str()), ref new String(p.second.c_str())));
	}else {
		// If there is no existing metadata, prompt for autofill
		ContentDialog^ autoPopulate = ref new ContentDialog();
		autoPopulate->Title = "Auto-populate fields?";
		autoPopulate->Content = "It appears that the reference you are editing has no metadata. Would you like the editor to generate some commonly-used meta-tags for you?";
		autoPopulate->CloseButtonText = "No";
		autoPopulate->PrimaryButtonText = "Yes";
		autoPopulate->DefaultButton = ContentDialogButton::Primary;

		IBindableVector^* ms = &(this->metaSource);
		auto task = concurrency::create_task(autoPopulate->ShowAsync());
		task.then([ms, ar](ContentDialogResult res) 
		{
			// If the user chose to autofill metadata, add pre-filled metadata entries
			if (res == ContentDialogResult::Primary) {
				(*ms)->Append(ref new MetaPair("Full Name", ref new String(ar.appName.c_str())));
				(*ms)->Append(ref new MetaPair("Developer", ""));
				(*ms)->Append(ref new MetaPair("Development Year", ""));
				(*ms)->Append(ref new MetaPair("Language(s)", ""));
				(*ms)->Append(ref new MetaPair("Developed With", ""));
				(*ms)->Append(ref new MetaPair("Description", ""));
			}
		});
	}

	// Fill editing name field from reference
	EditName->Text = ref new String(ar.appName.c_str());
}


void FinalUWP::MetaEdit::Cancel_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// Clear state before exiting
	editingIndex = -1;
	this->Frame->Navigate(TypeName(FinalUWP::MainPage::typeid));
	return;
}


void FinalUWP::MetaEdit::Commit_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// Initialize destination map
	std::map<std::wstring, std::wstring> dest = std::map<std::wstring, std::wstring>();

	// Copy meta-elements from bound vector to map
	for (UINT i = 0; i < metaSource->Size; i++) 
	{
		MetaPair^ elem = (MetaPair^)metaSource->GetAt(i);
		std::wstring key = std::wstring(elem->Name->Data());
		std::wstring val = std::wstring(elem->Value->Data());
		if(key != L"") dest.insert_or_assign(key, val); // Only copy named pairs
	}

	// Set values on target reference
	std::vector<AppRef>::iterator iter = AppIndex::getList().begin();
	iter += editingIndex;
	(*iter).metadata = dest;

	// Only update reference name if the name field has any content
	if (EditName->Text->Length() > 0) (*iter).appName = std::wstring(EditName->Text->Data());

	// Update reference path if it was changed
	if (modifiedPath != std::wstring()) (*iter).absolutePath = modifiedPath;

	// Clear edit index and navigate back to the main page
	editingIndex = -1;
	modifiedPath = std::wstring();
	this->Frame->Navigate(TypeName(FinalUWP::MainPage::typeid));
}


void FinalUWP::MetaEdit::AddEntry_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	metaSource->Append(ref new MetaPair("Name", "Value"));
}


void FinalUWP::MetaEdit::DeleteEntry_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	if (MetaList->SelectedIndex >= 0 && MetaList->SelectedIndex < metaSource->Size) // Check bounds before removing
		metaSource->RemoveAt(MetaList->SelectedIndex);
}


void FinalUWP::MetaEdit::PathEdit_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// Obtain target reference
	AppRef ar = AppIndex::getAt(editingIndex);

	// Autofill path field with current reference's filename, if reference has no valid
	// filename, autofill with set name instead
	if(ar.absolutePath.find('\\') != std::wstring::npos) 
		pathEditor->FileNameDisplay = ref new String(ar.absolutePath.substr(ar.absolutePath.find_last_of(L'\\') + 1, ar.absolutePath.length()).c_str());

	// Autofill name field
	pathEditor->ChosenName = EditName->Text;

	// Obtain pointers to class fields for use in lambda
	TextBox^* tbr = &(this->EditName);
	FileChooserDialog^* fch = &(this->pathEditor);
	std::wstring* mod = &(this->modifiedPath);
	concurrency::create_task(pathEditor->ShowAsync()).then([mod, tbr, fch](ContentDialogResult res) 
	{
		if (res == ContentDialogResult::Primary) 
		{
			// Name length checks have already been done in the dialog itself,
			// so we only need to assign values back
			(*tbr)->Text = (*fch)->ChosenName;

			// Chosen file is not preset when dialog is shown, so we need to check it before assigning any
			// potential changes
			if ((*fch)->ChosenFile) (*mod) = std::wstring((*fch)->ChosenFile->Path->Data());
		}
	});
}
