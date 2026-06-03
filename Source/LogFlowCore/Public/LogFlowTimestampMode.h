#pragma once

#include "CoreMinimal.h"
#include "LogFlowTimestampMode.generated.h"

/**
 * Controls how timestamp are displayed and written for each log entry
 */
UENUM(BlueprintType)
enum class ELogFlowTimestampMode : uint8
{
	/** Time elapsed since the start of the current PIE session. Format: HH:MM:SS.mmm */
	SessionTime UMETA(DisplayName = "Session Time"),
	
	/** Wall clock time of the local system at the moment the entry was created. Format: HH:MM:SS.mmm */
	SystemTime	UMETA(DisplayName = "System Time")
};