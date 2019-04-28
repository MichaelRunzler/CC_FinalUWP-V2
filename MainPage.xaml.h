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
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	protected:
		void setAdminStateL(BOOL state);
		void setAdminStateD(UINT delay);
		void notifyNoFile(UINT appIndex);

	private:
		Windows::UI::Xaml::Interop::IBindableVector^ appSource;
		Windows::UI::Xaml::Interop::IBindableVector^ metaSource;
		FinalUWP::FileChooserDialog^ appPickerDialog;
		FinalUWP::PINPadDialog^ PINEntryDialog;
		BOOL hasLauncher;

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
	};
}
