#pragma once

#include "CoreMinimal.h"
#include "LogFlowEntry.h"

/**
 * Abstract interface for all LogFlow log entry consumers.
 * 
 * Any component that wants to receive log entries from the FLogFlowDispatcher
 * must implement this interface. Current implementations are SLogFlowPanel
 * (edito panel) and FLogFlowFileWriter (session file writer).
 * 
 * This interface is the foundation of the Observer pattern used in LogFlow
 * to decouple LogFlowCore from its consumers. LogFlowCore only knows about
 * ILogFlowConsumers - never about concrete consumer implementations.
 * 
 * Usage:
 * @code
 * class FMyConsumer : ILogFlowConsumer
 * {
 * public:
 *		virtual void OnLogEntryReceived(const FLogFlowEntry& Entry) override;
 * };
 * @endcode 
 */
class LOGFLOWCORE_API ILogFlowConsumer
{
public:
	virtual ~ILogFlowConsumer() = default;

	/**
	 * Called by FLogFlowDispatcher when a new log entry is available.
	 * 
	 * Implementations must not block the calling thread. If heavy processing
	 * is needed (e.g. file I/O, UI updates), enqueue the entry and process
	 * it on the appropiate thread.
	 * 
	 * @param Entry The log entry that was just registered. Passed by const
	 *				reference - do not store the reference, copy the entry 
	 *				if you need to retain it beyond this call.
	 */
	virtual void OnLogFlowEntryReceived(const FLogFlowEntry& Entry) = 0;
};