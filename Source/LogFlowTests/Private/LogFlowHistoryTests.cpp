#include "CoreMinimal.h"
#include "FLogFlowViewerLine.h"
#include "Misc/AutomationTest.h"
#include "LogFlowSubsystem.h"
#include "LogFlowSessionManager.h"
#include "LogFlowBlueprintLibrary.h"
#include "LogFlowSettings.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

// -- Helpers ----------------------------------------------------------------------------------------------------------

/**
 * Runs a complete PIE session simulation for testing:
 * opens a session, executes the provided log calls, waits for flush
 * and closes the session.
 * 
 * @param Subsystem The LogFlow subsystem instance.
 * @param LogCalls Lambda containing the log calls to execute.
 */
static void RunTestSession(ULogFlowSubsystem* Subsystem, TFunction<void()> LogCalls)
{
	Subsystem->BeginLogSessionForTest();
	LogCalls();
	FPlatformProcess::Sleep(0.3f);
	Subsystem->EndLogSessionForTest();
	FPlatformProcess::Sleep(0.1f);
}

// -- History index tests ----------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowHistoryIndexIntegrityTest,
	"LogFlow.Integration.History.IndexContainsCorrectNumberOfSessions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FLogFlowHistoryIndexIntegrityTest::RunTest(const FString& Parameters)
{
	const FString TestLogDir = TEXT("Saved/LogFlow_HistoryTest/");
	const FString AbsDir = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectDir() / TestLogDir);
	
	IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
	IFileManager::Get().MakeDirectory(*AbsDir, true);
	
	ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get();
	if (!TestNotNull(TEXT("Subsystem must be available"), Subsystem))
	{
		return false;
	}
	
	FLogFlowSettings TestSettings = FLogFlowSettings::GetDefault();
	TestSettings.LogDirectory = TestLogDir;
	TestSettings.MaxSessionHistory = 10;
	Subsystem->UpdateSettings(TestSettings);
	
	FLogFlowSessionManager* Manager = Subsystem->GetSessionManager();
	if (!TestNotNull(TEXT("Session manager must be available"), Manager))
	{
		return false;
	}
	
	// Run 3 sessions
	for (int32 i = 0; i < 3; ++i)
	{
		RunTestSession(Subsystem, [i]()
		{
			ULogFlowBlueprintLibrary::LogMessage(
				FString::Printf(TEXT("Session %d message"), i),
				FName("HistoryTest"));
		});
		
		// Small delay to ensure unique file names
		FPlatformProcess::Sleep(1.1f);
	}
	
	const TArray<FLogFlowSessionInfo> Index = Manager->GetSessionIndex();
	
	TestEqual(TEXT("Index should contain exactly 3 sessions"), Index.Num(), 3);
	
	if (Index.Num() >= 2)
	{
		TestTrue(TEXT("Sessions should be sorted most-recent-first"),
			Index[0].StartTime >= Index[1].StartTime);
	}
	
	for (const FLogFlowSessionInfo& Info : Index)
	{
		TestTrue(
			FString::Printf(TEXT("Session file should exists: %s"), *Info.FileName),
			IFileManager::Get().FileExists(*Info.FilePath));
		
		TestFalse(TEXT("FileName should not be empty"), Info.FileName.IsEmpty());
		TestFalse(TEXT("FilePath should not be empty"), Info.FilePath.IsEmpty());
		TestTrue(TEXT("StartTime should be valid"),
			Info.StartTime != FDateTime::MinValue());
	}
	
	IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
	Subsystem->UpdateSettings(FLogFlowSettings::GetDefault());
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowHistoryRotationTest,
	"LogFlow.Integration.History.RotationDeletesOldestSessions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FLogFlowHistoryRotationTest::RunTest(const FString& Parameters)
{
	const FString TestLogDir = TEXT("Saved/LogFlow_RotationTest/");
	const FString AbsDir = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectDir() / TestLogDir);
	
	IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
	IFileManager::Get().MakeDirectory(*AbsDir, true);
	
	ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get();
	if (!TestNotNull(TEXT("Subsystem must be available"), Subsystem))
	{
		return false;
	}
	
	FLogFlowSettings TestSettings = FLogFlowSettings::GetDefault();
	TestSettings.LogDirectory = TestLogDir;
	TestSettings.MaxSessionHistory = 2;
	Subsystem->UpdateSettings(TestSettings);
	
	FLogFlowSessionManager* Manager = Subsystem->GetSessionManager();
	if (!TestNotNull(TEXT("Session manager must be available"), Manager))
	{
		return false;
	}
	
	FString FirstSessionFileName;
	
	for (int32 i = 0; i < 3; ++i)
	{
		RunTestSession(Subsystem, [i]()
		{
			ULogFlowBlueprintLibrary::LogMessage(
				FString::Printf(TEXT("Rotation test session %d"), i),
				FName("RotationTest"));
		});
		
		if (i == 0)
		{
			const TArray<FLogFlowSessionInfo> IndexAfterFirst = Manager->GetSessionIndex();
			if (IndexAfterFirst.Num() > 0)
			{
				FirstSessionFileName = IndexAfterFirst.Last().FileName;
			}
		}
		FPlatformProcess::Sleep(1.1f);
	}
	
	const TArray<FLogFlowSessionInfo> FinalIndex = Manager->GetSessionIndex();
	
	TestEqual(TEXT("Index should contain exactly 2 session after rotation"),
		FinalIndex.Num(), 2);
	
	if (!FirstSessionFileName.IsEmpty())
	{
		const FString FirstFilePath = AbsDir / FirstSessionFileName;
		TestFalse(TEXT("First session file should have been deleted by rotation"),
			IFileManager::Get().FileExists(*FirstFilePath));
	}
	
	// Verify that the session is no longer in the index
	const bool bFirstSessionInIndex = FinalIndex.ContainsByPredicate(
		[&FirstSessionFileName](const FLogFlowSessionInfo& Info)
		{
			return Info.FileName == FirstSessionFileName;
		});
	
	TestFalse(TEXT("First session should not be in the index after rotation"), bFirstSessionInIndex);
	
	for (const FLogFlowSessionInfo& Info : FinalIndex)
	{
		TestTrue(
			FString::Printf(TEXT("Remaining session file should exist: %s"), *Info.FileName),
			IFileManager::Get().FileExists(*Info.FilePath));
	}
	
	IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
	Subsystem->UpdateSettings(FLogFlowSettings::GetDefault());
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowHistoryIndexPersistenceTest,
	"LogFlow.Integration.History.IndexPersistsAfterSettingsUpdate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FLogFlowHistoryIndexPersistenceTest::RunTest(const FString& Parameters)
{
	const FString TestLogDir = TEXT("Saved/LogFlow_RotationTest/");
	const FString AbsDir = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectDir() / TestLogDir);
	
	IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
	IFileManager::Get().MakeDirectory(*AbsDir, true);
	
	ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get();
	if (!TestNotNull(TEXT("Subsystem must be available"), Subsystem))
	{
		return false;
	}
	
	FLogFlowSettings TestSettings = FLogFlowSettings::GetDefault();
	TestSettings.LogDirectory = TestLogDir;
	Subsystem->UpdateSettings(TestSettings);
	
	FLogFlowSessionManager* Manager = Subsystem->GetSessionManager();
	if (!TestNotNull(TEXT("Session manager must be available"), Manager))
	{
		return false;
	}
	
	RunTestSession(Subsystem, []()
	{
		ULogFlowBlueprintLibrary::LogMessage(
			TEXT("Persistence test message"),
			FName("PersistenceTest"));
	});
	
	const FString IndexPath = AbsDir / TEXT("LogFlow_Index.json");
	TestTrue(TEXT("Index JSON file should exist on disk"),
		IFileManager::Get().FileExists(*IndexPath));
	
	FString JsonContent;
	TestTrue(TEXT("Index JSON should be readable"), 
		FFileHelper::LoadFileToString(JsonContent, *IndexPath));
	
	TestTrue(TEXT("Index JSON should contain session array"),
		JsonContent.Contains(TEXT("sessions")));
	
	TestTrue(TEXT("Index JSON should contain filename field"),
		JsonContent.Contains(TEXT("fileName")));
	
	TestTrue(TEXT("Index JSON should contain startTime field"),
		JsonContent.Contains(TEXT("startTime")));
	
	TestTrue(TEXT("Index JSON should contain fileSizeBytes field"),
		JsonContent.Contains(TEXT("fileSizeBytes")));
	
	IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
	Subsystem->UpdateSettings(FLogFlowSettings::GetDefault());
	
	return true;
}

