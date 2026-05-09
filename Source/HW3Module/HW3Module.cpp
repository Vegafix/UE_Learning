#include "Modules/ModuleManager.h"

class FHW3Module : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_MODULE(FHW3Module, HW3Module)