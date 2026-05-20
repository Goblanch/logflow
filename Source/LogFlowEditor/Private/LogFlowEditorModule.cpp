#include "LogFlowEditorModule.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

// Forward declare - SLogFLowPanel will be implemented in #16
// Replace with real widget once available
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "LogFlowEditor"

static const FName LogFlowPanelTabName("LogFlowPanel");

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
	
	RegisterMenuExtensions();
}

void FLogFlowEditorModule::ShutdownModule()
{
	UnregisterMenuExtensions();
	
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LogFlowPanelTabName);
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
			// Placeholder - replace with SNew(SLogFlowPanel) in #16
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LogFlowPanelPlaceholder", "LogFlow Panel - Coming in #16"))
			]
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

IMPLEMENT_MODULE(FLogFlowEditorModule, LogFlowEditor);

#undef LOCTEXT_NAMESPACE
