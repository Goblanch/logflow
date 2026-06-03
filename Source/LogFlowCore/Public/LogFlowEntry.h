#pragma once

#include "CoreMinimal.h"
#include "LogFlowSeverity.h"
#include "LogFlowEntry.generated.h"

/**
 * Represents a single inmutable log entry in the LogFlow system.
 * 
 * FLogFlowEntry is the core data type passed between all LogFlow components:
 * from the subsystem to the dispatcher, from the dispatcher to consumers
 * (panel, file writer), and stored in the session file.
 * 
 * Always construct entries via the static Create() factory method to ensure
 * all fields are correctly initialized.
 */
USTRUCT(BlueprintType)
struct LOGFLOWCORE_API FLogFlowEntry
{
	GENERATED_BODY()
	
	/** The log message text provided by the developer. */
	UPROPERTY(BlueprintReadOnly, Category = "LogFlow")
	FString Message;
	
	/** The severity level of this entry: Log, Warning or Error. */
	UPROPERTY(BlueprintReadOnly, Category = "LogFlow")
	ELogFlowSeverity Severity;
	
	/**
	 * The developer-assigned category tag for this entry.
	 * Used for filtering and color coding in the panel.
	 * Empty if no tag was provided.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LogFlow")
	FName Tag;
	
	/**
	 * Time elapsed since the start of the current PIE session at the moment
	 * this entry was created. Used when timestamp mode is set to Session Time.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LogFlow")
	FTimespan Timestamp;
	
	/**
	 * Wall clock time at the moment this entry was created.
	 * Used when timestamp mode is set to System Time.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LogFlow")
	FDateTime SystemTime;
	
	/** Default constructor. Produces an empty, uninitialized entry.
	 * Prefer Create() for constructing valid entries. */
	FLogFlowEntry() 
		: Severity(ELogFlowSeverity::Log)
		, Timestamp((FTimespan::Zero()))
		, SystemTime(FDateTime::Now()) {}

	/**
	 * Factory method. Creates a fully initialized FLogFlowEntry.
	 * 
	 * @param InMessage		The log message text. 
	 * @param InSeverity	The severity level of the entry.
	 * @param InTag			Optional category tag. Pass NAME_None to leave empty.
	 * @param InTimestamp	Time elapsed since the start of the PIE session.
	 * @return 
	 */
	static FLogFlowEntry Create(
		const FString& InMessage,
		ELogFlowSeverity InSeverity,
		FName InTag,
		FTimespan InTimestamp);
};