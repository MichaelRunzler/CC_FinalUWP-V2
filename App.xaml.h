//
// App.xaml.h
// Declaration of the App class.
//

#pragma once

#include "App.g.h"
#include "Library/AppIndex.h"
#include "Library/SecurityManager.h"
#include <thread>
#include <chrono>

namespace FinalUWP
{
	/// <summary>
	/// Provides application-specific behavior to supplement the default Application class.
	/// </summary>
	ref class App sealed
	{	
	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	internal:
		App();

	private:
		template <typename T>
		static void writeData(Windows::Storage::StorageFolder^ parent, Platform::String^ name, std::vector<BYTE>& data, T&& lambda);

		template <typename T>
		static void readData(Windows::Storage::StorageFolder^ parent, Platform::String^ name, T&& lambda);

		template <typename T>
		static void readData(Windows::Storage::StorageFolder^ parent, Platform::String^ name, UINT start, UINT len, T&& lambda);

		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e);
	};
}