// -- Log Viewer content tests -----------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowViewerContentMatchesFileTest,
	"LogFlow.Integration.LogViewer.ContentMatchesSessionFile",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FLogFlowViewerContentMatchesFileTest::RunTest(const FString& Parameters)
{
	const FString TestLogDir = TEXT("Saved/LogFlow_ViewerTest/");
	const FString AbsDir = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectDir() / TestLogDir);

	IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
	IFileManager::Get().MakeDirectory(*AbsDir, true);

	ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get();
	if (!TestNotNull(TEXT("Subsystem must be available"), Subsystem))
	{
		return false;
	}

	FLogFlowSettings TestSettings = FLogFlowSettings::GetDefault();
	TestSettings.LogDirectory = TestLogDir;
	Subsystem->UpdateSettings(TestSettings);
	
	
	RunTestSession(Subsystem, []()
	{
		ULogFlowBlueprintLibrary::LogMessage(
			TEXT("Viewer test log"),
			FName("ViewerTest"));
		ULogFlowBlueprintLibrary::LogWarning(
			TEXT("Viewer test warning"),
			FName("ViewerTest"));
		ULogFlowBlueprintLibrary::LogError(
			TEXT("Viewer test error"),
			FName("ViewerTest"));
	});
	
	TArray<FString> Files;
	IFileManager::Get().FindFiles(
		Files, *(AbsDir / TEXT("LogFlow_*.txt")), true, false);
	
	if (!TestTrue(TEXT("Session file should exist"), Files.Num() > 0))
	{
		IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
		return false;
	}
	
	Files.Sort();
	const FString SessionFilePath = AbsDir / Files.Last();
	
	TArray<FString> FileLines;
	if (!TestTrue(TEXT("Session file should be readable"),
		FFileHelper::LoadFileToStringArray(FileLines, *SessionFilePath)))
	{
		IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
		return false;
	}
	
	// Simulate what SLogFlowViewer::LoadSessionContent does —
	// parse lines and verify severity detection
	TArray<FLogFlowViewerLine> ParsedLines;
	for (const FString& Line : FileLines)
	{
		if (!Line.IsEmpty())
		{
			ParsedLines.Add(FLogFlowViewerLine(
				Line,
				FLogFlowViewerLine::ParseSeverity(Line)));
		}
	}

	TestTrue(TEXT("Parsed lines should not be empty"), ParsedLines.Num() > 0);

	// Find and verify the three test entries
	int32 LogCount    = 0;
	int32 WarnCount   = 0;
	int32 ErrorCount  = 0;

	for (const FLogFlowViewerLine& ParsedLine : ParsedLines)
	{
		if (!ParsedLine.RawText.Contains(TEXT("[ViewerTest]")))
		{
			continue;
		}

		if (ParsedLine.RawText.Contains(TEXT("Viewer test log")))
		{
			TestEqual(TEXT("Log line severity should be Log"),
				ParsedLine.Severity, ELogFlowSeverity::Log);
			++LogCount;
		}
		else if (ParsedLine.RawText.Contains(TEXT("Viewer test warning")))
		{
			TestEqual(TEXT("Warning line severity should be Warning"),
				ParsedLine.Severity, ELogFlowSeverity::Warning);
			++WarnCount;
		}
		else if (ParsedLine.RawText.Contains(TEXT("Viewer test error")))
		{
			TestEqual(TEXT("Error line severity should be Error"),
				ParsedLine.Severity, ELogFlowSeverity::Error);
			++ErrorCount;
		}
	}

	TestEqual(TEXT("Should find exactly 1 log entry"),   LogCount,   1);
	TestEqual(TEXT("Should find exactly 1 warning entry"), WarnCount, 1);
	TestEqual(TEXT("Should find exactly 1 error entry"),  ErrorCount, 1);

	IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
	Subsystem->UpdateSettings(FLogFlowSettings::GetDefault());

	return true;
}