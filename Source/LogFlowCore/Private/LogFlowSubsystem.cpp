#include "LogFlowSubsystem.h"
#include "LogFlowDispatcher.h"
#include "LogFlowSessionManager.h"
#include "LogFlowFileWriter.h"
#include "Engine/Engine.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

void ULogFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	Settings = FLogFlowSettings::GetDefault();
	
	Dispatcher		= MakeUnique<FLogFlowDispatcher>();
	SessionManager	= MakeUnique<FLogFlowSessionManager>(Settings);
	FileWriter		= MakeUnique<FLogFlowFileWriter>(Settings);
	
	// Registers the file writer as the first consumer.
	Dispatcher->RegisterConsumer(FileWriter.Get());
	
#if WITH_EDITOR
	// Subscribe to PIE events so sessions open and close automatically.
	BeginPIEHandle = FEditorDelegates::BeginPIE.AddUObject(
		this, &ULogFlowSubsystem::OnBeginPIE);
	EndPIEHandle = FEditorDelegates::EndPIE.AddUObject(
		this, &ULogFlowSubsystem::OnEndPIE);
#endif
}

void ULogFlowSubsystem::Deinitialize()
{
	EndLogSession();
	
#if WITH_EDITOR
	FEditorDelegates::BeginPIE.Remove(BeginPIEHandle);
	FEditorDelegates::EndPIE.Remove(EndPIEHandle);
#endif
	
	if (Dispatcher && FileWriter)
	{
		Dispatcher->UnregisterConsumer(FileWriter.Get());
	}
	
	FileWriter.Reset();
	SessionManager.Reset();
	Dispatcher.Reset();
	
	Super::Deinitialize();
}

void ULogFlowSubsystem::LogMessage(const FString& Message, ELogFlowSeverity Severity, FName Tag)
{
	ULogFlowSubsystem* Instance = Get();
	if (Instance == nullptr || Instance->Dispatcher == nullptr) return;
	
	// Compute session-relative timestamp
	const FDateTime Now = FDateTime::Now();
	const FTimespan SessionTime = Now - Instance->PIESessionStartTime;
	
	const FLogFlowEntry Entry = FLogFlowEntry::Create(
		Message,
		Severity,
		Tag,
		SessionTime);
	
#if WITH_EDITOR
	// Break on error if configured
	if (Severity == ELogFlowSeverity::Error && Instance->Settings.bBreakOnError)
	{
		if (GEditor && GEditor->PlayWorld)
		{
			GEditor->PlayWorld->bDebugPauseExecution = true;
		}
	}
#endif
	
	Instance->Dispatcher->Dispatch(Entry);
	Instance->Dispatcher->NotifyAll();
}

const FLogFlowSettings& ULogFlowSubsystem::GetSettings() const
{
	return Settings;
}

void ULogFlowSubsystem::UpdateSettings(const FLogFlowSettings& NewSettings)
{
	Settings = NewSettings;
	
	if (SessionManager)
	{
		SessionManager->UpdateSettings(NewSettings);
	}
	
	if (FileWriter)
	{
		FileWriter->UpdateSettings(NewSettings);
	}
}

ULogFlowSubsystem* ULogFlowSubsystem::Get()
{
	if (GEngine == nullptr) return nullptr;
	
	return GEngine->GetEngineSubsystem<ULogFlowSubsystem>();
}

FLogFlowDispatcher* ULogFlowSubsystem::GetDispatcher() const
{
	return Dispatcher.Get();
}

void ULogFlowSubsystem::OnBeginPIE(bool bIsSimulating)
{
	PIESessionStartTime = FDateTime::Now();
	BeginLogSession();
	
	if (Settings.bAutoClear)
	{
		// Broadcast a notification for the panel to clear itself.
		// SLogFlowPanel will subscribe to this in the editor module.
		// Implementation handled in LogFlowEditor — no dependency here.
	}
}

void ULogFlowSubsystem::OnEndPIE(bool bIsSimulating)
{
	EndLogSession();
}

void ULogFlowSubsystem::BeginLogSession()
{
	if (SessionManager && FileWriter)
	{
		const FString SessionPath = SessionManager->BeginSession();
		if (!SessionPath.IsEmpty())
		{
			FileWriter->OpenSession(SessionPath);
		}
	}
}

void ULogFlowSubsystem::EndLogSession()
{
	if (FileWriter && FileWriter->IsSessionOpen())
	{
		FileWriter->CloseSession();
	}
	
	if (SessionManager && SessionManager->IsSessionActive())
	{
		SessionManager->EndSession();
	}
}

