#include "LogFlowFileWriter.h"

FLogFlowFileWriter::FLogFlowFileWriter(const FLogFlowSettings& InSettings)
	: Settings(InSettings)
	, FileHandle(nullptr)
	, Thread(nullptr)
	, bStopRequested(false)
	, bSessionOpen(false)
{
	Thread = FRunnableThread::Create(this, TEXT("LogFlowFileWriter"));
}

FLogFlowFileWriter::~FLogFlowFileWriter()
{
	CloseSession();
}

// -- FRunnable --------------------------------------------------------------------------------------

bool FLogFlowFileWriter::Init()
{
	return true;
}

uint32 FLogFlowFileWriter::Run()
{
	while (!bStopRequested)
	{
		FLogFlowEntry Entry;
		if (EntryQueue.Dequeue(Entry))
		{
			WriteEntry(Entry);
		}else
		{
			// No entries available - yield to avoid spinning at 100% CPU.
			FPlatformProcess::Sleep(0.001f);
		}
	}
	
	// Darin any remaining entries before exiting
	FlushPendingEntries();
	
	return 0;
}

void FLogFlowFileWriter::Stop()
{
	bStopRequested = true;
}

void FLogFlowFileWriter::Exit()
{
	if (FileHandle != nullptr)
	{
		FileHandle->Flush();
		delete FileHandle;
		FileHandle = nullptr;
	}
}

// -- ILogFlowConsumer --------------------------------------------------------------------------------------

void FLogFlowFileWriter::OnLogFlowEntryReceived(const FLogFlowEntry& Entry)
{
	if (bSessionOpen)
	{
		EntryQueue.Enqueue(Entry);
	}
}

// -- Session Management -------------------------------------------------------------------------------------

bool FLogFlowFileWriter::OpenSession(const FString& FilePath)
{
	if (bSessionOpen)
	{
		CloseSession();
	}

	FileHandle = IFileManager::Get().CreateFileWriter(*FilePath, FILEWRITE_AllowRead);
	if (FileHandle == nullptr)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("LogFlow: Failed to open session file at %s"), *FilePath);
		return false;
	}

	bSessionOpen = true;
	bStopRequested = false;
	return true;
}

void FLogFlowFileWriter::CloseSession()
{
	if (!bSessionOpen) return;
	
	// Signal the thread to stop and wait for it to exit cleanly.
	Stop();
	if (Thread != nullptr) Thread->WaitForCompletion();
	
	bSessionOpen = false;
}

bool FLogFlowFileWriter::IsSessionOpen() const
{
	return bSessionOpen;
}

void FLogFlowFileWriter::UpdateSettings(const FLogFlowSettings& InSettings)
{
	Settings = InSettings;
}

// -- Private --------------------------------------------------------------------------------------

void FLogFlowFileWriter::WriteEntry(const FLogFlowEntry& Entry)
{
	if (FileHandle == nullptr)
	{
		return;
	}

	const FString TagString = Entry.Tag.IsNone()
		? TEXT("-")
		: Entry.Tag.ToString();

	const FString Line = FString::Printf(
		TEXT("[%s] [%s] [%s] %s\n"),
		*FormatTimestamp(Entry),
		*FormatSeverity(Entry.Severity),
		*TagString,
		*Entry.Message
	);

	const FTCHARToUTF8 Utf8(*Line);
	FileHandle->Serialize(
		const_cast<char*>(Utf8.Get()),
		Utf8.Length()
	);
}

FString FLogFlowFileWriter::FormatTimestamp(const FLogFlowEntry& Entry) const
{
	if (Settings.TimestampMode == ELogFlowTimestampMode::SystemTime)
	{
		return FString::Printf(
			TEXT("%02d:%02d:%02d.%03d"),
			Entry.SystemTime.GetHour(),
			Entry.SystemTime.GetMinute(),
			Entry.SystemTime.GetSecond(),
			Entry.SystemTime.GetMillisecond()
		);
	}

	// Session time mode
	const int32 TotalSeconds = static_cast<int32>(Entry.Timestamp.GetTotalSeconds());
	const int32 Hours        = TotalSeconds / 3600;
	const int32 Minutes      = (TotalSeconds % 3600) / 60;
	const int32 Seconds      = TotalSeconds % 60;
	const int32 Milliseconds = Entry.Timestamp.GetFractionMilli();

	return FString::Printf(
		TEXT("%02d:%02d:%02d.%03d"),
		Hours, Minutes, Seconds, Milliseconds
	);
}

FString FLogFlowFileWriter::FormatSeverity(ELogFlowSeverity Severity)
{
	switch (Severity)
	{
		case ELogFlowSeverity::Log:     return TEXT("LOG    ");
		case ELogFlowSeverity::Warning: return TEXT("WARNING");
		case ELogFlowSeverity::Error:   return TEXT("ERROR  ");
		default:                        return TEXT("LOG    ");
	}
}

void FLogFlowFileWriter::FlushPendingEntries()
{
	FLogFlowEntry Entry;
	while (EntryQueue.Dequeue(Entry))
	{
		WriteEntry(Entry);
	}
}
