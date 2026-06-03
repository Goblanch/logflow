#include "CoreMinimal.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Misc/AutomationTest.h"
#include "LogFlowSubsystem.h"
#include "LogFlowBlueprintLibrary.h"
#include "LogFlowSettings.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Engine/Engine.h"

// -- Automation tests -------------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowIntegrationFullFlowTest,
	"LogFlow.Integration.FullFlow_AllSeverities_WrittenToSessionFile",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FLogFlowIntegrationFullFlowTest::RunTest(const FString& Parameters)
{
	const FString TestLogDir = TEXT("Saved/LogFlow_IntegrationTest/");
	const FString AbsDir = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectDir() / TestLogDir);
	
	// Clean up before test
	IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
	IFileManager::Get().MakeDirectory(*AbsDir, true);
	
	ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get();
	if (!TestNotNull(TEXT("Subsystem must be available."), Subsystem))
	{
		return false;
	}
	
	// Configure to use test directory
	FLogFlowSettings TestSettings = FLogFlowSettings::GetDefault();
	TestSettings.LogDirectory = TestLogDir;
	TestSettings.TimestampMode = ELogFlowTimestampMode::SessionTime;
	Subsystem->UpdateSettings(TestSettings);
	
	// Open session for test
	Subsystem->BeginLogSessionForTest();
	
	// Log three entries via the Blueprint Library (full stack test)
	ULogFlowBlueprintLibrary::LogMessage(
		TEXT("Integration test — log entry"),
		FName("IntegrationTest"));

	ULogFlowBlueprintLibrary::LogWarning(
		TEXT("Integration test — warning entry"),
		FName("IntegrationTest"));

	ULogFlowBlueprintLibrary::LogError(
		TEXT("Integration test — error entry"),
		FName("IntegrationTest"));
	
	// Wait for async file writer to flush.
	FPlatformProcess::Sleep(0.5f);
	
	// Close session before reading the file
	Subsystem->EndLogSessionForTest();
	FPlatformProcess::Sleep(0.1f);
	
	// Find the session file
	TArray<FString> Files;
	IFileManager::Get().FindFiles(Files, *(AbsDir / TEXT("LogFlow_*.txt")), true, false);
	
	if (!TestTrue(TEXT("Session file should have been generated."), Files.Num() > 0))
	{
		IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
		return false;
	}
	
	Files.Sort();
	const FString SessionFilePath = AbsDir / Files.Last();
	
	// Read the session file
	TArray<FString> Lines;
	if (!TestTrue(TEXT("Session file should be readable."),
		FFileHelper::LoadFileToStringArray(Lines, *SessionFilePath)))
	{
		IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
		return false;
	}
	
	// Filter only user entries by tag
	TArray<FString> UserLines;
	for (const FString& Line : Lines)
	{
		if (Line.Contains(TEXT("[IntegrationTest]")))
		{
			UserLines.Add(Line);
		}
	}
	
	TestEqual(TEXT("Session file should contain exactly 3 user entries"),
		UserLines.Num(), 3);
	
	if (UserLines.Num() >= 3)
	{
		// Verify severity fields
		TestTrue(TEXT("First entry should be LOG"),
			UserLines[0].Contains(TEXT("[LOG    ]")));
		TestTrue(TEXT("Second entry should be WARNING"),
			UserLines[1].Contains(TEXT("[WARNING]")));
		TestTrue(TEXT("Third entry should be ERROR"),
			UserLines[2].Contains(TEXT("[ERROR  ]")));

		// Verify message content
		TestTrue(TEXT("First entry message correct"),
			UserLines[0].Contains(TEXT("Integration test — log entry")));
		TestTrue(TEXT("Second entry message correct"),
			UserLines[1].Contains(TEXT("Integration test — warning entry")));
		TestTrue(TEXT("Third entry message correct"),
			UserLines[2].Contains(TEXT("Integration test — error entry")));

		// Verify timestamp format — line starts with [
		TestTrue(TEXT("First entry has timestamp"),
			UserLines[0].StartsWith(TEXT("[")));

		// Verify tag field present in all entries
		TestTrue(TEXT("First entry has correct tag"),
			UserLines[0].Contains(TEXT("[IntegrationTest]")));
		TestTrue(TEXT("Second entry has correct tag"),
			UserLines[1].Contains(TEXT("[IntegrationTest]")));
		TestTrue(TEXT("Third entry has correct tag"),
			UserLines[2].Contains(TEXT("[IntegrationTest]")));
	}
	
	// Clean up
	IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
	Subsystem->UpdateSettings(FLogFlowSettings::GetDefault());
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FLogFlowIntegrationNoTagTest,
    "LogFlow.Integration.LogMessage_WithNoTag_WritesHyphenInTagField",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowIntegrationNoTagTest::RunTest(const FString& Parameters)
{
    const FString TestLogDir = TEXT("Saved/LogFlow_IntegrationTest2/");
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

    // Open session for test
    Subsystem->BeginLogSessionForTest();

    // Log without tag
    ULogFlowBlueprintLibrary::LogMessage(
        TEXT("No tag message"),
        NAME_None);

    FPlatformProcess::Sleep(0.5f);

    // Close session before reading
    Subsystem->EndLogSessionForTest();
    FPlatformProcess::Sleep(0.1f);

    TArray<FString> Files;
    IFileManager::Get().FindFiles(
        Files, *(AbsDir / TEXT("LogFlow_*.txt")), true, false);

    if (!TestTrue(TEXT("Session file should exist"), Files.Num() > 0))
    {
        IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
        return false;
    }

    Files.Sort();
    TArray<FString> Lines;
    FFileHelper::LoadFileToStringArray(Lines, *(AbsDir / Files.Last()));

    // Find the no-tag line
    bool bFoundHyphen = false;
    for (const FString& Line : Lines)
    {
        if (Line.Contains(TEXT("No tag message")))
        {
            bFoundHyphen = Line.Contains(TEXT("[-]"));
            break;
        }
    }

    TestTrue(TEXT("Entry without tag should have [-] in tag field"), bFoundHyphen);

    IFileManager::Get().DeleteDirectory(*AbsDir, false, true);
    Subsystem->UpdateSettings(FLogFlowSettings::GetDefault());

    return true;
}