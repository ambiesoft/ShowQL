// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <string>

#include "framework.h"
#include "../../lsMisc/SessionGlobalMemory/SessionGlobalMemory.h"
#include "../../lsMisc/CHandle.h"
#include "../../profile/cpp/Profile/include/ambiesoft.profile.h"
#include "../../lsMisc/stdosd/stdosd.h"
#include "../../lsMisc/OpenCommon.h"

#define I18N(s) (s)

using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;

#endif //PCH_H
