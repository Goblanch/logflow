#include "LogFlowEditorModule.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "SLogFlowPanel.h"
#include "LogFlowEditorSettings.h"
#include "LogFlowSubsystem.h"
#include "Widgets/SBoxPanel.h"
#include "LogFlowEditorSettings.h"
#include "SLogFlowViewer.h"

#define LOCTEXT_NAMESPACE "LogFlowEditor"

static const FName LogFlowPanelTabName("LogFlowPanel");
static const FName LogFlowViewerTabName("LogFlowViewer");

void FLogFlowEditorModule::StartupModule()
{
	// Register the dockable tab with global tab manager.
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		LogFlowPanelTabName,
		FOnSpawnTab::CreateRaw(this, &FLogFlowEditorModule::SpawnLogFlowPanelTab))
		.SetDisplayName(LOCTEXT("LogFlowPanelTitle", "LogFlow Panel"))
		.SetTooltipText(LOCTEXT("LogFlowPanelTooltip",
			"Open the LogFlow logging panel to view real-time log entries."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());
	
	// Register Log Viewer tab
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		LogFlowViewerTabName,
		FOnSpawnTab::CreateRaw(this, &FLogFlowEditorModule::SpawnLogFlowViewerTab))
		.SetDisplayName(LOCTEXT("LogFlowViewerTitle", "LogFlow Viewer"))
		.SetTooltipText(LOCTEXT("LogFlowViewerTooltip",
			"Open the LogFlow session viewer to browse and read past log sessions."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());
	
	RegisterMenuExtensions();
	
	// Push editor preferences to subsystem on startup.
	if (ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get())
	{
		if (const ULogFlowEditorSettings* EditorSettings = ULogFlowEditorSettings::Get())
		{
			Subsystem->UpdateSettings(EditorSettings->ToRuntimeSettings());
		}
	}
}

void FLogFlowEditorModule::ShutdownModule()
{
	UnregisterMenuExtensions();
	
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LogFlowPanelTabName);
	
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LogFlowViewerTabName);
}

FName FLogFlowEditorModule::GetPanelTabName()
{
	return LogFlowPanelTabName;
}

TSharedRef<SDockTab> FLogFlowEditorModule::SpawnLogFlowPanelTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SLogFlowPanel)
		];
}

void FLogFlowEditorModule::RegisterMenuExtensions()
{
	WindowMenuExtender = MakeShareable(new FExtender);

	WindowMenuExtender->AddMenuExtension(
		"WindowLayout",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateLambda([](FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("LogFlowPanelMenuEntry", "LogFlow Panel"),
				LOCTEXT("LogFlowPanelMenuEntryTooltip",
					"Open the LogFlow real-time logging panel."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					FGlobalTabmanager::Get()->TryInvokeTab(
						FTabId(LogFlowPanelTabName));
				}))
			);
			
			MenuBuilder.AddMenuEntry(
				LOCTEXT("LogFlowViewerMenuEntry", "LogFlow Viewer"),
				 LOCTEXT("LogFlowViewerMenuEntryTooltip",
					"Open the LogFlow session viewer to browse past log sessions."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					FGlobalTabmanager::Get()->TryInvokeTab(
						FTabId(LogFlowViewerTabName));
				}))
			);
		})
	);

	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(WindowMenuExtender);
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(WindowMenuExtender);
}

void FLogFlowEditorModule::UnregisterMenuExtensions()
{
	if (WindowMenuExtender.IsValid())
	{
		FLevelEditorModule* LevelEditorModule = 
			FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
		
		if (LevelEditorModule != nullptr)
		{
			LevelEditorModule->GetMenuExtensibilityManager()
			->RemoveExtender(WindowMenuExtender);
		}
		
		WindowMenuExtender.Reset();
	}
}

FName FLogFlowEditorModule::GetViewerTabName()
{
	return LogFlowViewerTabName;
}

TSharedRef<SDockTab> FLogFlowEditorModule::SpawnLogFlowViewerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SLogFlowViewer)
		];
}

IMPLEMENT_MODULE(FLogFlowEditorModule, LogFlowEditor);

#undef LOCTEXT_NAMESPACE
