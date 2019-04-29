#pragma once

namespace FinalUWP 
{
	/// <summary>
	/// Bindable C++/CX equivalent of a Platform::String-only C++ std::pair.
	/// Stores a key-value pair and provides access via properties.
	/// </summary>
	public ref class MetaPair sealed
	{
	public:
		MetaPair();
		MetaPair(Platform::String^ name, Platform::String^ value);

		property Platform::String^ Name {
			Platform::String^ get() { return name; }
			void set(Platform::String^ newName) { this->name = newName; }
		}

		property Platform::String^ Value {
			Platform::String^ get() { return value; }
			void set(Platform::String^ newValue) { this->value = newValue; }
		}

	private:
		Platform::String^ name;
		Platform::String^ value;
	};
}
