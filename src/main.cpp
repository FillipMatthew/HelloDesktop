// Platform headers must precede `import std;` (GCC modules)
#include "Platform.h"

import std;

#include "App.h"
#include "Log.h"

using namespace std;

int main(int argc, char ** argv)
{
	AttachParentConsole();

	if (nullptr == g_app)
		return 1;

	if (false == g_app->Initialise())
		return 1;

	g_app->Run();

	g_app->Uninitialise();

	return 0;
}
