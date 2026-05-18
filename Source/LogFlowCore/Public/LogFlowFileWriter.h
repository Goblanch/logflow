#pragma once

#include "CoreMinimal.h"
#include "ILogFlowConsumer.h"
#include "LogFlowEntry.h"
#include "LogFlowSettings.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Containers/Queue.h"
#include <atomic>

/**
 * Asynchronous log entry writer that operates on a dedicated secondary thread.
 * 
 * FLogFileWriter implements both FRunnable (to run on its own thread) and
 * ILogFlowConsumer (to receive entries from FLogFlowDispatcher). When an entry
 * is received via OnLogEntryReceived() it is enqueued in a thread-safe queue.
 * The background thread continuously dequeues and writes entries to the active
 * session file.
 * 
 * Lifecycle:
 * 1. Construct FlogFileWriter
 * 2. Call OpenSession() with the path returned by FLogFlowSessionManager::BeginSession()
 * 3. Register with FLogFlowDispatcher - entries will start flowing in.
 * 4. Call Stop() to flush pending entries, close the file and shut down the thread.
 */
class LOGFLOWCORE_API FLogFlowFileWriter : public FRunnable, public ILogFlowConsumer
{
public:
	/**
	 * Construct the file writer and starts the background thread.
	 * 
	 * @param InSettings Runtime configuration. Used to read timestamp mode.
	 */
	explicit FLogFlowFileWriter(const FLogFlowSettings& InSettings);
	
	/** Destructor. Calls Stop() if the thread is still running. */
	virtual ~FLogFlowFileWriter() override;
	
	// -- FRunnable -------------------------------------------------------------------------------------
	
	/** FRunnable interface. Called once when thread starts. */
	virtual bool Init() override;

	/**
	 * FRunnable interface. Main loop of the background thread.
	 * Continuously dequeues entries and writes them to the session file
	 * until bStopRequested is set to true.
	 * 
	 * @return Exit code. Always return 0.
	 */
	virtual uint32 Run() override;

	/**
	 * FRunnable interface. Signals the background thread to stop.
	 * Does not block - the thread will finish its current write cycle 
	 * and then exit.
	 */
	virtual void Stop() override;
	
	/** FRunnable interface. Called once after the thread exits. */
	virtual void Exit() override;
	
	// -- ILogFlowConsumer -------------------------------------------------------------------------------------

	/**
	 * ILogFlowConsumer interface. Called by FLogFlowDispatcher when a new 
	 * entry is available. Enqueues the entry for asynchronous writing.
	 * Safe to call from any thread.
	 * 
	 * @param Entry The log entry to write.
	 */
	virtual void OnLogFlowEntryReceived(const FLogFlowEntry& Entry) override;
	
	// -- Session Management ------------------------------------------------------------------------------------

	/**
	 * Opens a session file for writing.
	 * Must be called before entries start arriving via OnLogEntryReceived().
	 * If a file is already open it will be closed before opening the new one.
	 * 
	 * @param FilePath Absolute path to the session file to create or overwrite.
	 * @return True if the file was opened successfully.
	 */
	bool OpenSession(const FString& FilePath);

	/**
	 * Flushes all pending entries to disk, closes the active session file
	 * and signals the background thread to stop.
	 * 
	 * Blocks until the background thread has exited cleanly.
	 * Safe to call multiple times.
	 */
	void CloseSession();

	/**
	 * Returns whether the file writer currently has an open session file.
	 * 
	 * @return True if a session file is open and ready to receive entries.
	 */
	bool IsSessionOpen() const;

	/**
	 * Updates the runtime configuration.
	 * Safe to call between sessions only.
	 *
	 * @param InSettings The new configuration to apply.
	 */
	void UpdateSettings(const FLogFlowSettings& InSettings);
	
private:
	/**
	 * Formats a single log entry as a text line and writes it to the 
	 * active session file.
	 * 
	 * Format: [TIMESTAMP] [SEVERITY] [TAG] Message
	 * Severity: field is padded to 7 characters for aligment:
	 *		[LOG    ] [WARNING] [ERROR  ]
	 * 
	 * @param Entry The entry to format and write.
	 */
	void WriteEntry(const FLogFlowEntry& Entry);

	/**
	 * Formats the timestamp filed of a log entry based on the configured
	 * timestamp mode.
	 * 
	 * @param Entry The entry whose timestamp to format.
	 * @return Formatted timestamp string. Format: HH:MM:SS.mmm
	 */
	FString FormatTimestamp(const FLogFlowEntry& Entry) const;

	/**
	 * Formats the severity field of a log entry, padded to 7 characters.
	 * 
	 * @param Severity The severity level to format.
	 * @return Padded severity string: "LOG    ", "WARNING" or "ERROR  ".
	 */
	static FString FormatSeverity(ELogFlowSeverity Severity);

	/**
	 * Drains all remaining entries from the queue and writes them to disk.
	 * Called during CloseSession() to ensure no entries are lost on shutdown.
	 */
	void FlushPendingEntries();
	
	/** Runtime configuration. */
	FLogFlowSettings Settings;
	
	/** Handle to the active session file. Null if no session is open. */
	FArchive* FileHandle;
	
	/** The background thread managed by this writer. */
	FRunnableThread* Thread;
	
	/**
	 * Thread-safe queue between the game thread (producer via
	 * OnLogEntryReceived) and the background thread (consumer via Run()).
	 */
	TQueue<FLogFlowEntry, EQueueMode::Spsc> EntryQueue;
	
	/** Set to true by Stop() to signal the background thread to exit. */
	std::atomic<bool> bStopRequested;
	
	/** Set to true by OpenSession(), false by CloseSession(). */
	std::atomic<bool> bSessionOpen;
};