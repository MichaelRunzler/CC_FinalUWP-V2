//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "MetaEdit.xaml.h"
#include "FileChooserDialog.xaml.h"
#include "PINPadDialog.xaml.h"
#include "Library/AppIndex.h"
#include "Library/SecurityManager.h"

namespace FinalUWP
{
	/// <summary>
	/// The main display page for the app. Displays a list of launchable app references,
	/// metadata information for the selected app, and a set of lockable controls for
	/// editing references and program settings.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

		property Windows::UI::Xaml::Interop::IBindableVector^ AppSource
		{
			Windows::UI::Xaml::Interop::IBindableVector^ get() { return appSource; }
			void set(Windows::UI::Xaml::Interop::IBindableVector^ value) { appSource = value; }
		}

		property Windows::UI::Xaml::Interop::IBindableVector^ MetaSource
		{
			Windows::UI::Xaml::Interop::IBindableVector^ get() { return metaSource; }
			void set(Windows::UI::Xaml::Interop::IBindableVector^ value) { metaSource = value; }
		}

	protected:
		// Cross-thread access methods for task chain lambdas
		void setAdminStateL(bool state);
		void notifyNoFile(UINT appIndex);

		void RunWin32Subroutine(byte actionID, Platform::String^ targetPath);
		bool CheckLaunchRequirements();

	private:
		Windows::UI::Xaml::Interop::IBindableVector^ appSource; // Bound to AppList
		Windows::UI::Xaml::Interop::IBindableVector^ metaSource; // Bound to DetailList
		FinalUWP::FileChooserDialog^ appPickerDialog; // Persistent app reference file chooser
		FinalUWP::PINPadDialog^ PINEntryDialog; // Persistent security PIN entry dialog
		bool hasLauncher; // TRUE if the launcher interface module (UWPProcessLauncher.exe) was found in the app package, FALSE if not

		void Launch_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void MoveUp_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void MoveDown_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void AddApp_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void RemoveApp_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Lock_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PassCode_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Reset_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void EditMetadata_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void AppList_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void AppList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void ExportSettings_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ImportSettings_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
