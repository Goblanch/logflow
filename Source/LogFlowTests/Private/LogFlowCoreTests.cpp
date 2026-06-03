#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "LogFlowEntry.h"
#include "LogFlowSeverity.h"
#include "LogFlowDispatcher.h"
#include "LogFlowSessionManager.h"
#include "LogFlowSettings.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

// -- Helpers ---------------------------------------------------------

/**
 * Minimal ILogFlowConsumer implementation used in dispatchyer tests.
 * Records every entry received for later assertions.
 */
class FTestConsumer : public ILogFlowConsumer
{
public:
	TArray<FLogFlowEntry> ReceivedEntries;
	
	virtual void OnLogFlowEntryReceived(const FLogFlowEntry& Entry) override
	{
		ReceivedEntries.Add(Entry);
	}
};

// -- FLogFlowEntry tests ----------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowEntryCreateTest,
	"LogFlow.Core.FLogFlowEntry.Create_InitializesAllFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowEntryCreateTest::RunTest(const FString& Parameters)
{
	const FString    Message  = TEXT("Test message");
	const ELogFlowSeverity Severity = ELogFlowSeverity::Warning;
	const FName      Tag      = FName("TestTag");
	const FTimespan  Timestamp = FTimespan::FromSeconds(5.0);

	const FLogFlowEntry Entry = FLogFlowEntry::Create(Message, Severity, Tag, Timestamp);

	TestEqual(TEXT("Message should match"),   Entry.Message,   Message);
	TestEqual(TEXT("Severity should match"),  Entry.Severity,  Severity);
	TestEqual(TEXT("Tag should match"),       Entry.Tag,       Tag);
	TestEqual(TEXT("Timestamp should match"), Entry.Timestamp, Timestamp);
	TestTrue(TEXT("SystemTime should be set"), Entry.SystemTime != FDateTime());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowEntryCreateDefaultTagTest,
	"LogFlow.Core.FLogFlowEntry.Create_WithNoTag_TagIsNone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowEntryCreateDefaultTagTest::RunTest(const FString& Parameters)
{
	const FLogFlowEntry Entry = FLogFlowEntry::Create(
		TEXT("No tag message"),
		ELogFlowSeverity::Log,
		NAME_None,
		FTimespan::Zero());
	
	TestTrue(TEXT("Tag should be NAME_None"), Entry.Tag.IsNone());
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowEntryAllSeveritiesTest,
	"LogFlow.Core.FLogFlowEntry.Create_AllSeverityLevels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowEntryAllSeveritiesTest::RunTest(const FString& Parameters)
{
	const FLogFlowEntry LogEntry = FLogFlowEntry::Create(
		TEXT("Log"), ELogFlowSeverity::Log, NAME_None, FTimespan::Zero());
	const FLogFlowEntry WarningEntry = FLogFlowEntry::Create(
		TEXT("Warning"), ELogFlowSeverity::Warning, NAME_None, FTimespan::Zero());
	const FLogFlowEntry ErrorEntry = FLogFlowEntry::Create(
		TEXT("Error"), ELogFlowSeverity::Error, NAME_None, FTimespan::Zero());

	TestEqual(TEXT("Log severity"),     LogEntry.Severity,     ELogFlowSeverity::Log);
	TestEqual(TEXT("Warning severity"), WarningEntry.Severity, ELogFlowSeverity::Warning);
	TestEqual(TEXT("Error severity"),   ErrorEntry.Severity,   ELogFlowSeverity::Error);

	return true;
}

// -- FLogFlowDispatcher tests -------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowDispatcherRegisterTest,
	"LogFlow.Core.FLogFlowDispatcher.RegisterConsumer_AddsConsumer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowDispatcherRegisterTest::RunTest(const FString& Parameters)
{
	FLogFlowDispatcher Dispatcher;
	FTestConsumer Consumer;
	
	TestEqual(TEXT("No consumers initialy"), Dispatcher.GetConsumerCount(), 0);
	
	Dispatcher.RegisterConsumer(&Consumer);
	TestEqual(TEXT("One consumer after register"), Dispatcher.GetConsumerCount(), 1);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowDispatcherRegisterDuplicateTest,
	"LogFlow.Core.FLogFlowDispatcher.RegisterConsumer_DoesNotAddDuplicate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowDispatcherRegisterDuplicateTest::RunTest(const FString& Parameters)
{
	FLogFlowDispatcher Dispatcher;
	FTestConsumer Consumer;
	
	Dispatcher.RegisterConsumer(&Consumer);
	Dispatcher.RegisterConsumer(&Consumer);
	
	TestEqual(TEXT("Duplicate should not be added"), Dispatcher.GetConsumerCount(), 1);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowDispatcherRegisterNullTest,
	"LogFlow.Core.FLogFlowDispatcher.RegisterConsumer_IgnoresNullPointer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowDispatcherRegisterNullTest::RunTest(const FString& Parameters)
{
	FLogFlowDispatcher Dispatcher;
	
	Dispatcher.RegisterConsumer(nullptr);
	TestEqual(TEXT("Null consumer should be ignored"), Dispatcher.GetConsumerCount(), 0);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowDispatcherUnregisterTest,
	"LogFlow.Core.FLogFlowDispatcher.UnregisterConsumer_RemovesConsumer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowDispatcherUnregisterTest::RunTest(const FString& Parameters)
{
	FLogFlowDispatcher Dispatcher;
	FTestConsumer Consumer;
	
	Dispatcher.RegisterConsumer(&Consumer);
	Dispatcher.UnregisterConsumer(&Consumer);
	
	TestEqual(TEXT("Consumer should be removed"), Dispatcher.GetConsumerCount(), 0);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowDispatcherUnregisterNonExistentTest,
	"LogFlow.Core.FLogFlowDispatcher.UnregisterConsumer_IgnoresNonExistent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowDispatcherUnregisterNonExistentTest::RunTest(const FString& Parameters)
{
	FLogFlowDispatcher Dispatcher;
	FTestConsumer Consumer;
	
	// Should not crash or produce error
	Dispatcher.UnregisterConsumer(&Consumer);
	TestEqual(TEXT("Count remains zero"), Dispatcher.GetConsumerCount(), 0);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowDispatcherDispatchTest,
	"LogFlow.Core.FLogFlowDispatcher.Dispatch_DeliversEntryToConsumer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowDispatcherDispatchTest::RunTest(const FString& Parameters)
{
	FLogFlowDispatcher Dispatcher;
	FTestConsumer Consumer;
	
	Dispatcher.RegisterConsumer(&Consumer);
	
	const FLogFlowEntry Entry = FLogFlowEntry::Create(
		TEXT("Dispatch test"),
		ELogFlowSeverity::Log,
		FName("Test"),
		FTimespan::Zero());
	
	Dispatcher.Dispatch(Entry);
	Dispatcher.NotifyAll();
	
	TestEqual(TEXT("Consumer should have received one entry"), Consumer.ReceivedEntries.Num(), 1);
	TestEqual(TEXT("Received message should match"), 
		Consumer.ReceivedEntries[0].Message, FString(TEXT("Dispatch test")));
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowDispatchNoConsumersTest,
	"LogFlow.Core.FLogFlowDispatcher.Dispatch_WithNoConsumers_ProducesNoError",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowDispatchNoConsumersTest::RunTest(const FString& Parameters)
{
	FLogFlowDispatcher Dispatcher;
	
	const  FLogFlowEntry Entry = FLogFlowEntry::Create(
		TEXT("No consumers"),
		ELogFlowSeverity::Log,
		NAME_None,
		FTimespan::Zero());
	
	// Should not crash
	Dispatcher.Dispatch(Entry);
	Dispatcher.NotifyAll();
	
	TestEqual(TEXT("No consumers registered"), Dispatcher.GetConsumerCount(), 0);
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowDispatcherMultipleConsumersTest,
	"LogFlow.Core.FLogFlowDispatcher.Dispatch_DeliversToAllConsumers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowDispatcherMultipleConsumersTest::RunTest(const FString& Parameters)
{
	FLogFlowDispatcher Dispatcher;
	FTestConsumer ConsumerA;
	FTestConsumer ConsumerB;
	
	Dispatcher.RegisterConsumer(&ConsumerA);
	Dispatcher.RegisterConsumer(&ConsumerB);
	
	const FLogFlowEntry Entry = FLogFlowEntry::Create(
		TEXT("Multi consumer"),
		ELogFlowSeverity::Error,
		NAME_None,
		FTimespan::Zero());
	
	Dispatcher.Dispatch(Entry);
	Dispatcher.NotifyAll();
	
	TestEqual(TEXT("ConsumerA received entry"), ConsumerA.ReceivedEntries.Num(), 1);
	TestEqual(TEXT("ConsumerB received entry"), ConsumerB.ReceivedEntries.Num(), 1);
	
	return true;
}

// -- FLogFlowSessionManager tests ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowSessionManagerBuildFileNameTest,
	"LogFlow.Core.FLogFlowSessionManager.BuildFileName_HasCorrectFormat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowSessionManagerBuildFileNameTest::RunTest(const FString& Parameters)
{
	FLogFlowSettings Settings = FLogFlowSettings::GetDefault();
	FLogFlowSessionManager Manager(Settings);
	
	const FString FileName = Manager.BuildFileName();
	
	// Must start with LogFlow_
	TestTrue(TEXT("File name starts with LogFlow_"), 
		FileName.StartsWith(TEXT("LogFlow_")));
	
	// Must end with .txt
	TestTrue(TEXT("File name ends with .txt"),
		FileName.EndsWith(TEXT(".txt")));
	
	// Must be exactly LogFlow_YYYYMMDD_HHMMSS.txt = 27 characters
	TestEqual(TEXT("File name has correct length"), FileName.Len(), 27);
	
	// Underscore at position 16 separating date and time
	TestEqual(TEXT("Separator underscore at correct position"), FileName[16], TCHAR('_'));
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowSessionManagerBuildFileNameUniqueTest,
	"LogFlow.Core.FLogFlowSessionManager.BuildFileName_ReflectsCurrentTime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowSessionManagerBuildFileNameUniqueTest::RunTest(const FString& Parameters)
{
	FLogFlowSettings Settings = FLogFlowSettings::GetDefault();
	FLogFlowSessionManager Manager(Settings);

	// Record time before and after generating the file name
	const FDateTime Before = FDateTime::Now();
	const FString FileName = Manager.BuildFileName();
	const FDateTime After  = FDateTime::Now();

	// Extract the date portion from the file name: LogFlow_YYYYMMDD_HHMMSS.txt
	// Date is at positions 8-15: YYYYMMDD
	const FString DateInName = FileName.Mid(8, 8);
	const FString ExpectedDate = FString::Printf(TEXT("%04d%02d%02d"),
		Before.GetYear(), Before.GetMonth(), Before.GetDay());

	TestEqual(TEXT("Date in file name should match current date"),
		DateInName, ExpectedDate);

	// Extract hour from file name: position 16-17
	const FString HourInName = FileName.Mid(16, 2);
	const int32 HourValue = FCString::Atoi(*HourInName);

	TestTrue(TEXT("Hour in file name should be valid (0-23)"),
		HourValue >= 0 && HourValue <= 23);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FLogFlowSessionManagerRotateHistoryWithinLimitTest,
    "LogFlow.Core.FLogFlowSessionManager.RotateHistory_DoesNotDeleteWhenWithinLimit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowSessionManagerRotateHistoryWithinLimitTest::RunTest(const FString& Parameters)
{
    const FString TestDir = FPaths::ConvertRelativePathToFull(
        FPaths::ProjectDir() / TEXT("Saved/LogFlow_Test2/"));

    IFileManager::Get().MakeDirectory(*TestDir, true);

    FLogFlowSettings Settings = FLogFlowSettings::GetDefault();
    Settings.LogDirectory      = TEXT("Saved/LogFlow_Test2/");
    Settings.MaxSessionHistory = 5;

    FLogFlowSessionManager Manager(Settings);

    const TArray<FString> DummyFiles = {
        TEXT("LogFlow_20260101_100000.txt"),
        TEXT("LogFlow_20260101_110000.txt"),
        TEXT("LogFlow_20260101_120000.txt")
    };

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    for (const FString& FileName : DummyFiles)
    {
        const FString FilePath = TestDir / FileName;
        IFileHandle* Handle = PlatformFile.OpenWrite(*FilePath);
        if (Handle != nullptr)
        {
            delete Handle;
        }
    }

    Manager.RotateHistory();

    TArray<FString> FilesAfterRotation;
    IFileManager::Get().FindFiles(
        FilesAfterRotation, *(TestDir / TEXT("LogFlow_*.txt")), true, false);

    TestEqual(TEXT("No files should be deleted when within limit"),
        FilesAfterRotation.Num(), 3);

    IFileManager::Get().DeleteDirectory(*TestDir, false, true);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLogFlowSessionManagerBeginEndSessionTest,
	"LogFlow.Core.FLogFlowSessionManager.BeginEndSession_LifecycleIsCorrect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLogFlowSessionManagerBeginEndSessionTest::RunTest(const FString& Parameters)
{
	const FString TestDir = FPaths::ProjectSavedDir() / TEXT("LogFlow_Test3/");

	FLogFlowSettings Settings = FLogFlowSettings::GetDefault();
	Settings.LogDirectory = TEXT("Saved/LogFlow_Test3/");

	FLogFlowSessionManager Manager(Settings);

	TestFalse(TEXT("No session active initially"), Manager.IsSessionActive());

	const FString SessionPath = Manager.BeginSession();

	TestTrue(TEXT("Session should be active after BeginSession"),
		Manager.IsSessionActive());
	TestFalse(TEXT("Session path should not be empty"),
		SessionPath.IsEmpty());
	TestTrue(TEXT("Session path should end with .txt"),
		SessionPath.EndsWith(TEXT(".txt")));

	Manager.EndSession();

	TestFalse(TEXT("No session active after EndSession"),
		Manager.IsSessionActive());
	TestTrue(TEXT("Active session path should be empty after EndSession"),
		Manager.GetActiveSessionPath().IsEmpty());

	IFileManager::Get().DeleteDirectory(*TestDir, false, true);

	return true;
}
