#pragma once

#include "CoreMinimal.h"
#include "ILogFlowConsumer.h"
#include "LogFlowEntry.h"
#include "LogFlowSettings.h"
#include "SLogFlowEntryRow.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SComboBox.h"

/**
 * Main dockable panel widget for the LogFlow system.
 * 
 * SLogFlowPanel implements ILogFlowConsumer to receive log entries from 
 * FLogFlowDispatcher in real time during PIE sessions. Incoming entries
 * are enqueued thread-safely and processed on the editor main thread via
 * a Slate tick, ensuring no game thread work happens on the UI thread.
 * 
 * The panel renders entries using SListView<TSharedPtr<FLogFlowEntry>>
 * with SLogFlowEntryRow as the row widget. It auto-scrolls to the latest
 * entry on each new arrival.
 * 
 */
class LOGFLOWEDITOR_API SLogFlowPanel : public SCompoundWidget, public ILogFlowConsumer
{
public:
	SLATE_BEGIN_ARGS(SLogFlowPanel){}
	SLATE_END_ARGS()

	/**
	 * Constructs the panel widget and registers it as a consumer
	 * with the LogFlowCore dispatcher.
	 * 
	 * @param InArgs Slate construction arguments.
	 */
	void Construct(const FArguments& InArgs);
	
	/** Destructor. Unregisters from the dispatcher. */
	virtual ~SLogFlowPanel() override;
	
	// -- ILogFlowConsumer ------------------------------------------------

	/**
	 * Called by FLogFlowDispatcher when a new entry arrives.
	 * Enqueues the entry for processing on the main thread.
	 * Safe to call from any thread.
	 * 
	 * @param Entry The incoming log entry.
	 */
	virtual void OnLogFlowEntryReceived(const FLogFlowEntry& Entry) override;
	
	// -- Public interface ---------------------------------------------------------

	/**
	 * Clears all entries from the panel.
	 * Called when bAutoClear is active and a new PIE session starts.
	 * Must be called from the main thread.
	 */
	void ClearEntries();

	/**
	 * Updates the runtime settings used by the panel and its rows.
	 * Called when the user changes preferences.
	 * 
	 * @param NewSettings The updated configuration.
	 */
	void UpdateSettings(const FLogFlowSettings& NewSettings);
	
private:
	
	// -- Slate tick ----------------------------------------------------------------

	/**
	 * Slate tick callback. Drains the pending entry queue and adds
	 * new entries to the list view on the main thread.
	 * 
	 * @param AllottedGeometry The geometry allocated to this widget.
	 * @param InCurrentTime Current time in seconds.
	 * @param InDeltaTime Time elapsed since the last tick.
	 */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	// -- List view --------------------------------------------------------------------------------

	/**
	 * Generates a row widget for a given entry in the list view.
	 * 
	 * @param Entry The entry to render.
	 * @param OwnerTable The list view that owns the row.
	 * @return The constructed row widget.
	 */
	TSharedRef<ITableRow> GenerateRow(
		TSharedPtr<FLogFlowEntry> Entry,
		const TSharedRef<STableViewBase>& OwnerTable);
	
	// -- Helpers ------------------------------------------------------------------------------------

	/**
	 * Registers the panel as a consumer with the LogFlowCore dispatcher.
	 * Called during Construct().
	 */
	void RegisterWithDispatcher();

	/**
	 * Unregisters the panel from the LogFlowCore dispatcher.
	 * Called during destruction.
	 */
	void UnregisterFromDispatcher();
	
	// -- Data --------------------------------------------------------------------------------------------

	/**
	 * All entries currently displayed in the panel.
	 * Only accesed form the main thread.
	 */
	TArray<TSharedPtr<FLogFlowEntry>> Entries;

	/**
	 * Thread-safe queue for entries arriving from the game thread.
	 * Produced by OnLogEntryReceived() from any thread.
	 * Consumed by Tick() on the main thread.
	 */
	TQueue<FLogFlowEntry, EQueueMode::Spsc> PendingEntries;
	
	/** The list view widget displaying the entries. */
	TSharedPtr<SListView<TSharedPtr<FLogFlowEntry>>> ListView;
	
	/** Active runtime settings. */
	FLogFlowSettings Settings;

	/**
	 * Set to true by Tick() when new entries were added in the last frame.
	 * Used to trigger auto-scroll only when needed.
	 */
	bool bNewEntriesAdded;
	
	// -- Severity Filter ---------------------------------------------------------------------------------------------

	/**
	 * Recalculates FilteredEntries by applying all active filters to Entries.
	 * Must be called from the main thread.
	 */
	void ApplyFilters();

	/**
	 * Returns the display text for a severity toggle button.
	 * Format: "LOG (42)" - label plus total count for that severity.
	 * 
	 * @param Severity The severity level for this button.
	 * @return Formatted button label text.
	 */
	FText GetSeverityButtonText(ELogFlowSeverity Severity) const;

	/**
	 * Returns the color of a severity toggle button based on its active state.
	 * 
	 * @param Severity The severity for this button.
	 * @return reflecting active or inactive state.
	 */
	FSlateColor GetSeverityButtonColor(ELogFlowSeverity Severity) const;

	/**
	 * Entries currently visible in the list view after filters are applied.
	 * The ListView sources from this array, not from Entries directly.
	 */
	TArray<TSharedPtr<FLogFlowEntry>> FilteredEntries;
	
	/** Whether Log entries are currently visible. */
	bool bShowLog;
	
	/** Whether Warning entries are currently visible. */
	bool bShowWarning;
	
	/** Whether Error entries are currently visible. */
	bool bShowError;
	
	/** Total count of Log entries received. Used for button label. */
	int32 LogCount;
	
	/** Total count of Warning entries received. Used for button label. */
	int32 WarningCount;
	
	/** Total count of Error entries received. Used for button label. */
	int32 ErrorCount;
	
	// -- Tag filter -----------------------------------------------------------------------
	
	/**
	 * Rebuilds the tag selector options list from KnownTags.
	 * Called when a new tag is seen for the first time.
	 */
	void RebuildTagOptions();

	/**
	 * Returns true if the given entry passes the active tag filter.
	 * 
	 * @param Entry The entry to evaluate.
	 * @return True if the entry should be visible given the active tag filter.
	 */
	bool PassesTagFilter(const FLogFlowEntry& Entry) const;
	
	/**
	 * All tags seen during the current session.
	 * Used to populate the tag selector dropdown.
	 */
	TSet<FName> KnownTags;
	
	/**
	 * Options list for the tag combo box.
	 * First element is always NAME_None (displayed as "All").
	 */
	TArray<TSharedPtr<FName>> TagOptions;
	
	/**
	 * The currently selected tag filter.
	 * NAME_None means "All" - no tag filter active.
	 */
	FName ActiveTagFilter;
	
	/** The tag combo box widget. */
	TSharedPtr<SComboBox<TSharedPtr<FName>>> TagComboBox;
	
	// -- Text search -----------------------------------------------------------------------------------------

	/**
	 * Returns true if the given entry passes the active text search filter.
	 * Search is case-insensitive and matches against the message text.
	 * 
	 * @param Entry The entry to evaluate
	 * @return True if the entry should be visible given the active search text.
	 */
	bool PassesSearchFilter(const FLogFlowEntry& Entry) const;
	
	/**
	 * The current search string entered by the user.
	 * Empty string means no search filter active.
	 */
	FString ActiveSearchText;
	
	/** The search box widget. Used to clear it */
	TSharedPtr<SSearchBox> SearchBox;
};