#include "SLogFlowViewer.h"
#include "LogFlowSubsystem.h"
#include "LogFlowSessionManager.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "SlateOptMacros.h"
#include "Misc/FileHelper.h"
#include "Widgets/Input/SSearchBox.h"

#define LOCTEXT_NAMESPACE "SlogFlowViewer"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogFlowViewer::Construct(const FArguments& InArgs)
{
	ActiveSearchText = TEXT("");
	CurrentSearchResultIndex = -1;
	
	ChildSlot
	[
		SNew(SSplitter)
		.Orientation(Orient_Horizontal)

		// ── Left sidebar — session list ───────────────────────────
		+ SSplitter::Slot()
		.Value(0.25f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.Padding(FMargin(4.0f))
			[
				SNew(SVerticalBox)

				// Sidebar header
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 0.0f, 0.0f, 4.0f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SessionListHeader", "Sessions"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]

				// Session list
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SAssignNew(SessionListView,
						SListView<TSharedPtr<FLogFlowSessionInfo>>)
					.ListItemsSource(&SessionItems)
					.OnGenerateRow(this, &SLogFlowViewer::GenerateSessionRow)
					.OnSelectionChanged(this, &SLogFlowViewer::OnSessionSelected)
					.SelectionMode(ESelectionMode::Single)
				]
			]
		]

		// ── Right area — content viewer ───────────────────────────
		+ SSplitter::Slot()
		.Value(0.75f)
		[
			BuildViewingArea()
		]
	];

	// Load initial session list
	RefreshSessionList();

	// Subscribe to index changes so the list refreshes after each PIE session
	if (ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get())
	{
		if (FLogFlowSessionManager* Manager = Subsystem->GetSessionManager())
		{
			SessionIndexChangedHandle =
				Manager->OnSessionIndexChanged.AddSP(
					this, &SLogFlowViewer::RefreshSessionList);
		}
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLogFlowViewer::~SLogFlowViewer()
{
	if (ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get())
	{
		if (FLogFlowSessionManager* Manager = Subsystem->GetSessionManager())
		{
			Manager->OnSessionIndexChanged.Remove(SessionIndexChangedHandle);
		}
	}
}

void SLogFlowViewer::RefreshSessionList()
{
	SessionItems.Empty();
	
	if (ULogFlowSubsystem* Subsystem = ULogFlowSubsystem::Get())
	{
		if (FLogFlowSessionManager* Manager = Subsystem->GetSessionManager())
		{
			const TArray<FLogFlowSessionInfo> Index = Manager->GetSessionIndex();
			for (const FLogFlowSessionInfo& Info : Index)
			{
				SessionItems.Add(MakeShared<FLogFlowSessionInfo>(Info));
			}
		}
	}
	
	if (SessionListView.IsValid())
	{
		SessionListView->RequestListRefresh();
	}
}

TSharedRef<ITableRow> SLogFlowViewer::GenerateSessionRow(TSharedPtr<FLogFlowSessionInfo> SessionInfo, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!SessionInfo.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FLogFlowSessionInfo>>, OwnerTable);
	}

	return SNew(STableRow<TSharedPtr<FLogFlowSessionInfo>>, OwnerTable)
		.Padding(FMargin(4.0f, 2.0f))
		[
			SNew(SVerticalBox)

			// Date and time
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(SessionInfo->GetDisplayTime()))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]

			// File size
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(SessionInfo->GetDisplaySize()))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
			]
		];
}

void SLogFlowViewer::OnSessionSelected(TSharedPtr<FLogFlowSessionInfo> SessionInfo, ESelectInfo::Type SelectInfo)
{	
	SelectedSession = SessionInfo;

	if (SessionInfo.IsValid())
	{
		LoadSessionContent(SessionInfo->FilePath);
	}
	else
	{
		ViewerLines.Empty();
		if (ContentListView.IsValid())
		{
			ContentListView->RequestListRefresh();
		}
	}
}

