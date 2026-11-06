#pragma once

// Must be included before `import std;` (GCC modules + MinGW headers).
// winnt.h pulls in <x86intrin.h> from inside an extern "C" block, which
// clashes with the C++ linkage declarations exported by `std`. Including it
// first (outside extern "C") makes winnt.h's include a no-op.
#include <x86intrin.h>

#include <Windows.h>
