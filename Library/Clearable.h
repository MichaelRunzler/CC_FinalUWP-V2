#pragma once
#include "include.h"

namespace FinalUWP 
{
	/// <summary>
	/// Allows a C++/CX class to implement the ability to let other classes clear
	/// certain persistent data structures within it. For example, a dialog or page-type class
	/// might implement this interface to allow other pages or dialogs to effectively 're-initialize' it
	/// without actually calling its constructor again.
	/// </summary>
	public interface class Clearable
	{
	public:
		virtual void Clear();
	};
}