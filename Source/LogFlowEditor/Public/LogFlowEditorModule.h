#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Framework/Docking/TabManager.h"

/**
 * Entry point for the LogFlowEditor module.
 * 
 * Registers the LogFlow dockable panel and Log Viewer windows in the UE5
 * editor tab system on startup, and unregisters them cleanly on shutdown.
 * 
 * The panel is accessible from Window -> LogFlow Panel in the editor menu.
 */
class FLogFlowEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Returns the tab name used to identify the LogFlow panel tab.
	 * Used by FGlobalTabManager to register and spawn the tab.
	 */
	static FName GetPanelTabName();
	
private:
	/**
	 * Spawns the LogFlow panel tab contents when the tab is opnened.
	 * 
	 * @param SpawnTabArgs Arguments provided by the tab manager on spawn.
	 * @return The spawned tab containing the LogFlow panel widget.
	 */
	TSharedRef<SDockTab> SpawnLogFlowPanelTab(const FSpawnTabArgs& SpawnTabArgs);

	/**
	 * Extends the editor Window menu to add the LogFlow Panel entry.
	 * Called during StartUpModule().
	 */
	void RegisterMenuExtensions();

	/**
	 * Removes the LogFlow entry from the editor Window menu.
	 * Called during ShutdownModule().
	 */
	void UnregisterMenuExtensions();
	
	/** Handle used to unregister the Window menu extension on shutdown. */
	TSharedPtr<FExtender> WindowMenuExtender;
};