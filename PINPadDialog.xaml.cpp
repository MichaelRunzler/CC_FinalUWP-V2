//
// PINPadDialog.xaml.cpp
// Implementation of the PINPadDialog class
//

#include "pch.h"
#include "PINPadDialog.xaml.h"

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

// The Content Dialog item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

FinalUWP::PINPadDialog::PINPadDialog()
{
	InitializeComponent();
	entered = "";
}

void FinalUWP::PINPadDialog::ContentDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args)
{
	if(PasswordEntry->Password->IsEmpty())
	{
		args->Cancel = TRUE;
		ErrorDisplay->Text = "Please enter at least 1 digit.";
		ErrorDisplay->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}else{
		entered = PasswordEntry->Password;
	}
}

void FinalUWP::PINPadDialog::ContentDialog_SecondaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args)
{
}


void FinalUWP::PINPadDialog::Num1_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	PasswordEntry->Password += "1";
}

void FinalUWP::PINPadDialog::Num2_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	PasswordEntry->Password += "2";
}

void FinalUWP::PINPadDialog::Num3_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	PasswordEntry->Password += "3";
}

void FinalUWP::PINPadDialog::Num4_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	PasswordEntry->Password += "4";
}

void FinalUWP::PINPadDialog::Num5_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	PasswordEntry->Password += "5";
}

void FinalUWP::PINPadDialog::Num6_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	PasswordEntry->Password += "6";
}

void FinalUWP::PINPadDialog::Num7_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	PasswordEntry->Password += "7";
}

void FinalUWP::PINPadDialog::Num8_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	PasswordEntry->Password += "8";
}

void FinalUWP::PINPadDialog::Num9_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	PasswordEntry->Password += "9";
}

void FinalUWP::PINPadDialog::Num0_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	PasswordEntry->Password += "0";
}


void FinalUWP::PINPadDialog::Del_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	if(!PasswordEntry->Password->IsEmpty())
	{
		std::wstring pass = std::wstring(PasswordEntry->Password->Data());
		pass.erase(pass.end() - 1);
		PasswordEntry->Password = ref new String(pass.c_str());
	}
}


void FinalUWP::PINPadDialog::Clear_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
	PasswordEntry->Password = "";
}


void FinalUWP::PINPadDialog::PasswordEntry_PasswordChanged(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	std::wstring pass = std::wstring(PasswordEntry->Password->Data());
	BOOL foundViolations = FALSE;
	for (auto ptr = pass.begin(); ptr < pass.end(); ptr++) 
	{
		WCHAR c = *ptr;
		if(!std::isdigit(c))
		{
			foundViolations = TRUE;
			std::wstring tmp = L"";
			for (WCHAR c : pass) 
				if (std::isdigit(c)) tmp += c;
			
			PasswordEntry->Password = ref new String(tmp.c_str());
			break;
		}
	}

	if (foundViolations) {
		ErrorDisplay->Text = "Only numeric digits are allowed.";
		ErrorDisplay->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
}

void FinalUWP::PINPadDialog::Clear()
{
	PasswordEntry->Password = "";
	entered = "";
	ErrorDisplay->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}
