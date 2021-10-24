#include "../../lsMisc/stdosd/stdosd.h"
#include "common.h"

using namespace Ambiesoft::stdosd;

std::wstring GetIniPath() 
{
	return stdCombinePath(
		stdGetParentDirectory(stdGetModuleFileName()),
		L"ShowQL.ini");
}
