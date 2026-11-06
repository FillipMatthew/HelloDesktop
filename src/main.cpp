import std;

#include "App.h"

using namespace std;

int main(int argc, char ** argv)
{
	if (nullptr == g_app)
		return 1;

	if (false == g_app->Initialise())
		return 1;

	g_app->Run();

	g_app->Uninitialise();

	return 0;
}
