#pragma once

class IApp
{
public:
	virtual ~IApp() = default;

	virtual std::string GetName() const = 0;
	virtual bool Initialise() = 0;
	virtual void Uninitialise() = 0;
	virtual void Run() = 0;
};

extern std::unique_ptr<IApp> g_app;

#define REGISTER_SERVICE(appClass)                                                                                                                             \
	std::unique_ptr<IApp> g_app                                                                                                                                \
	{                                                                                                                                                          \
		static_cast<IApp *>(new appClass())                                                                                                                    \
	}
