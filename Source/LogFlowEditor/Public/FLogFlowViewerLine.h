#pragma once

#include "CoreMinimal.h"
#include "LogFlowSeverity.h"

/**
 * Represents a single parsed line from a session log file.
 * Used as the data model for the LogViewer content list.
 */
struct FLogFlowViewerLine
{
	/** The raw text of the line as read from the list. */
	FString RawText;
	
	/**
	 * Severity parsed from the line. Used for color coding.
	 * Defaults to Log if the severity field cannot be parsed.
	 */
	ELogFlowSeverity Severity;
	
	FLogFlowViewerLine() : Severity(ELogFlowSeverity::Log) {}
	
	FLogFlowViewerLine(const FString& InRawText, ELogFlowSeverity InSeverity)
		: RawText(InRawText), Severity(InSeverity) {}
	
	
	static ELogFlowSeverity ParseSeverity(const FString& Line)
	{
		if (Line.Contains(TEXT("[WARNING]")))
		{
			return ELogFlowSeverity::Warning;
		}
		if (Line.Contains(TEXT("[ERROR  ]")))
		{
			return ELogFlowSeverity::Error;
		}
		return ELogFlowSeverity::Log;
	}
};