#pragma once

#include "MetaEdit.g.h"
#include "FileChooserDialog.xaml.h"
#include "Library/MetaPair.h"
#include "MainPage.xaml.h"

namespace FinalUWP
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class MetaEdit sealed
	{
	public:
		MetaEdit();

		property Windows::UI::Xaml::Interop::IBindableVector^ MetaData {
			Windows::UI::Xaml::Interop::IBindableVector^ get() { return metaSource; }
			void set(Windows::UI::Xaml::Interop::IBindableVector^ newVal) { this->metaSource = newVal; }
		}

	protected:
		void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

	private:
		UINT editingIndex;
		Windows::UI::Xaml::Interop::IBindableVector^ metaSource;
		FinalUWP::FileChooserDialog^ pathEditor;

		void Cancel_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Commit_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void AddEntry_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void DeleteEntry_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PathEdit_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
