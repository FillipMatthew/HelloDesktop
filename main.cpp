#include <memory>

#include "App.h"

using namespace std;

int main(int argc, char ** argv)
{
	if (g_app == nullptr)
		return 1;

	if (!g_app->Initialise())
		return 1;

	g_app->Run();

	g_app->Uninitialise();

	return 0;
}
