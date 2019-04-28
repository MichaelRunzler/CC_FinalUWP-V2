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
	metaSource = ref new Platform::Collections::Vector<MetaPair^>();
	InitializeComponent();

	editingIndex = -1;
}

void FinalUWP::MetaEdit::OnNavigatedTo(NavigationEventArgs^ e)
{
	if (!e->Parameter) {
		editingIndex = -1;
		this->Frame->Navigate(TypeName(FinalUWP::MainPage::typeid));
		return;
	}

	editingIndex = (int)(e->Parameter);
	metaSource->Clear();
	std::vector<AppRef>::iterator iter = AppIndex::getList().begin();
	if (iter + editingIndex > AppIndex::getList().end() || iter + editingIndex < AppIndex::getList().begin()) Cancel_Click(NULL, ref new RoutedEventArgs());
	iter += editingIndex;
	
	if ((*iter).metadata.size() != 0) {
		for (std::pair<std::wstring, std::wstring> p : (*iter).metadata)
			metaSource->Append(ref new MetaPair(ref new String(p.first.c_str()), ref new String(p.second.c_str())));
	}else 
	{
		ContentDialog^ autoPopulate = ref new ContentDialog();
		autoPopulate->Title = "Auto-populate fields?";
		autoPopulate->Content = "It appears that the reference you are editing has no metadata. Would you like the editor to generate some commonly-used meta-tags for you?";
		autoPopulate->CloseButtonText = "No";
		autoPopulate->PrimaryButtonText = "Yes";
		autoPopulate->DefaultButton = ContentDialogButton::Primary;

		IBindableVector^* ms = &(this->metaSource);
		auto task = concurrency::create_task(autoPopulate->ShowAsync());
		task.then([ms](ContentDialogResult res) 
		{
			if (res == ContentDialogResult::Primary) {
				(*ms)->Append(ref new MetaPair("Full Name", ""));
				(*ms)->Append(ref new MetaPair("Developer", ""));
				(*ms)->Append(ref new MetaPair("Class Year", ""));
				(*ms)->Append(ref new MetaPair("Language(s)", ""));
				(*ms)->Append(ref new MetaPair("Developed With", ""));
				(*ms)->Append(ref new MetaPair("Description", ""));
			}
		});
	}

	EditState->Text = "Editing: " + ref new String((*iter).appName.c_str());
}


void FinalUWP::MetaEdit::Cancel_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	editingIndex = -1;
	this->Frame->Navigate(TypeName(FinalUWP::MainPage::typeid));
	return;
}


void FinalUWP::MetaEdit::Commit_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	std::map<std::wstring, std::wstring> dest = std::map<std::wstring, std::wstring>();

	for (UINT i = 0; i < metaSource->Size; i++) 
	{
		MetaPair^ elem = (MetaPair^)metaSource->GetAt(i);
		std::wstring key = std::wstring(elem->Name->Data());
		std::wstring val = std::wstring(elem->Value->Data());
		if(key != L"") dest.insert_or_assign(key, val);
	}

	std::vector<AppRef>::iterator iter = AppIndex::getList().begin();
	iter += editingIndex;
	(*iter).metadata = dest;

	editingIndex = -1;
	this->Frame->Navigate(TypeName(FinalUWP::MainPage::typeid));
}


void FinalUWP::MetaEdit::AddEntry_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	metaSource->Append(ref new MetaPair("Name", "Value"));
}


void FinalUWP::MetaEdit::DeleteEntry_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	if (MetaList->SelectedIndex >= 0 && MetaList->SelectedIndex < metaSource->Size)
		metaSource->RemoveAt(MetaList->SelectedIndex);
}
