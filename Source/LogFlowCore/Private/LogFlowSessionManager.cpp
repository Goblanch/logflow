#include "LogFlowSessionManager.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

FLogFlowSessionManager::FLogFlowSessionManager(const FLogFlowSettings& InSettings)
	: Settings(InSettings)
{
	LoadIndex();
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
	
	const FString FileName = BuildFileName();
	const FString FilePath = GetAbsoluteLogDirectory() / FileName;
	
	ActiveSessionPath = FilePath;
	
	// Build session info for the index
	ActiveSessionInfo = FLogFlowSessionInfo(
		FileName,
		FilePath,
		FDateTime::Now(),
		0);
	
	return ActiveSessionPath;
}

void FLogFlowSessionManager::EndSession()
{
	if (!ActiveSessionPath.IsEmpty())
	{
		// Update file size now that the writer has closed the file
		UpdateActiveSessionSize();
		
		// Add to index and persist
		SessionIndex.Add(ActiveSessionInfo);
		SaveIndex();
		
		OnSessionIndexChanged.Broadcast();
	}
	
	ActiveSessionPath.Empty();
	ActiveSessionInfo = FLogFlowSessionInfo();
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
		const FString FullPath = FPaths::Combine(Dir, SessionFiles[i]);
		IFileManager::Get().Delete(*FullPath, false, true, true);
	}
	
	// Remove deleted sessions from the index.
	// Cast away const to modify index - RotateHistory is logically mutating.
	FLogFlowSessionManager* MutableThis = const_cast<FLogFlowSessionManager*>(this);
	MutableThis->SessionIndex.RemoveAll([&](const FLogFlowSessionInfo& Info)
	{
		return !IFileManager::Get().FileExists(*Info.FilePath);
	});
}

TArray<FLogFlowSessionInfo> FLogFlowSessionManager::GetSessionIndex() const
{
    // Return sorted copy — most recent first
    TArray<FLogFlowSessionInfo> Sorted = SessionIndex;
    Sorted.Sort([](const FLogFlowSessionInfo& A, const FLogFlowSessionInfo& B)
    {
        return A.StartTime > B.StartTime;
    });
    return Sorted;
}

FString FLogFlowSessionManager::GetIndexFilePath() const
{
    return GetAbsoluteLogDirectory() / TEXT("LogFlow_Index.json");
}

void FLogFlowSessionManager::LoadIndex()
{
    SessionIndex.Empty();

    const FString IndexPath = GetIndexFilePath();
    if (!IFileManager::Get().FileExists(*IndexPath))
    {
        return;
    }

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *IndexPath))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("LogFlow: Could not read session index at %s"), *IndexPath);
        return;
    }

    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("LogFlow: Session index is not valid JSON — rebuilding"));
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* SessionsArray;
    if (!RootObject->TryGetArrayField(TEXT("sessions"), SessionsArray))
    {
        return;
    }

    for (const TSharedPtr<FJsonValue>& Value : *SessionsArray)
    {
        const TSharedPtr<FJsonObject>* SessionObj;
        if (!Value->TryGetObject(SessionObj))
        {
            continue;
        }

        FLogFlowSessionInfo Info;
        (*SessionObj)->TryGetStringField(TEXT("fileName"), Info.FileName);
        (*SessionObj)->TryGetStringField(TEXT("filePath"), Info.FilePath);

        FString StartTimeStr;
        if ((*SessionObj)->TryGetStringField(TEXT("startTime"), StartTimeStr))
        {
            FDateTime::ParseIso8601(*StartTimeStr, Info.StartTime);
        }

        (*SessionObj)->TryGetNumberField(TEXT("fileSizeBytes"), Info.FileSizeBytes);

        // Only add if the file still exists on disk
        if (IFileManager::Get().FileExists(*Info.FilePath))
        {
            SessionIndex.Add(Info);
        }
    }
}

void FLogFlowSessionManager::SaveIndex() const
{
    if (!EnsureDirectoryExists())
    {
        return;
    }

    TArray<TSharedPtr<FJsonValue>> SessionsArray;

    for (const FLogFlowSessionInfo& Info : SessionIndex)
    {
        TSharedPtr<FJsonObject> SessionObj = MakeShared<FJsonObject>();
        SessionObj->SetStringField(TEXT("fileName"), Info.FileName);
        SessionObj->SetStringField(TEXT("filePath"), Info.FilePath);
        SessionObj->SetStringField(TEXT("startTime"),
            Info.StartTime.ToIso8601());
        SessionObj->SetNumberField(TEXT("fileSizeBytes"),
            static_cast<double>(Info.FileSizeBytes));

        SessionsArray.Add(MakeShared<FJsonValueObject>(SessionObj));
    }

    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
    RootObject->SetArrayField(TEXT("sessions"), SessionsArray);

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    FFileHelper::SaveStringToFile(JsonString, *GetIndexFilePath());
}

void FLogFlowSessionManager::UpdateActiveSessionSize()
{
    if (IFileManager::Get().FileExists(*ActiveSessionInfo.FilePath))
    {
        ActiveSessionInfo.FileSizeBytes =
            IFileManager::Get().FileSize(*ActiveSessionInfo.FilePath);
    }
}
