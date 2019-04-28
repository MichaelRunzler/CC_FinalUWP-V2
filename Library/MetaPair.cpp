#include "pch.h"
#include "MetaPair.h"

using namespace Platform;

FinalUWP::MetaPair::MetaPair()
{
	name = "";
	value = "";
}

FinalUWP::MetaPair::MetaPair(String^ pName, String^ pValue) 
{
	name = pName;
	value = pValue;
}
