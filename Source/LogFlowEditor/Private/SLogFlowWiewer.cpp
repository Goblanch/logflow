#include "SLogFlowViewer.h"
#include "LogFlowSubsystem.h"
#include "LogFlowSessionManager.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "SlateOptMacros.h"

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

	if (ViewingAreaText.IsValid())
	{
		if (SessionInfo.IsValid())
		{
			ViewingAreaText->SetText(FText::FromString(
				FString::Printf(TEXT("Selected: %s\n\nContent viewer coming in #33."),
					*SessionInfo->FileName)));
		}
		else
		{
			ViewingAreaText->SetText(
				LOCTEXT("NoSessionSelected", "Select a session from the list."));
		}
	}
}

TSharedRef<SWidget> SLogFlowViewer::BuildViewingArea()
{
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
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(ViewingAreaText, STextBlock)
				.Text(LOCTEXT("NoSessionSelected", "Select a session from the list."))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
			]
		];
}

#undef LOCTEXT_NAMESPACE