TSharedRef<SWidget> SLogFlowViewer::BuildViewingArea()
{
	TSharedPtr<SScrollBar> ScrollBar =
        SNew(SScrollBar).Orientation(Orient_Vertical);

    return SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
        .Padding(FMargin(4.0f))
        [
            SNew(SVerticalBox)

            // ── Header ────────────────────────────────────────────
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0.0f, 0.0f, 0.0f, 4.0f))
            [
                SNew(STextBlock)
                .Text(LOCTEXT("ViewerHeader", "Session Content"))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
            ]

            // ── Search bar ────────────────────────────────────────
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0.0f, 0.0f, 0.0f, 4.0f))
            [
                SNew(SHorizontalBox)

                // Search box
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .VAlign(VAlign_Center)
                [
                    SAssignNew(ViewerSearchBox, SSearchBox)
                    .HintText(LOCTEXT("SearchHint", "Search in session..."))
                    .OnTextChanged_Lambda([this](const FText& NewText)
                    {
                        ActiveSearchText = NewText.ToString();
                        CurrentSearchResultIndex = -1;
                        RebuildSearchResults();

                        if (SearchResultIndices.Num() > 0)
                        {
                            CurrentSearchResultIndex = 0;
                            ScrollToSearchResult(0);
                        }

                        UpdateSearchResultLabel();
                    })
                ]

                // Previous button
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
                [
                    SNew(SButton)
                    .Text(LOCTEXT("PrevResult", "▲"))
                    .ToolTipText(LOCTEXT("PrevResultTooltip", "Previous result"))
                    .OnClicked_Lambda([this]() -> FReply
                    {
                        if (SearchResultIndices.Num() == 0)
                        {
                            return FReply::Handled();
                        }
                        CurrentSearchResultIndex =
                            (CurrentSearchResultIndex - 1 + SearchResultIndices.Num())
                            % SearchResultIndices.Num();
                        ScrollToSearchResult(CurrentSearchResultIndex);
                        UpdateSearchResultLabel();
                        return FReply::Handled();
                    })
                ]

                // Next button
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(FMargin(2.0f, 0.0f, 0.0f, 0.0f))
                [
                    SNew(SButton)
                    .Text(LOCTEXT("NextResult", "▼"))
                    .ToolTipText(LOCTEXT("NextResultTooltip", "Next result"))
                    .OnClicked_Lambda([this]() -> FReply
                    {
                        if (SearchResultIndices.Num() == 0)
                        {
                            return FReply::Handled();
                        }
                        CurrentSearchResultIndex =
                            (CurrentSearchResultIndex + 1)
                            % SearchResultIndices.Num();
                        ScrollToSearchResult(CurrentSearchResultIndex);
                        UpdateSearchResultLabel();
                        return FReply::Handled();
                    })
                ]

                // Result counter
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
                [
                    SAssignNew(SearchResultLabel, STextBlock)
                    .Text(FText::GetEmpty())
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
                    .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
                ]
            ]

            // ── Content list + scrollbar ──────────────────────────
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                [
                    SAssignNew(ContentListView,
                        SListView<TSharedPtr<FLogFlowViewerLine>>)
                    .ListItemsSource(&ViewerLines)
                    .OnGenerateRow(this, &SLogFlowViewer::GenerateContentRow)
                    .SelectionMode(ESelectionMode::Single)
                    .ExternalScrollbar(ScrollBar.ToSharedRef())
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    ScrollBar.ToSharedRef()
                ]
            ]
        ];
}

void SLogFlowViewer::LoadSessionContent(const FString& FilePath)
{
    ViewerLines.Empty();
	
	ActiveSearchText = TEXT("");
	CurrentSearchResultIndex = -1;
	SearchResultIndices.Empty();
	if (ViewerSearchBox.IsValid())
	{
		ViewerSearchBox->SetText(FText::GetEmpty());
	}
	if (SearchResultLabel.IsValid())
	{
		SearchResultLabel->SetText(FText::GetEmpty());
	}

    TArray<FString> RawLines;
    if (!FFileHelper::LoadFileToStringArray(RawLines, *FilePath))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("LogFlow Viewer: Could not read session file at %s"), *FilePath);

        ViewerLines.Add(MakeShared<FLogFlowViewerLine>(
            FString::Printf(TEXT("Could not read file: %s"), *FilePath),
            ELogFlowSeverity::Error));

        if (ContentListView.IsValid())
        {
            ContentListView->RequestListRefresh();
        }
        return;
    }

    ViewerLines.Reserve(RawLines.Num());
    for (const FString& Line : RawLines)
    {
        if (Line.IsEmpty())
        {
            continue;
        }

        ViewerLines.Add(MakeShared<FLogFlowViewerLine>(
            Line,
            FLogFlowViewerLine::ParseSeverity(Line)));
    }

    if (ContentListView.IsValid())
    {
        ContentListView->RequestListRefresh();

        // Scroll to top when loading a new session
        if (ViewerLines.Num() > 0)
        {
            ContentListView->RequestScrollIntoView(ViewerLines[0]);
        }
    }
}

