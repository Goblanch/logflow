#include "LogFlowBlueprintLibrary.h"
#include "LogFlowSubsystem.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, LogFlowBlueprintLibrary);

void ULogFlowBlueprintLibrary::LogMessage(const FString& Message, FName Tag)
{
	ULogFlowSubsystem::LogMessage(Message, ELogFlowSeverity::Log, Tag);
}

void ULogFlowBlueprintLibrary::LogWarning(const FString& Message, FName Tag)
{
	ULogFlowSubsystem::LogMessage(Message, ELogFlowSeverity::Warning, Tag);
}

void ULogFlowBlueprintLibrary::LogError(const FString& Message, FName Tag)
{
	ULogFlowSubsystem::LogMessage(Message, ELogFlowSeverity::Error, Tag);
}
