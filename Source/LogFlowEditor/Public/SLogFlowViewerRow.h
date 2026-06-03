#pragma once

#include "CoreMinimal.h"
#include "FLogFlowViewerLine.h"
#include "Widgets/Views/STableRow.h"

/**
 * Table row widget for the Log Viewer content list.
 * Handles right-click context menu for clipboard copy.
 */
class SLogFlowViewerRow
	: public STableRow<TSharedPtr<FLogFlowViewerLine>>
{
public:

	SLATE_BEGIN_ARGS(SLogFlowViewerRow) {}
	SLATE_ARGUMENT(TSharedPtr<FLogFlowViewerLine>, Line)
	SLATE_ATTRIBUTE(FSlateColor, RowColor)
	SLATE_EVENT(FSimpleDelegate, OnCopyRequested)
SLATE_END_ARGS()

void Construct(
	const FArguments& InArgs,
	const TSharedRef<STableViewBase>& InOwnerTable);

private:

	virtual FReply OnMouseButtonUp(
		const FGeometry& MyGeometry,
		const FPointerEvent& MouseEvent) override;

	TSharedPtr<FLogFlowViewerLine> Line;
	FSimpleDelegate OnCopyRequested;
};