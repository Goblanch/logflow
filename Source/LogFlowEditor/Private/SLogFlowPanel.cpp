#include "SLogFlowPanel.h"
#include "LogFlowSubsystem.h"
#include "LogFlowDispatcher.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "SlateOptMacros.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "SLogFlowPanel"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogFlowPanel::Construct(const FArguments& InArgs)
{
	Settings         = FLogFlowSettings::GetDefault();
    bNewEntriesAdded = false;

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

            // Clear button
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
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

        // ── Column headers ────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(4.0f, 0.0f))
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(FMargin(4.0f, 0.0f, 8.0f, 0.0f))
            [
                SNew(SBox).WidthOverride(36.0f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("ColSeverity", "SEV"))
                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
                    .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
                ]
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
            [
                SNew(SBox).WidthOverride(90.0f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("ColTime", "TIMESTAMP"))
                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
                    .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
                ]
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
            [
                SNew(SBox).WidthOverride(80.0f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("ColTag", "TAG"))
                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
                    .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
                ]
            ]

            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("ColMessage", "MESSAGE"))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
                .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
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
            .ListItemsSource(&Entries)
            .OnGenerateRow(this, &SLogFlowPanel::GenerateRow)
            .SelectionMode(ESelectionMode::Single)
        ]
    ];

    RegisterWithDispatcher();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLogFlowPanel::~SLogFlowPanel()
{
    unregisterFromDispatcher();
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
        Entries.Add(MakeShared<FLogFlowEntry>(Incoming));
        bNewEntriesAdded = true;
        ++EntriesProcessed;
    }
    
    if (bNewEntriesAdded && ListView.IsValid())
    {
        ListView->RequestListRefresh();
        
        // Auto-scroll to the latest entry
        if (Entries.Num() > 0)
        {
            ListView->RequestScrollIntoView(Entries.Last());
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

void SLogFlowPanel::unregisterFromDispatcher()
{
    if (ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get())
    {
        if (FLogFlowDispatcher* Dispatcher = Subsystem->GetDispatcher())
        {
            Dispatcher->UnregisterConsumer(this);
        }
    }
}

#undef LOCTEXT_NAMESPACE