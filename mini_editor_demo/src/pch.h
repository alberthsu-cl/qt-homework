#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

// Keep MFC's min/max out of std::min/std::max, which Interface.h's <atomic>
// and <condition_variable> pull in.
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <afxwin.h>
#include <afxext.h>
