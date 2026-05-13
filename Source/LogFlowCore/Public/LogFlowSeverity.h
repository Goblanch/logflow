#pragma once

#include "CoreMinimal.h"
#include "LogFlowSeverity.generated.h"

/**
 * Defines the severity levels available for LogFlow log entries.
 * Used to classify entries in the panel and session file, and to
 * control filtering and visual representation.
 */
UENUM(BlueprintType)
enum class ELogFlowSeverity : uint8
{
	/** Informational message. Displayed in neutral color in the panel. */
	Log		UMETA(displayName = "Log"),
	
	/** Non-critical anomaly. Displayed in yellow in the panel. */
	Warning UMETA(displayName = "Warning"),
	
	/** Critical failure or error condition. Displayed in red in the panel.
	 *  If bBreakOnError is active in settings, triggers a PIE pause. */
	Error	UMETA(displayName = "Error")
};