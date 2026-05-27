#include "SLogFlowPanel.h"
#include "LogFlowSubsystem.h"
#include "LogFlowDispatcher.h"
#include "LogFlowEditorSettings.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "SlateOptMacros.h"
#include "Engine/Engine.h"
#include "LogFlowSeverity.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "HAL/PlatformApplicationMisc.h"
#include "LogFlowEditorSettings.h"

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
    ActiveTagFilter     = NAME_None;
    ActiveSearchText    = TEXT("");
    TagOptions.Add(MakeShared<FName>(NAME_None));

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

        // Tag filter
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
        [
            SNew(SBox)
            .WidthOverride(100.0f)
            [
                SAssignNew(TagComboBox, SComboBox<TSharedPtr<FName>>)
                .OptionsSource(&TagOptions)
                .OnSelectionChanged_Lambda([this](TSharedPtr<FName> Selected, ESelectInfo::Type)
                {
                    ActiveTagFilter = Selected.IsValid() ? *Selected : NAME_None;
                    ApplyFilters();
                })
                .OnGenerateWidget_Lambda([](TSharedPtr<FName> Item) -> TSharedRef<SWidget>
                {
                    return SNew(STextBlock)
                        .Text(Item.IsValid() && !Item->IsNone()
                            ? FText::FromName(*Item)
                            : FText::FromString(TEXT("All")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 9));
                })
                .Content()
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]() -> FText
                    {
                        return ActiveTagFilter.IsNone()
                            ? FText::FromString(TEXT("All"))
                            : FText::FromName(ActiveTagFilter);
                    })
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
                ]
            ]
        ]

        // ── Search box ─────────────────────────────────────────────
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
        [
            SNew(SBox)
            .WidthOverride(160.0f)
            [
                SAssignNew(SearchBox, SSearchBox)
                .HintText(LOCTEXT("SearchHint", "Search..."))
                .OnTextChanged_Lambda([this](const FText& NewText)
                {
                    ActiveSearchText = NewText.ToString();
                    ApplyFilters();
                })
                .OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
                {
                    ActiveSearchText = NewText.ToString();
                    ApplyFilters();
                })
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

        // ── Copy All button ────────────────────────────────────────
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
        [
            SNew(SButton)
            .Text(LOCTEXT("CopyAllButton", "Copy All"))
            .OnClicked_Lambda([this]() -> FReply
            {
                CopyAllToClipboard();
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
    
    BeginPIEHandle = FEditorDelegates::BeginPIE.AddSP(this, &SLogFlowPanel::OnBeginPIE);
    
    RegisterWithDispatcher();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLogFlowPanel::~SLogFlowPanel()
{
    FEditorDelegates::BeginPIE.Remove(BeginPIEHandle);
    
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
    LogCount            = 0;
    WarningCount        = 0;
    ErrorCount          = 0;
    KnownTags.Empty();
    ActiveTagFilter     = NAME_None;
    ActiveSearchText    = TEXT("");
    TagOptions.Empty();
    TagOptions.Add(MakeShared<FName>(NAME_None));
    
    if (TagComboBox.IsValid())
    {
        TagComboBox->RefreshOptions();
        TagComboBox->SetSelectedItem(TagOptions[0]);
    }
    
    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
    }
    
    if (SearchBox.IsValid())
    {
        SearchBox->SetText(FText::GetEmpty());
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
        
        // Track new tags
        if (!Incoming.Tag.IsNone() && !KnownTags.Contains(Incoming.Tag))
        {
            KnownTags.Add(Incoming.Tag);
            RebuildTagOptions();
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
        
        if (bPassesFilter) bPassesFilter = PassesTagFilter(Incoming);
        
        if (bPassesFilter) bPassesFilter = PassesSearchFilter(Incoming);
        
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
        .Settings(Settings)
        .OnCopyRequested(FSimpleDelegate::CreateLambda([this, Entry]()
        {
            CopyEntryToClipboard(Entry);
        }));
}

// -- Helpers ---------------------------------------------------------------------------------------------------------

void SLogFlowPanel::RegisterWithDispatcher()
{
    // Load settings from editor preferences
    if (const ULogFlowEditorSettings* EditorSettings = ULogFlowEditorSettings::Get())
    {
        Settings = EditorSettings->ToRuntimeSettings();
    }
    
    if (ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get())
    {
        if (FLogFlowDispatcher* Dispatcher = Subsystem->GetDispatcher())
        {
            Dispatcher->RegisterConsumer(this);
        }
        
        SettingsChangedHandle = Subsystem->OnSettingsChanged.AddSP(
            this, &SLogFlowPanel::UpdateSettings);
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
        
        Subsystem->OnSettingsChanged.Remove(SettingsChangedHandle);
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
        
        if (bVisible) bVisible = PassesTagFilter(*Entry);
        
        if (bVisible) bVisible = PassesSearchFilter(*Entry);
        
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

void SLogFlowPanel::RebuildTagOptions()
{
    TagOptions.Empty();
    TagOptions.Add(MakeShared<FName>(NAME_None));
    
    for (const FName& currentTag : KnownTags)
    {
        TagOptions.Add(MakeShared<FName>(currentTag));
    }
    
    if (TagComboBox.IsValid())
    {
        TagComboBox->RefreshOptions();
    }
}

bool SLogFlowPanel::PassesTagFilter(const FLogFlowEntry& Entry) const
{
    if (ActiveTagFilter.IsNone())
    {
        return true;
    }
    
    return Entry.Tag == ActiveTagFilter;
}

bool SLogFlowPanel::PassesSearchFilter(const FLogFlowEntry& Entry) const
{
    if (ActiveSearchText.IsEmpty())
    {
        return true;
    }
    
    return Entry.Message.Contains(ActiveSearchText, ESearchCase::IgnoreCase);
}

void SLogFlowPanel::OnBeginPIE(bool bIsSimulating)
{
    // DIAGNÓSTICO
    UE_LOG(LogTemp, Warning, TEXT("LogFlow: OnBeginPIE called — bAutoClear = %s"),
        Settings.bAutoClear ? TEXT("true") : TEXT("false"));
    
    if (Settings.bAutoClear)
    {
        ClearEntries();
        UE_LOG(LogTemp, Warning, TEXT("LogFlow: Panel cleared"));
    }
}

FString SLogFlowPanel::FormatEntryForClipboard(const FLogFlowEntry& Entry) const
{
    // Timestamp
    FString Timestamp;
    if (Settings.TimestampMode == ELogFlowTimestampMode::SystemTime)
    {
        Timestamp = FString::Printf(TEXT("%02d:%02d:%02d.%03d"),
            Entry.SystemTime.GetHour(),
            Entry.SystemTime.GetMinute(),
            Entry.SystemTime.GetSecond(),
            Entry.SystemTime.GetMillisecond());
    }
    else
    {
        const int32 TotalSeconds = static_cast<int32>(Entry.Timestamp.GetTotalSeconds());
        Timestamp = FString::Printf(TEXT("%02d:%02d:%02d.%03d"),
            TotalSeconds / 3600,
            (TotalSeconds % 3600) / 60,
            TotalSeconds % 60,
            Entry.Timestamp.GetFractionMilli());
    }

    // Severity — padded to 7 chars for alignment
    FString Severity;
    switch (Entry.Severity)
    {
    case ELogFlowSeverity::Warning: Severity = TEXT("WARNING"); break;
    case ELogFlowSeverity::Error:   Severity = TEXT("ERROR  "); break;
    default:                        Severity = TEXT("LOG    "); break;
    }

    // Tag
    const FString EntryTag = Entry.Tag.IsNone()
        ? TEXT("-")
        : Entry.Tag.ToString();

    return FString::Printf(TEXT("[%s] [%s] [%s] %s"),
        *Timestamp, *Severity, *EntryTag, *Entry.Message);
}

void SLogFlowPanel::CopyEntryToClipboard(const TSharedPtr<FLogFlowEntry>& Entry) const
{
    if (!Entry.IsValid()) return;
    FPlatformApplicationMisc::ClipboardCopy(*FormatEntryForClipboard(*Entry));
}

void SLogFlowPanel::CopyAllToClipboard() const
{
    if (FilteredEntries.Num() == 0) return;
    
    TArray<FString> Lines;
    Lines.Reserve(FilteredEntries.Num());
    
    for (const TSharedPtr<FLogFlowEntry>& Entry : FilteredEntries)
    {
        if (Entry.IsValid())
        {
            Lines.Add(FormatEntryForClipboard(*Entry));
        }
    }
    
    FPlatformApplicationMisc::ClipboardCopy(*FString::Join(Lines, TEXT("\n")));
}

#undef LOCTEXT_NAMESPACE