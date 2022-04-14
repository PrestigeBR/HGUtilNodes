#pragma once

#include "Modules/ModuleInterface.h"

class FCustomK2Module : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};