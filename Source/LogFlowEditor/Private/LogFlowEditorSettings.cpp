#include "LogFlowEditorSettings.h"
#include "LogFlowSubsystem.h"
#include "SLogFlowPanel.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "LogFlowEditorSettings"

ULogFlowEditorSettings::ULogFlowEditorSettings()
{
	LogDirectory		= TEXT("Saved/LogFlow/");
	MaxSessionHistory	= 10;
	TimestampMode		= ELogFlowTimestampMode::SessionTime;
	bBreakOnError		= false;
	bAutoClear			= false;
}

ULogFlowEditorSettings* ULogFlowEditorSettings::Get()
{
	return GetMutableDefault<ULogFlowEditorSettings>();
}

FLogFlowSettings ULogFlowEditorSettings::ToRuntimeSettings() const
{
	FLogFlowSettings RuntimeSettings;
	RuntimeSettings.LogDirectory		= LogDirectory;
	RuntimeSettings.MaxSessionHistory	= MaxSessionHistory;
	RuntimeSettings.TimestampMode		= TimestampMode;
	RuntimeSettings.bBreakOnError		= bBreakOnError;
	RuntimeSettings.bAutoClear			= bAutoClear;
	RuntimeSettings.TagColors			= TagColors;
	return RuntimeSettings;	
}

FName ULogFlowEditorSettings::GetCategoryName() const
{
	return FName("Plugins");
}

FText ULogFlowEditorSettings::GetSectionText() const
{
	return LOCTEXT("SectionText", "LogFlow");
}

FText ULogFlowEditorSettings::GetSectionDescription() const
{
	return LOCTEXT("SectionDescription",
		"Configure the LogFlow custom logging panel and session file settings.");
}

#if WITH_EDITOR

void ULogFlowEditorSettings::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	const FLogFlowSettings NewSettings = ToRuntimeSettings();
	
	// DIAGNÓSTICO
	UE_LOG(LogTemp, Warning, TEXT("LogFlow: PostEditChangeProperty called — bAutoClear = %s"),
		NewSettings.bAutoClear ? TEXT("true") : TEXT("false"));
	
	// Propagate to subsystem - updates session manager and file writer
	if (ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get())
	{
		Subsystem->UpdateSettings(NewSettings);
		// DIAGNÓSTICO
		UE_LOG(LogTemp, Warning, TEXT("LogFlow: Settings pushed to subsystem"));
	}
}

#endif

#undef LOCTEXT_NAMESPACE
