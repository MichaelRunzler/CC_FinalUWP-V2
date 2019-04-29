//
// FileChooserDialog.xaml.h
// Declaration of the FileChooserDialog class
//

#pragma once

#include "FileChooserDialog.g.h"
#include "Library/Clearable.h"

namespace FinalUWP
{
	/// <summary>
	/// Subclass of ContentDialog that presents the ability to
	/// choose a file from the filesystem and assign a custom name to
	/// the resulting reference.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class FileChooserDialog sealed : Clearable
	{
	public:
		FileChooserDialog();

		/// <summary>
		/// The file that the user has chosen from the filesystem.
		/// Will evaluate to NULL if the user clicked "Cancel" or
		/// the dialog has not been displayed yet.
		/// Read-only access.
		/// </summary>
		property Windows::Storage::StorageFile^ ChosenFile
		{
			Windows::Storage::StorageFile^ get() { return chosen; }
		}

		/// <summary>
		/// Provides write-only access to the text shown in the 'chosen file'
		/// text box. Does not alter the value of ChosenFile, nor does the
		/// provided string evaluate to a file in any way. This is provided
		/// as a means to 'autofill' the value contained in the field,
		/// for example when the dialog is being used to edit an existing
		/// association instead of creating a new one.
		/// </summary>
		property Platform::String^ FileNameDisplay
		{
			void set(Platform::String^ name) { FilePath->Text = name; }
		}

		/// <summary>
		/// The name that the user has chosen to associate with the selected file.
		/// May also be the chosen file's name and extension, if the user selected
		/// a file and did not change the value of the name field.
		/// Will evaluate to "" if the user clicked "Cancel" or the dialog
		/// has not been displayed yet.
		/// Read/write access.
		/// </summary>
		property Platform::String^ ChosenName
		{
			Platform::String^ get() { return AppName->Text; }
			void set(Platform::String^ name) { 
				AppName->Text = name; 
			}
		}

		// Inherited via Clearable
		virtual void Clear();

	private:
		// Storage fields for accessible properties
		Windows::Storage::StorageFile^ chosen;

		void ContentDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void ContentDialog_SecondaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void ChangePath_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ChangePath_Tapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
	};
}
