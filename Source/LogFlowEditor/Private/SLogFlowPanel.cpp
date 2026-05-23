#include "SLogFlowPanel.h"
#include "LogFlowSubsystem.h"
#include "LogFlowDispatcher.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "SlateOptMacros.h"
#include "Engine/Engine.h"
#include "LogFlowSeverity.h"

#define LOCTEXT_NAMESPACE "SLogFlowPanel"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogFlowPanel::Construct(const FArguments& InArgs)
{
	Settings            = FLogFlowSettings::GetDefault();
    bNewEntriesAdded    = false;
    bShowLog            = true;
    bShowWarning        = true;
    bShowError          = true;
    LogCount            = 0;
    WarningCount        = 0;
    ErrorCount          = 0;

    ChildSlot
    [
        SNew(SVerticalBox)

        // ── Header ────────────────────────────────────────────────────────
+ SVerticalBox::Slot()
.AutoHeight()
.Padding(FMargin(4.0f, 4.0f))
[
    SNew(SHorizontalBox)

    // Title
    + SHorizontalBox::Slot()
    .VAlign(VAlign_Center)
    .FillWidth(1.0f)
    [
        SNew(STextBlock)
        .Text(LOCTEXT("PanelTitle", "LogFlow"))
        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
    ]

    // Log toggle
    + SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(FMargin(2.0f, 0.0f))
    [
        SNew(SCheckBox)
        .Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
        .IsChecked_Lambda([this]()
        {
            return bShowLog ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        })
        .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
        {
            bShowLog = (NewState == ECheckBoxState::Checked);
            ApplyFilters();
        })
        .ForegroundColor(TAttribute<FSlateColor>::Create(
            TAttribute<FSlateColor>::FGetter::CreateSP(
                this, &SLogFlowPanel::GetSeverityButtonColor,
                ELogFlowSeverity::Log)))
        [
            SNew(STextBlock)
            .Text(TAttribute<FText>::Create(
                TAttribute<FText>::FGetter::CreateSP(
                    this, &SLogFlowPanel::GetSeverityButtonText,
                    ELogFlowSeverity::Log)))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
        ]
    ]

    // Warning toggle
    + SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(FMargin(2.0f, 0.0f))
    [
        SNew(SCheckBox)
        .Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
        .IsChecked_Lambda([this]()
        {
            return bShowWarning ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        })
        .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
        {
            bShowWarning = (NewState == ECheckBoxState::Checked);
            ApplyFilters();
        })
        .ForegroundColor(TAttribute<FSlateColor>::Create(
            TAttribute<FSlateColor>::FGetter::CreateSP(
                this, &SLogFlowPanel::GetSeverityButtonColor,
                ELogFlowSeverity::Warning)))
        [
            SNew(STextBlock)
            .Text(TAttribute<FText>::Create(
                TAttribute<FText>::FGetter::CreateSP(
                    this, &SLogFlowPanel::GetSeverityButtonText,
                    ELogFlowSeverity::Warning)))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
        ]
    ]

    // Error toggle
    + SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(FMargin(2.0f, 0.0f))
    [
        SNew(SCheckBox)
        .Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
        .IsChecked_Lambda([this]()
        {
            return bShowError ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        })
        .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
        {
            bShowError = (NewState == ECheckBoxState::Checked);
            ApplyFilters();
        })
        .ForegroundColor(TAttribute<FSlateColor>::Create(
            TAttribute<FSlateColor>::FGetter::CreateSP(
                this, &SLogFlowPanel::GetSeverityButtonColor,
                ELogFlowSeverity::Error)))
        [
            SNew(STextBlock)
            .Text(TAttribute<FText>::Create(
                TAttribute<FText>::FGetter::CreateSP(
                    this, &SLogFlowPanel::GetSeverityButtonText,
                    ELogFlowSeverity::Error)))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
        ]
    ]

    // Clear button
    + SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
    [
        SNew(SButton)
        .Text(LOCTEXT("ClearButton", "Clear"))
        .OnClicked_Lambda([this]() -> FReply
        {
            ClearEntries();
            return FReply::Handled();
        })
    ]
]

        // ── Separator ─────────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SSeparator)
            .Orientation(Orient_Horizontal)
        ]

        // ── Entry list ────────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SAssignNew(ListView, SListView<TSharedPtr<FLogFlowEntry>>)
            .ListItemsSource(&FilteredEntries)
            .OnGenerateRow(this, &SLogFlowPanel::GenerateRow)
            .SelectionMode(ESelectionMode::Single)
        ]
    ];

    RegisterWithDispatcher();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLogFlowPanel::~SLogFlowPanel()
{
    UnregisterFromDispatcher();
}

// -- ILogFlowConsumer ------------------------------------------------------------------------------------------------

void SLogFlowPanel::OnLogFlowEntryReceived(const FLogFlowEntry& Entry)
{
    PendingEntries.Enqueue(Entry);
}

// -- Public interface ------------------------------------------------------------------------------------------------

void SLogFlowPanel::ClearEntries()
{
    Entries.Empty();
    FilteredEntries.Empty();
    LogCount        = 0;
    WarningCount    = 0;
    ErrorCount      = 0;
    
    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
    }
}

