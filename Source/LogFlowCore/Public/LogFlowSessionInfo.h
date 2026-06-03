#pragma once

#include "CoreMinimal.h"
#include "LogFlowSessionInfo.generated.h"

/**
 * Stores metadata about a single saved LogFlow session.
 * Used by FLogFlowSessionManager to maintain the session index
 * and by SLogFlowViewer to populate the session list.
 */
USTRUCT(BlueprintType)
struct LOGFLOWCORE_API FLogFlowSessionInfo
{
	GENERATED_BODY()
	
	/** File name of the session log file. Format: LogFlow_YYYYMMDD_HHMMSS.txt */
	UPROPERTY()
	FString FileName;
	
	/** Absolute path to the session log file on disk. */
	UPROPERTY()
	FString FilePath;
	
	/** Date and time when session started. */
	UPROPERTY()
	FDateTime StartTime;
	
	/** Size of the session log file in bytes. */
	UPROPERTY()
	int64 FileSizeBytes;
	
	FLogFlowSessionInfo()
		: StartTime(FDateTime::MinValue())
		, FileSizeBytes(0){}
	
	FLogFlowSessionInfo(
		const FString& InFileName, 
		const FString& InFilePath,
		const FDateTime& InStartTime,
		int64 InFileSizeBytes)
			: FileName(InFileName)
			, FilePath(InFilePath)
			, StartTime(InStartTime)
			, FileSizeBytes(InFileSizeBytes){}
	
	/**
	 * Returns a human-readable string representing the session start time.
	 * Format: YYYY-MM-DD HH:MM:SS
	 */
	FString GetDisplayTime() const
	{
		return FString::Printf(TEXT("%04d-%02d-%02d %02d:%02d:%02d"),
			StartTime.GetYear(), StartTime.GetMonth(), StartTime.GetDay(),
			StartTime.GetHour(), StartTime.GetMinute(), StartTime.GetSecond());
	}
	
	/**
	 * Returns a human-readable string representing the file size.
	 * Format: X.X KB or X.X MB
	 */
	FString GetDisplaySize() const
	{
		if (FileSizeBytes < 1024)
		{
			return FString::Printf(TEXT("%lld B"), FileSizeBytes);
		}
		else if (FileSizeBytes < 1024 * 1024)
		{
			return FString::Printf(TEXT("%.1f KB"),
				static_cast<float>(FileSizeBytes) / 1024.0f);
		}
		return FString::Printf(TEXT("%.1f MB"),
			static_cast<float>(FileSizeBytes) / (1024.0f * 1024.0f));
	}
	
};