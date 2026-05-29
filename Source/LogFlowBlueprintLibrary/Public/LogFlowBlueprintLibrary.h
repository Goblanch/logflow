#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LogFlowSeverity.h"
#include "LogFlowBlueprintLibrary.generated.h"

/**
 * Blueprint Function Library that exposes the LogFlow API as Blueprint nodes.
 * 
 * All three nodes delegate directly to ULogFlowSubsystem::LogMessage()
 * with the corresponding severity level. No logic lives here - this class
 * is a pure wrapper over LogFlowCore.
 * 
 * Nodes are grouped under the LogFlow category in the Blueprint node search
 * and are findable by searching: log, debug, print, warning, error, logflow.
 */
UCLASS()
class LOGFLOWBLUEPRINTLIBRARY_API ULogFlowBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	 * Registers an informational log entry with LogFlow system.
	 * Displayed in the LogFlow panel with neutral color.
	 * 
	 * @param Message The log message text.
	 * @param Tag Optional category tag for filtering and color coding.
	 *			  Leave empty to log without tag.
	 */
	UFUNCTION(BlueprintCallable, Category="LogFlow",
		meta=(
			DisplayName="Log Message",
			Keywords="log debug print logflow message",
			Tooltip="Registers an informational log entry in the LogFlow panel.",
			AdvancedDisplay="Tag"
		))
	static void LogMessage(
		const FString& Message,
		FName Tag = NAME_None);

	/**
	 * Registers a warning log entry with the LogFlow system.
	 * DIsplayed in the LogFlow panel with amber color.
	 * 
	 * @param Message The warning message text.
	 * @param Tag Optional category tag for filtering and color coding.
	 *			  Leave empty to log without tag.
	 */
	UFUNCTION(BlueprintCallable, Category="LogFlow",
		meta=(
			DisplayName="Log Warning",
			Keywords="log warning debug print logflow",
			Tooltip="Registers a warning entry in the LogFlow panel.",
			AdvancedDisplay="Tag"
		))
	static void LogWarning(
		const FString& Message,
		FName Tag = NAME_None);
	
	/**
	 * Registers an error log entry with the LogFlow system.
	 * Displayed in the LogFlow panel with red color.
	 * If Break on Error is active in settings, PIE will pause automatically.
	 *
	 * @param Message The error message text.
	 * @param Tag     Optional category tag for filtering and color coding.
	 *                Leave empty to log without a tag.
	 */
	UFUNCTION(BlueprintCallable, Category="LogFlow",
		meta=(
			DisplayName="Log Error",
			Keywords="log error debug print logflow critical",
			ToolTip="Registers an error entry in the LogFlow panel. Pauses PIE if Break on Error is active.",
			AdvancedDisplay="Tag"
		))
	static void LogError(
		const FString& Message,
		FName Tag = NAME_None);
	
};