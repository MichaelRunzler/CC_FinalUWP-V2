//
// PINPadDialog.xaml.h
// Declaration of the PINPadDialog class
//

#pragma once

#include "PINPadDialog.g.h"
#include "Library/Clearable.h"

namespace FinalUWP
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class PINPadDialog sealed : Clearable
	{
	public:
		PINPadDialog();

		property Platform::String^ Entered
		{
			Platform::String^ get() { return entered; }
		}

		// Inherited via Clearable
		virtual void Clear();

	private:
		Platform::String^ entered;

		void ContentDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void ContentDialog_SecondaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void Num1_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Num2_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Num3_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Num4_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Num5_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Num6_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Num7_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Num8_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Num9_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Num0_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Del_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Clear_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PasswordEntry_PasswordChanged(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
