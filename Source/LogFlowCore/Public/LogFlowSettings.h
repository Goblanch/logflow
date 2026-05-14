#pragma once

#include "CoreMinimal.h"
#include "LogFlowTimestampMode.h"
#include "LogFlowSettings.generated.h"

/**
 * Holds a tag name and its associated display color.
 * Used in FLogFlowSettings to configure per-tag color coding in the panel.
 */
USTRUCT(BlueprintType)
struct LOGFLOWCORE_API FLogFlowTagConfig
{
	GENERATED_BODY()
	
	/** The tag name as used in LogMessage calls. */
	UPROPERTY(BlueprintReadWrite, Category = "LogFlow")
	FName TagName;
	
	/** The color used to display this tag in the panel. */
	UPROPERTY(BlueprintReadWrite, Category = "LogFlow")
	FLinearColor Color;
	
	FLogFlowTagConfig() : TagName(NAME_None), Color(FLinearColor::White) {}
};

/**
 * Runtime configuration container for the LogFlow system.
 * 
 * Holds all user-configurable options that control the behavior of the
 * subsystem, the file writer and the editor panel. Obtain the default
 * configuration via GetDefault().
 * 
 * In the editor this struct is populated from FLogFlowEditorSettings
 * (UDeveloperSettings). At runtime, it can be constructed manually.
 */
USTRUCT(BlueprintType)
struct LOGFLOWCORE_API FLogFlowSettings
{
	GENERATED_BODY()
	
	/**
	 * Directory where session log files are saved, relative to the project root. 
	 * Defaults to Saved/LogFlow/.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LogFlow|Storage")
	FString LogDirectory;
	
	/**
	 * Maximum number of session log files to keep on disk.
	 * When the limit is exceeded the oldest session is deleted automatically.
	 * Defaults to 10.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LogFlow|Storage")
	int32 MaxSessionHistory;
	
	/**
	 * Controls whether timestamps in the panel and session file show
	 * elapsed session time or wall clock system time.
	 * Defaults to SessionTime.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LogFlow|Display")
	ELogFlowTimestampMode TimestampMode;
	
	/**
	 * If true, the PIE session is automatically paused when an Error
	 * Entry is registered. Has no effect in package builds.
	 * Defaults to false.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LogFlow|Behavior")
	bool bBreakOnError;
	
	/**
	 * If true, the editor panel is cleared automatically at the start
	 * of each new PIE session.
	 * Defaults to false.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LogFlow|Behavior")
	bool bAutoClear;
	
	/**
	 * Per-tag color configuration. Each entry associates a tag name with 
	 * a display color used in the panel and Log Viewer.
	 * Empty by default - unconfigured tags use the panel default color.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LogFlow|Display")
	TArray<FLogFlowTagConfig> TagColors;
	
	FLogFlowSettings()
		: LogDirectory(TEXT("Saved/LogFlow/"))
		, MaxSessionHistory(10)
		, TimestampMode(ELogFlowTimestampMode::SessionTime)
		, bBreakOnError(false)
		, bAutoClear(false) {}
	
	/**
	 * Returns a default-initialized FLogFlowSettings instance.
	 * Equivalent to the default constructor - provided for readability
	 * at call sites that explicity want the default configuration.
	 * 
	 * @return A fully initialized FLogFlowSettings with default values.
	 */
	static FLogFlowSettings GetDefault();
};