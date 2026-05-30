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

#define LOCTEXT_NAMESPACE "SlogFlowViewer"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogFlowViewer::Construct(const FArguments& InArgs)
{
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
	// Build scrollbar first so it can be passed to the list view
	TSharedPtr<SScrollBar> ScrollBar =
		SNew(SScrollBar).Orientation(Orient_Vertical);

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(4.0f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 4.0f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ViewerHeader", "Session Content"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			]

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

    const FSlateColor RowColor = GetContentRowColor(Line->Severity);

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

#undef LOCTEXT_NAMESPACE