TSharedRef<ITableRow> SLogFlowViewer::GenerateContentRow(
    TSharedPtr<FLogFlowViewerLine> Line,
    const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!Line.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FLogFlowViewerLine>>, OwnerTable);
	}

	// Find the index of this line in ViewerLines for search highlight
	const int32 LineIndex = ViewerLines.IndexOfByKey(Line);
	const FSlateColor RowColor =
		GetContentRowColorWithSearch(LineIndex, Line->Severity);

	return SNew(STableRow<TSharedPtr<FLogFlowViewerLine>>, OwnerTable)
		.Padding(FMargin(4.0f, 1.0f))
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(RowColor)
			.Padding(FMargin(2.0f, 1.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Line->RawText))
				.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
		];
}

FSlateColor SLogFlowViewer::GetContentRowColor(ELogFlowSeverity Severity)
{
    switch (Severity)
    {
        case ELogFlowSeverity::Warning:
            return FSlateColor(FLinearColor(0.25f, 0.18f, 0.0f, 0.6f));
        case ELogFlowSeverity::Error:
            return FSlateColor(FLinearColor(0.3f, 0.0f, 0.0f, 0.6f));
        default:
            return FSlateColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
    }
}

void SLogFlowViewer::RebuildSearchResults()
{
	SearchResultIndices.Empty();
	
	if (ActiveSearchText.IsEmpty())
	{
		if (ContentListView.IsValid())
		{
			ContentListView->RebuildList();
		}
		return;
	}
	
	for (int32 i = 0; i < ViewerLines.Num(); ++i)
	{
		if (ViewerLines[i].IsValid() &&
			ViewerLines[i]->RawText.Contains(ActiveSearchText, ESearchCase::IgnoreCase))
		{
			SearchResultIndices.Add(i);
		}
	}
	
	if (ContentListView.IsValid())
	{
		ContentListView->RebuildList();
	}
}

void SLogFlowViewer::ScrollToSearchResult(int32 ResultIndex)
{
	if (!ContentListView.IsValid() ||
		!SearchResultIndices.IsValidIndex(ResultIndex))
	{
		return;
	}
	
	const int32 LineIndex = SearchResultIndices[ResultIndex];
	if (ViewerLines.IsValidIndex(LineIndex))
	{
		ContentListView->RebuildList();
		ContentListView->RequestScrollIntoView(ViewerLines[LineIndex]);
	}
}

void SLogFlowViewer::UpdateSearchResultLabel()
{
	if (!SearchResultLabel.IsValid()) return;
	
	if (ActiveSearchText.IsEmpty() || SearchResultIndices.Num() == 0)
	{
		SearchResultLabel->SetText(FText::GetEmpty());
		return;
	}
	
	const FString LabelText = FString::Printf(
		TEXT("%d / %d"),
		CurrentSearchResultIndex + 1,
		SearchResultIndices.Num());
	
	SearchResultLabel->SetText(FText::FromString(LabelText));
}

FSlateColor SLogFlowViewer::GetContentRowColorWithSearch(int32 LineIndex, ELogFlowSeverity Severity) const
{
	// Highlighted search result — bright amber regardless of severity
	if (!ActiveSearchText.IsEmpty() &&
		SearchResultIndices.IsValidIndex(CurrentSearchResultIndex) &&
		SearchResultIndices[CurrentSearchResultIndex] == LineIndex)
	{
		return FSlateColor(FLinearColor(0.5f, 0.4f, 0.0f, 0.9f));
	}

	// Search match but not the current result — subtle highlight
	if (!ActiveSearchText.IsEmpty() &&
		SearchResultIndices.Contains(LineIndex))
	{
		return FSlateColor(FLinearColor(0.2f, 0.15f, 0.0f, 0.7f));
	}

	// Default severity color
	return GetContentRowColor(Severity);
}

#undef LOCTEXT_NAMESPACE