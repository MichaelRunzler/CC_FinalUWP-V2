//
// FileChooserDialog.xaml.h
// Declaration of the FileChooserDialog class
//

#pragma once

#include "FileChooserDialog.g.h"
#include "Library/Clearable.h"

namespace FinalUWP
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class FileChooserDialog sealed : Clearable
	{
	public:
		FileChooserDialog();

		property Windows::Storage::StorageFile^ ChosenFile
		{
			Windows::Storage::StorageFile^ get() { return chosen; }
		}

		property Platform::String^ FileNameDisplay
		{
			void set(Platform::String^ name) { FilePath->Text = name; }
		}

		property Platform::String^ ChosenName
		{
			Platform::String^ get() { return setName; }
			void set(Platform::String^ name) { AppName->Text = name; }
		}

		// Inherited via Clearable
		virtual void Clear();

	private:
		Windows::Storage::StorageFile^ chosen;
		Platform::String^ setName;

		void ContentDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void ContentDialog_SecondaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void ChangePath_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ChangePath_Tapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
	};
}
