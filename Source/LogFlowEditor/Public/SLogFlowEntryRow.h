#pragma once
#include "CoreMinimal.h"
#include "LogFlowEntry.h"
#include "LogFlowSettings.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"

/**
 * Slate widget that renders a single FLogFlowEntry as a row in the 
 * LogFlow Panel list view.
 * 
 * Each row displays:
 * - A colored severity indicator on the left.
 * - Timestamp formatted according to the active settings.
 * - Tag name rendered in its configuration custom color.
 * - Message text.
 * 
 * Background color varies by severity:
 * - Log:		neutral dark (default panel background).
 * - Warning:	soft amber.
 * - Error:		soft red.
 */
class SLogFlowEntryRow : public STableRow<TSharedPtr<FLogFlowEntry>>
{
public:
	SLATE_BEGIN_ARGS(SLogFlowEntryRow) {}
		/** The log entry to display in this row. */
		SLATE_ARGUMENT(TSharedPtr<FLogFlowEntry>, Entry)
		/*+ Active runtime settings used for timestamp mode and tag colors */
		SLATE_ARGUMENT(FLogFlowSettings, Settings)
	SLATE_END_ARGS()

	/**
	 * Constructs the row widget.
	 * 
	 * @param InArgs Slate arguments containing the entry and settings.
	 * @param InOwnerTable The list view that owns this row.
	 */
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable);
	
private:
	/**
	 * Returns the background color for this row based on entry severity.
	 * 
	 * @return FSlateColor appropiate for the entry severity level.
	 */
	FSlateColor GetRowBackgroundColor() const;

	/**
	 * Returns the display color for the severity indicator blcok.
	 * 
	 * @return FSlateColor matching the severity level.
	 */
	FSlateColor GetSeverityColor() const;

	/**
	 * Returns the display color for the tag text.
	 * Uses the configured color if the tag has one, otherwise white.
	 * 
	 * @return FSlateColor for the tag label.
	 */
	FSlateColor GetTagColor() const;

	/**
	 * Formats the timestamp of the entry based on the configured module.
	 * 
	 * @return Formatted timestamp string. Format: HH:MM:SS.mmm
	 */
	FText GetFormattedTimestamp() const;

	/**
	 * Returns the severity label text for display.
	 * 
	 * @return Short severity string: LOG / WRN / ERR
	 */
	FText GetSeverityText() const;
	
	/** The log entry displayed by this row */
	TSharedPtr<FLogFlowEntry> Entry;
	
	/** Runtime settings - used for timestamp mode and tag color lookup. */
	FLogFlowSettings Settings;
	
	// -- Colors ----------------------------------------------------------------------------
	
	/** Background color for Log entries — transparent, uses list default. */
	static const FLinearColor BackgroundLog;

	/** Background color for Warning entries — soft amber. */
	static const FLinearColor BackgroundWarning;

	/** Background color for Error entries — soft red. */
	static const FLinearColor BackgroundError;

	/** Indicator color for Log severity. */
	static const FLinearColor SeverityColorLog;

	/** Indicator color for Warning severity. */
	static const FLinearColor SeverityColorWarning;

	/** Indicator color for Error severity. */
	static const FLinearColor SeverityColorError;

	/** Default tag color when no custom color is configured. */
	static const FLinearColor TagColorDefault;
};