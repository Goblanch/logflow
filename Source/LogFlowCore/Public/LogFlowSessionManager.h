#pragma once

#include "CoreMinimal.h"
#include "LogFlowSettings.h"
#include "LogFlowSessionInfo.h"

/**
 * Manages the lifecycle of LogFlow log sessions.
 * 
 * A session corresponds to a single PIE run. FLogFlowSessionManager is
 * responsible for generating session file names, tracking the active session
 * path, closing session cleanly and rotating the session history on disk 
 * when the configured limit is exceeded.
 * 
 * Owned and managed by ULogFlowSubsystem. Should not be instantiated
 * directly by user code.
 */
class LOGFLOWCORE_API FLogFlowSessionManager
{
public:
	/**
	 * Constructs a session manager with the given configuration.
	 * 
	 * @param InSettings The runtime configuration to use. The manager reads
	 *					 LogDirectory and MaxSessionHistory from this struct.
	 */
	explicit FLogFlowSessionManager(const FLogFlowSettings& InSettings);
	
	~FLogFlowSessionManager();

	/**
	 * 
	 * @return The full absolute path to the session file that should be opened
	 *		   for writing. Empty string if the session could not be started.
	 */
	FString BeginSession();

	/**
	 * Ends the current log session.
	 * 
	 * Clears the active session path. Does not delete any files.
	 * Safe to call even if no session is currently active.
	 */
	void EndSession();

	/**
	 * Returns whether a session is currently active.
	 * 
	 * @return True if BeginSession() has been called and EndSession has not.
	 */
	bool IsSessionActive() const;

	/**
	 * Returns the full path to the currently active session file.
	 * Returns an empty string if no session is active.
	 * 
	 * @return Absolute path to the active session file, or empty string.
	 */
	FString GetActiveSessionPath() const;

	/**
	 * Updates the runtime configuration used by the session manager.
	 * Safe to call between sessions. Do not call during an active session.
	 * 
	 * @param InSettings The new configuration to apply.
	 */
	void UpdateSettings(const FLogFlowSettings& InSettings);
	
	/**
	 * Generates a unique session file name based on the current date and time.
	 * Format: LogFlow_YYYYMMDD_HHMMSS.txt
	 * 
	 * @return The generated file name including the .txt extension.
	 */
	FString BuildFileName() const;
	
	/**
	 * Deletes the oldest session files if the number of session files in 
	 * the log directory exceeds MaxSessionHistory.
	 * 
	 * Files are stored by name (which encodes creation time via the
	 * LogFlow_YYYYMMDD_HHMMSS format) and the oldest are deleted first.
	 */
	void RotateHistory() const;

	/**
	 * Returns a copy of the current session index.
	 * Used by SLogFlowViewer to populate the session list.
	 * Sessions are stored from most recent to oldest.
	 * 
	 * @return Array of FLogFlowSessionInfo entries.
	 */
	TArray<FLogFlowSessionInfo> GetSessionIndex() const;
	
	/**
	 * Returns a delegate broadcast when the session index changes.
	 * SLogFlowViewer subscribes to this to refresh the session list.
	 */
	DECLARE_MULTICAST_DELEGATE(FOnSessionIndexChanged);
	FOnSessionIndexChanged OnSessionIndexChanged;
	
private:

	/**
	 * Returns the absolute path to the log directory on disk.
	 * The path is constructed from the project directory and the configured
	 * LogDirectory relative path.
	 * 
	 * @return Absolut path to the log directory.
	 */
	FString GetAbsoluteLogDirectory() const;

	/**
	 * Ensures the log directory exists on disk, creating it if necessary.
	 * 
	 * @return True if the directory exists or was created successfully.
	 */
	bool EnsureDirectoryExists() const;
	
	/** Runtime configuration. Controls directory and history limit. */
	FLogFlowSettings Settings;
	
	/** Full path to the active session file. Empty if no session is active. */
	FString ActiveSessionPath;
	
	/**
	 * Loads the session index from LogFlow_Index.json if it exists.
	 * Called during construction.
	 */
	void LoadIndex();
	
	/**
	 * Saves the current session index to LogFlow_Index.json.
	 * Called after BeginSession() and RotateHistory().
	 */
	void SaveIndex() const;

	/**
	 * Returns the absolute path to the index file.
	 * 
	 * @return Absolute path to LogFlow_Index.json.
	 */
	FString GetIndexFilePath() const;
	
	/**
	 * Updates the file size of the active session entry in the index.
	 * Called by EndSession() after the file writer has closed the file.
	 */
	void UpdateActiveSessionSize();
	
	/** In-memory session index. Loaded from disk on construction. */
	TArray<FLogFlowSessionInfo> SessionIndex;
	
	/** Info for the session currently begin written. */
	FLogFlowSessionInfo ActiveSessionInfo;
	
};