void SLogFlowPanel::UpdateSettings(const FLogFlowSettings& NewSettings)
{
    Settings = NewSettings;
    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
    }
}

// -- Slate tick ------------------------------------------------------------------------------------------------------

void SLogFlowPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
    
    bNewEntriesAdded = false;
    
    // Drain up to 200 entries per frame to avoid hitches with burst logging.
    int32 EntriesProcessed = 0;
    FLogFlowEntry Incoming;
    while (EntriesProcessed < 200 && PendingEntries.Dequeue(Incoming))
    {
        TSharedPtr<FLogFlowEntry> NewEntry = MakeShared<FLogFlowEntry>(Incoming);
        Entries.Add(NewEntry);
        
        // Update severity counter
        switch (Incoming.Severity)
        {
            case ELogFlowSeverity::Log:     ++LogCount;     break;
            case ELogFlowSeverity::Warning: ++WarningCount; break;
            case ELogFlowSeverity::Error:   ++ErrorCount;   break;
            default: break;
        }
        
        // Add to filtered list directly if it passes the active filters.
        bool bPassesFilter = false;
        switch (Incoming.Severity)
        {
            case ELogFlowSeverity::Log:     bPassesFilter = bShowLog;     break;
            case ELogFlowSeverity::Warning: bPassesFilter = bShowWarning; break;
            case ELogFlowSeverity::Error:   bPassesFilter = bShowError;   break;
            default: bPassesFilter = true; break;
        }
        
        if (bPassesFilter)
        {
            FilteredEntries.Add(NewEntry);
            bNewEntriesAdded = true;
        }
        
        ++EntriesProcessed;
    }
    
    if (bNewEntriesAdded && ListView.IsValid())
    {
        ListView->RequestListRefresh();
        if (FilteredEntries.Num() > 0)
        {
            ListView->RequestScrollIntoView(FilteredEntries.Last());
        }
    }
}

// -- List View -------------------------------------------------------------------------------------------------------

TSharedRef<ITableRow> SLogFlowPanel::GenerateRow(TSharedPtr<FLogFlowEntry> Entry, 
    const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(SLogFlowEntryRow, OwnerTable)
        .Entry(Entry)
        .Settings(Settings);
}

// -- Helpers ---------------------------------------------------------------------------------------------------------

void SLogFlowPanel::RegisterWithDispatcher()
{
    if (ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get())
    {
        if (FLogFlowDispatcher* Dispatcher = Subsystem->GetDispatcher())
        {
            Dispatcher->RegisterConsumer(this);
        }
    }
}

void SLogFlowPanel::UnregisterFromDispatcher()
{
    if (ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get())
    {
        if (FLogFlowDispatcher* Dispatcher = Subsystem->GetDispatcher())
        {
            Dispatcher->UnregisterConsumer(this);
        }
    }
}

void SLogFlowPanel::ApplyFilters()
{
    FilteredEntries.Empty();
    
    for (const TSharedPtr<FLogFlowEntry>& Entry : Entries)
    {
        if (!Entry.IsValid()) continue;
        
        bool bVisible = false;
        switch (Entry->Severity)
        {
            case ELogFlowSeverity::Log:     bVisible = bShowLog;     break;
            case ELogFlowSeverity::Warning: bVisible = bShowWarning; break;
            case ELogFlowSeverity::Error:   bVisible = bShowError;   break;
            default:                        bVisible = true;          break;
        }
        
        if (bVisible)
        {
            FilteredEntries.Add(Entry);
        }
    }
    
    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
        
        if (FilteredEntries.Num() > 0)
        {
            ListView->RequestScrollIntoView(FilteredEntries.Last());
        }
    }
}

FText SLogFlowPanel::GetSeverityButtonText(ELogFlowSeverity Severity) const
{
    switch (Severity)
    {
    case ELogFlowSeverity::Warning:
        return FText::FromString(FString::Printf(TEXT("WRN (%d)"), WarningCount));
    case ELogFlowSeverity::Error:
        return FText::FromString(FString::Printf(TEXT("ERR (%d)"), ErrorCount));
    default:
        return FText::FromString(FString::Printf(TEXT("LOG (%d)"), LogCount));
    }
}

FSlateColor SLogFlowPanel::GetSeverityButtonColor(ELogFlowSeverity Severity) const
{
    switch (Severity)
    {
    case ELogFlowSeverity::Warning:
        return FSlateColor(bShowWarning
            ? FLinearColor(1.0f, 0.75f, 0.0f, 1.0f)
            : FLinearColor(0.4f, 0.3f, 0.0f, 1.0f));
    case ELogFlowSeverity::Error:
        return FSlateColor(bShowError
            ? FLinearColor(1.0f, 0.2f, 0.2f, 1.0f)
            : FLinearColor(0.4f, 0.0f, 0.0f, 1.0f));
    default:
        return FSlateColor(bShowLog
            ? FLinearColor(0.4f, 0.6f, 1.0f, 1.0f)
            : FLinearColor(0.1f, 0.2f, 0.4f, 1.0f));
    }
}

#undef LOCTEXT_NAMESPACE