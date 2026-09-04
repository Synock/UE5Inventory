#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/** Plugin-wide log category. Use instead of LogTemp so log filtering works in production. */
INVENTORYPLUGIN_API DECLARE_LOG_CATEGORY_EXTERN(LogInventoryPlugin, Log, All);

class FInventoryPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
