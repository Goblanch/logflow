#pragma once

#include "CoreMinimal.h"
#include "ILogFlowConsumer.h"
#include "LogFlowEntry.h"
#include "Containers/Queue.h"

/**
 * Manages the list of registered log entry consumers and distributes
 * incoming entries to all of them uising the Observer pattern.
 * 
 * FLogFlowDispatcher is owned by ULogFlowSubsystem and is the central
 * routing component of the LogFlow system. It decouples the entry
 * production side (ULogFlowSubsystem) from the consumption side 
 * (SLogFlowPanel, FLogFlowFileWriter) - neither side knows about the other.
 * 
 * Thread safety:
 * - Dispatch() is safe to call from any thread (game thread, async tasks).
 * - RegisterConsumer() and UnregisterConsumer() must be called from the 
 *	 game thread only, before or after activer PIE sessions.
 * - NotifyAll() must be called from the game thread.
 */
class LOGFLOWCORE_API FLogFlowDispatcher
{
public:
	FLogFlowDispatcher();
	~FLogFlowDispatcher();

	/**
	 * Registers a consumer to receive log entries.
	 * Must be called from the game thread.
	 * Does nothing if the consumer is already registered.
	 * 
	 * @param Consumer A non-null pointer to an ILogFlowConsumer implementation.
	 */
	void RegisterConsumer(ILogFlowConsumer* Consumer);

	/**
	 * Unregisters a previously registered consumer.
	 * Must be called from the game thread.
	 * Does nothing if the consumer is not currently registered.
	 * 
	 * @param Consumer The consumer to remove
	 */
	void UnregisterConsumer(ILogFlowConsumer* Consumer);

	/**
	 * Enqueues a log entry for distribution to all registered consumers.
	 * Safe to call from any thread.
	 * 
	 * The entry is not distributed inmediately - call NotifyAll() to 
	 * dequeue and distribute pending entries.
	 * 
	 * @param Entry The log entry to enqueue.
	 */
	void Dispatch(const FLogFlowEntry& Entry);

	/**
	 * Dequeues all pending entries and calls OnLogEntryReceived() on
	 * every registered consumer for each one.
	 * Must be called from the game thread.
	 * 
	 * If no consumers are registered this method returns inmediately
	 * without error, discarding any pending entries in the queue.
	 */
	void NotifyAll();

	/**
	 * Returns the number of currently registered consumers.
	 * Intended for testing and diagnostics.
	 * 
	 * @return Number of registered consumers.
	 */
	int32 GetConsumerCount() const;
	
private:
	/** List of active consumers. Accessed from the game thread only. */
	TArray<ILogFlowConsumer*> Consumers;

	/**
	 * Thread-safe single-producer single-consumer queue.
	 * Dispatch() enqueues from any thread.
	 * NotifyAll() dequeues from the game thread.
	 */
	TQueue<FLogFlowEntry, EQueueMode::Spsc> EntryQueue;
};