#include "LogFlowCore/Public/LogFlowEntry.h"

FLogFlowEntry FLogFlowEntry::Create(
	const FString& InMessage, 
	ELogFlowSeverity InSeverity, 
	FName InTag, 
	FTimespan InTimestamp)
{
	FLogFlowEntry Entry;
	Entry.Message = InMessage;
	Entry.Severity = InSeverity;
	Entry.Tag = InTag;
	Entry.Timestamp = InTimestamp;
	Entry.SystemTime = FDateTime::Now();
	return Entry;
}
