#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LogFlowSettings.h"
#include "LogFlowEditorSettings.generated.h"

/**
 * LogFlow user preferences, exposed in Edit -> Editor Preferences -> Plugins -> LogFlow.
 * 
 * FLogFlowEditorSettings is a UDeveloperSettings subclass - the engine
 * persists all UPROPERTY fields automatically to the project's Saved/Config
 * directory between editor sessions. No manual serialization is needed.
 * 
 * When the user changes any value, PostEditChangeProperty() propagates
 * the new configuration to ULogFlowSubsystem and SLogFlowPanel in real time
 * without requiring an editor restart.
 */
UCLASS(Config=EditorPerProjectUserSettings, meta=(DisplayName="LogFlow"))
class LOGFLOWEDITOR_API ULogFlowEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	ULogFlowEditorSettings();

	/**
	 * Returns the singleton settings instance.
	 * Equivalent to GetDefault<ULogFlowEditorSettings>().
	 * 
	 * @return Pointer to the settings instance. Never null.
	 */
	static ULogFlowEditorSettings* Get();

	/**
	 * Builds and return an FLogFlowSettings struct populated from 
	 * the current editor preferences values. Used to pass configuration
	 * to LogFlowCore components.
	 * 
	 * @return FLogFlowSettings reflecting the current preferences.
	 */
	FLogFlowSettings ToRuntimeSettings() const;
	
	// -- UDeveloperSettings interface -----------------------------------------------------
	
	/** Return the settings category shown in Editor Preferences. */
	virtual FName GetCategoryName() const override;
	
	/** Returns the section name shown within the category. */
	virtual FText GetSectionText() const override;
	
	/** Returns the section description shown in Editor Preferences. */
	virtual FText GetSectionDescription() const override;
	
#if WITH_EDITOR
	/**
	 * Called by the engine when the user changes any property in the
	 * preferences panel. Propagates to the subsystem and panel 
	 * in real time.
	 */
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
	// -- Settings fields -------------------------------------------------------------------------------------------

	/**
	 * Directory where session log files are saved, relative to the project root.
	 * Default: Saved/LogFlow/
	 */
	UPROPERTY(Config, EditAnywhere, Category="Storage",
		meta=(DisplayName="Log Directory",
		Tooltip="Directory where LogFlow session files are saved, relative to the project root."))
	FString LogDirectory;

	/**
	 * Maximum number of session log files to keep on disk.
	 * Older files are deleted automatically when the limit is exceeded.
	 * Default: 10
	 */
	UPROPERTY(Config, EditAnywhere, Category="Storage",
		meta=(DisplayName="Max Session History",
			Tooltip="Maximum number of session log files to keep. Oldest files are deleted when the limit is exceeded.",
			ClampMin="1", ClampMax="100"))
	int32 MaxSessionHistory;
	
	/**
	 * Controls how timestamps are displayed in the panel and written
	 * to session files.
	 * Default: SessionTime
	 */
	UPROPERTY(Config, EditAnywhere, Category="Display",
		meta=(DisplayName="Timestamp Mode",
			  ToolTip="SessionTime shows time elapsed since PIE start. SystemTime shows wall clock time."))
	ELogFlowTimestampMode TimestampMode;
	/**
	 * If enabled, the PIE session pauses automatically when an Error
	 * entry is registered. Has no effect in packaged builds.
	 * Default: false
	 */
	UPROPERTY(Config, EditAnywhere, Category="Behavior",
		meta=(DisplayName="Break on Error",
			  ToolTip="Automatically pause PIE when an Error entry is logged."))
	bool bBreakOnError;

	/**
	 * If enabled, the LogFlow panel clears automatically at the start
	 * of each new PIE session.
	 * Default: false
	 */
	UPROPERTY(Config, EditAnywhere, Category="Behavior",
		meta=(DisplayName="Auto-Clear Panel on PIE Start",
			  ToolTip="Clear the LogFlow panel automatically when a new PIE session starts."))
	bool bAutoClear;

	/**
	 * Per-tag color configuration. Each entry associates a tag name
	 * with a display color used in the panel and Log Viewer.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Display",
		meta=(DisplayName="Tag Colors",
			  ToolTip="Assign a custom display color to each tag name."))
	TArray<FLogFlowTagConfig> TagColors;
	
};