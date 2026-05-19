#pragma once
#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "LogFlowSettings.h"
#include "LogFlowSeverity.h"
#include "LogFlowDispatcher.h"
#include "LogFlowSessionManager.h"
#include "LogFlowFileWriter.h"
#include "LogFlowSubsystem.generated.h"


/**
 * Central facade and public entry point of the LogFlow system.
 * 
 * ULogFlowSubsystem is a UEngineSubsystem - the engine manages its lifecycle
 * automatically. It is created when the engine starts and destroyed when it
 * shuts down. No manual initialization is required from user code.
 * 
 * All log registration calls from user C++ code go through the static
 * LogMessage() method. ULogFLowBlueprintLibrary delegates to this method
 * for Blueprint calls.
 * 
 * Internally ULogFlowSubsystem owns and coordinates:
 *  - FLogFlowDispatcher: routes entries to registered consumers.
 *  - FLogFlowSessionManager: manages session file lifecycle.
 *  - FLogFlowFileWriter: writes entries to disk asynchronously.
 *  
 * Usage from C++:
 * @code 
 * ULogFlowSubsystem::LogMessage(
 *		TEXT("Player health dropped to zero"),
 *		ELogFlowSeverity::Error,
 *		FName("Combat")
 *	);
 *	@endcode
 */
UCLASS()
class LOGFLOWCORE_API ULogFlowSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:
	
	// -- USubsystem interface -----------------------------------------------------

	/**
	 * Called by the engine when the subsystem is created.
	 * Initializes the dispatcher, session manager and file writer, 
	 * registers the file writer as a consumer and starts the first session.
	 * 
	 * @param Collection The subsystem collection that owns this subsystem.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Called by the engine when the subsystem is about to be destroyed.
	 * Closes all the active session, unregisters consumers and releases
	 * all owned resources cleanly.
	 */
	virtual void Deinitialize() override;
	
	// -- Public API -------------------------------------------------------------------

	/**
	 * Registers a log entry with the LogFlow system.
	 * 
	 * This is the primary entry point for all logging calls from user code.
	 * The call is thread-safe and returns immediately - all heavy work
	 * (file I/O, panel update) happens asynchronously.
	 * 
	 * @param Message The log message text.
	 * @param Severity The severity level: Log, Warning or Error.
	 * @param Tag Optional category tag. Pass NAME_None to omit.
	 */
	UFUNCTION(BlueprintCallable, Category = "LogFlow")
	static void LogMessage(
		const FString& Message,
		ELogFlowSeverity Severity,
		FName Tag = NAME_None);

	/**
	 * Return the active runtime settings.
	 * Intended for use by LogFlowEditor to read current configuration.
	 * 
	 * @return A const reference to the current FLogFlowSettings.
	 */
	const FLogFlowSettings& GetSettings() const;

	/**
	 * Updates the runtime settings and propagates them to all owned components.
	 * Called by FLogFlowEditorSettings when the user changes preferences.
	 * 
	 * @param NewSettings The updated configuration to apply.
	 */
	void UpdateSettings(const FLogFlowSettings& NewSettings);

	/**
	 * Returns the global ULogFlowSubsystem instance.
	 * Returns nullptr if the engine has not yet created the subsystem
	 * or if GEngine is not available.
	 * 
	 * @return Pointer to the subsystem instance, or nullptr.
	 */
	static ULogFlowSubsystem* Get();

	/**
	 * Returns the dispatcher owned by this subsystem.
	 * Intended for use by LogFlowEditor to register the panel as a consumer.
	 * 
	 * @return Pointer to the dispatcher. Never null after Initialize().
	 */
	FLogFlowDispatcher* GetDispatcher() const;
	
private:
	
	// -- PIE event handlers ---------------------------------------------------------
	
	/** Called when a PIE session starts. Opens a new log session. */
	void OnBeginPIE(bool bIsSimulating);
	
	/** Called when a PIE session ends. Closes the active log session. */
	void OnEndPIE(bool bIsSimulating);
	
	// -- Helpers --------------------------------------------------------------------
	
	/** Starts a new log session: generates the file path and opens the writer. */
	void BeginLogSession();
	
	/** Ends the current log session: closes the writer. */
	void EndLogSession();
	
	// -- Owned Components -------------------------------------------------------------
	
	/** Routes log entries to all registered consumers. */
	TUniquePtr<FLogFlowDispatcher> Dispatcher;
	
	/** Manages session file names and history rotation. */
	TUniquePtr<FLogFlowSessionManager> SessionManager;
	
	/**
	 * Writes log entries to disk asynchronously.
	 * Also registered as a consumer in the dispatcher.
	 */
	TUniquePtr<FLogFlowFileWriter> FileWriter;
	
	/** Active runtime configuration. */
	FLogFlowSettings Settings;
	
	/** Handles for PIE delegate subscription. Stored for safe unsubscription. */
	FDelegateHandle BeginPIEHandle;
	FDelegateHandle EndPIEHandle;
	
	/**
	 * Tracks the PIE session start time for session-relative timestamps.
	 * Set in OnBeginPIE and used by LogMessage() to compute FTimespan.
	 */
	FDateTime PIESessionStartTime;
};
