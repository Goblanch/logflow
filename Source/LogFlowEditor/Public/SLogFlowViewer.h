#pragma once

#include "CoreMinimal.h"
#include "LogFlowSessionInfo.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "FLogFlowViewerLine.h"

/**
 * Log Viewer widget - provides a dedicated editor window for browsing
 * and reading past LogFlow session files.
 * 
 * Layout:
 * ┌─────────────────┬──────────────────────────────────┐
 * │  Session list   │  Session content viewing area    │
 * │  (left sidebar) │  (implemented in #33)            │
 * └─────────────────┴──────────────────────────────────┘
 * 
 * The session list reads from FLogFlowSessionManager via 
 * ULogFlowSubsystem and refreshes automatically when a new 
 * session ends via the OnSessionIndexChanged delegate.
 */
class LOGFLOWEDITOR_API SLogFlowViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLogFlowViewer){}
	SLATE_END_ARGS()
	
	/**
	 * Constructs the viewer widget, loads the session index and
	 * subscribes to the OnSessionIndexChanged delegate.
	 */
	void Construct(const FArguments& InArgs);
	
	/** Destructor. Unsubscribes from the OnSessionIndexChanged delegate. */
	virtual ~SLogFlowViewer() override;
	
	/**
	 * Reloads the session list from FLogFlowSessionManager.
	 * Called on construction and when OnSessionIndexChanged fires.
	 */
	void RefreshSessionList();
	
private:
	
	// -- Session list -------------------------------------------------------------------------------------------------

	/**
	 * Generates a row widget for a session entry in the list.
	 * 
	 * @param SessionInfo The session info to render.
	 * @param OwnerTable The list view that owns the row.
	 * @return The constructed row widget.
	 */
	TSharedRef<ITableRow> GenerateSessionRow(
		TSharedPtr<FLogFlowSessionInfo> SessionInfo,
		const TSharedRef<STableViewBase>& OwnerTable);

	/**
	 * Called when the user selects a session in the list.
	 * Triggers loading the session content in the viewing area.
	 * 
	 * @param SessionInfo The selected session.
	 * @param SelectInfo How the selection was made.
	 */
	void OnSessionSelected(
		TSharedPtr<FLogFlowSessionInfo> SessionInfo,
		ESelectInfo::Type SelectInfo);

	/**
	 * Returns the content widget for the right-hand viewing area.
	 * 
	 * @return The viewing area widget.
	 */
	TSharedRef<SWidget> BuildViewingArea();
	
	// -- Data ---------------------------------------------------------------------------------------------------------
	
	/** Session entries displayed in the left sidebar. */
	TArray<TSharedPtr<FLogFlowSessionInfo>> SessionItems;
	
	/** The session list view widget. */
	TSharedPtr<SListView<TSharedPtr<FLogFlowSessionInfo>>> SessionListView;
	
	/** The currently selected session. Nullptr if none selected. */
	TSharedPtr<FLogFlowSessionInfo> SelectedSession;

	/** Text block in the viewing area showing the selected session path. */
	TSharedPtr<STextBlock> ViewingAreaText;
	
	/* Handle for the OnSessionIndexChanged delegate subscription. */
	FDelegateHandle SessionIndexChangedHandle;
	
	// -- Content viewer -----------------------------------------------------------------------------------------------

	/**
	 * Loads the content of the given session file into ViewerLines
	 * and refresesh the content list view.
	 * 
	 * @param FilePath Absolute path to the session file to load.
	 */
	void LoadSessionContent(const FString& FilePath);

	/**
	 * Genertaes a row widget for a line in the content list view.
	 * 
	 * @param Line The parsed line to render.
	 * @param OwnerTable The list view that owns the row.
	 * @return The constructed row widget.
	 */
	TSharedRef<ITableRow> GenerateContentRow(
		TSharedPtr<FLogFlowViewerLine> Line,
		const TSharedRef<STableViewBase>& OwnerTable);

	/**
	 * Returns the background color for a content row based on severity.
	 * 
	 * @param Severity The severity of the line.
	 * @return FSlateColor for the row background.
	 */
	static FSlateColor GetContentRowColor(ELogFlowSeverity Severity);
	
	/** Parsed lines from the currently loaded session file. */
	TArray<TSharedPtr<FLogFlowViewerLine>> ViewerLines;
	
	/** The content list view widget. */
	TSharedPtr<SListView<TSharedPtr<FLogFlowViewerLine>>> ContentListView;
};