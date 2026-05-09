#include "Modules/ModuleManager.h"

class FHW3TestModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_MODULE(FHW3TestModule, HW3TestModule)