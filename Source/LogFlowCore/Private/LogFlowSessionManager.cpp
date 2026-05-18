#include "LogFlowSessionManager.h"

FLogFlowSessionManager::FLogFlowSessionManager(const FLogFlowSettings& InSettings)
{
	
}

FLogFlowSessionManager::~FLogFlowSessionManager()
{
	EndSession();
}

FString FLogFlowSessionManager::BeginSession()
{
	if (IsSessionActive()) EndSession();
	
	if (!EnsureDirectoryExists())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("LogFlow: Could not create log directory at %s"),
			*GetAbsoluteLogDirectory());
		return FString();
	}
	
	RotateHistory();
	
	ActiveSessionPath = GetAbsoluteLogDirectory() / BuildFileName();
	return ActiveSessionPath;
}

void FLogFlowSessionManager::EndSession()
{
	ActiveSessionPath.Empty();
}

bool FLogFlowSessionManager::IsSessionActive() const
{
	return !ActiveSessionPath.IsEmpty();
}

FString FLogFlowSessionManager::GetActiveSessionPath() const
{
	return ActiveSessionPath;
}

void FLogFlowSessionManager::UpdateSettings(const FLogFlowSettings& InSettings)
{
	Settings = InSettings;
}

FString FLogFlowSessionManager::BuildFileName() const
{
	const FDateTime Now = FDateTime::Now();
	return FString::Printf(
		TEXT("LogFlow_%04d%02d%02d_%02d%02d%02d.txt"),
		Now.GetYear(),
		Now.GetMonth(),
		Now.GetDay(),
		Now.GetHour(),
		Now.GetMinute(),
		Now.GetSecond()
	);
}

FString FLogFlowSessionManager::GetAbsoluteLogDirectory() const
{
	return FPaths::ConvertRelativePathToFull(
		FPaths::ProjectDir() / Settings.LogDirectory
	);
}

bool FLogFlowSessionManager::EnsureDirectoryExists() const
{
	const FString Dir = GetAbsoluteLogDirectory();
	if (!IFileManager::Get().DirectoryExists(*Dir))
	{
		return IFileManager::Get().MakeDirectory(*Dir, true);
	}
	return true;
}

void FLogFlowSessionManager::RotateHistory() const
{
	const FString Dir = GetAbsoluteLogDirectory();
	
	// Find all LogFlow sessions files in the directory.
	TArray<FString> SessionFiles;
	IFileManager::Get().FindFiles(SessionFiles, *(Dir / TEXT("LogFlow_*.txt")), true, false);
	
	if (SessionFiles.Num() < Settings.MaxSessionHistory) return;
	
	// Sort ascending by name. Oldest first.
	SessionFiles.Sort();
	
	// Delete oldest files until we are one below the limit.
	const int32 FilesToDelete = SessionFiles.Num() - Settings.MaxSessionHistory + 1;
	for (int32 i = 0; i < FilesToDelete; i++)
	{
		const FString FullPath = Dir / SessionFiles[i];
		IFileManager::Get().Delete(*FullPath, false, true);
	}
}
