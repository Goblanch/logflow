#include "SLogFlowEntryRow.h"
#include "LogFlowSeverity.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "SlateOptMacros.h"

// -- Static color definitions ---------------------------------------------------------------------

const FLinearColor SLogFlowEntryRow::BackgroundLog(0.0f, 0.0f, 0.0f, 0.0f);
const FLinearColor SLogFlowEntryRow::BackgroundWarning(0.25f, 0.18f, 0.0f, 0.6f);
const FLinearColor SLogFlowEntryRow::BackgroundError(0.3f, 0.0f, 0.0f, 0.6f);

const FLinearColor SLogFlowEntryRow::SeverityColorLog(0.4f, 0.6f, 1.0f, 1.0f);
const FLinearColor SLogFlowEntryRow::SeverityColorWarning(1.0f, 0.75f, 0.0f, 1.0f);
const FLinearColor SLogFlowEntryRow::SeverityColorError(1.0f, 0.2f, 0.2f, 1.0f);

const FLinearColor SLogFlowEntryRow::TagColorDefault(0.7f, 0.7f, 0.7f, 1.0f);

// -- Construction ------------------------------------------------------------------------------------

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogFlowEntryRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
{
	Entry = InArgs._Entry;
	Settings = InArgs._Settings;
	
	STableRow<TSharedPtr<FLogFlowEntry>>::Construct(
		STableRow<TSharedPtr<FLogFlowEntry>>::FArguments()
		.Padding(FMargin(0.0f))
		.ShowSelection(true),
		InOwnerTable
	);
	
	 // Row content
    ChildSlot
    [
        SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
        .ColorAndOpacity(FLinearColor::White)
        .Padding(FMargin(0.0f))
        [
            SNew(SHorizontalBox)

            // ── Severity color indicator ──────────────────────────────────
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Fill)
            [
                SNew(SBox)
                .WidthOverride(4.0f)
                [
                    SNew(SBorder)
                    .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                    .BorderBackgroundColor(
                        TAttribute<FSlateColor>::Create(
                            TAttribute<FSlateColor>::FGetter::CreateSP(
                                this, &SLogFlowEntryRow::GetSeverityColor)))
                ]
            ]

            // ── Row background + content ──────────────────────────────────
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(
                    TAttribute<FSlateColor>::Create(
                        TAttribute<FSlateColor>::FGetter::CreateSP(
                            this, &SLogFlowEntryRow::GetRowBackgroundColor)))
                .Padding(FMargin(6.0f, 2.0f))
                [
                    SNew(SHorizontalBox)

                    // Severity label
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    .Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))
                    [
                        SNew(SBox)
                        .WidthOverride(32.0f)
                        [
                            SNew(STextBlock)
                            .Text(TAttribute<FText>::Create(
                                TAttribute<FText>::FGetter::CreateSP(
                                    this, &SLogFlowEntryRow::GetSeverityText)))
                            .ColorAndOpacity(
                                TAttribute<FSlateColor>::Create(
                                    TAttribute<FSlateColor>::FGetter::CreateSP(
                                        this, &SLogFlowEntryRow::GetSeverityColor)))
                            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
                        ]
                    ]

                    // Timestamp
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    .Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
                    [
                        SNew(SBox)
                        .WidthOverride(90.0f)
                        [
                            SNew(STextBlock)
                            .Text(TAttribute<FText>::Create(
                                TAttribute<FText>::FGetter::CreateSP(
                                    this, &SLogFlowEntryRow::GetFormattedTimestamp)))
                            .ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f)))
                            .Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
                        ]
                    ]

                    // Tag
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    .Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
                    [
                        SNew(SBox)
                        .WidthOverride(80.0f)
                        [
                            SNew(STextBlock)
                            .Text(Entry.IsValid()
                                ? (Entry->Tag.IsNone()
                                    ? FText::FromString(TEXT("-"))
                                    : FText::FromName(Entry->Tag))
                                : FText::GetEmpty())
                            .ColorAndOpacity(
                                TAttribute<FSlateColor>::Create(
                                    TAttribute<FSlateColor>::FGetter::CreateSP(
                                        this, &SLogFlowEntryRow::GetTagColor)))
                            .Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
                        ]
                    ]

                    // Message
                    + SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(Entry.IsValid()
                            ? FText::FromString(Entry->Message)
                            : FText::GetEmpty())
                        .ColorAndOpacity(FSlateColor(FLinearColor::White))
                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
                    ]
                ]
            ]
        ]
    ];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

// -- Private helpers -------------------------------------------------------------------------------------------

FSlateColor SLogFlowEntryRow::GetRowBackgroundColor() const
{
    if (!Entry.IsValid())
    {
        return FSlateColor(BackgroundLog);
    }
    
    switch (Entry->Severity)
    {
        case ELogFlowSeverity::Warning: return FSlateColor(BackgroundWarning);
        case ELogFlowSeverity::Error:   return FSlateColor(BackgroundError);
        default:                        return FSlateColor(BackgroundLog);
    }
}

FSlateColor SLogFlowEntryRow::GetSeverityColor() const
{
    if (!Entry.IsValid())
    {
        return FSlateColor(SeverityColorLog);
    }

    switch (Entry->Severity)
    {
        case ELogFlowSeverity::Warning: return FSlateColor(SeverityColorWarning);
        case ELogFlowSeverity::Error:   return FSlateColor(SeverityColorError);
        default:                        return FSlateColor(SeverityColorLog);
    }
}

FSlateColor SLogFlowEntryRow::GetTagColor() const
{
    if (!Entry.IsValid() || Entry->Tag.IsNone())
    {
        return FSlateColor(TagColorDefault);
    }

    // Look up the configured color for this tag
    for (const FLogFlowTagConfig& TagConfig : Settings.TagColors)
    {
        if (TagConfig.TagName == Entry->Tag)
        {
            return FSlateColor(TagConfig.Color);
        }
    }

    return FSlateColor(TagColorDefault);
}

FText SLogFlowEntryRow::GetFormattedTimestamp() const
{
    if (!Entry.IsValid())
    {
        return FText::FromString(TEXT("00:00:00.000"));
    }

    if (Settings.TimestampMode == ELogFlowTimestampMode::SystemTime)
    {
        return FText::FromString(FString::Printf(
            TEXT("%02d:%02d:%02d.%03d"),
            Entry->SystemTime.GetHour(),
            Entry->SystemTime.GetMinute(),
            Entry->SystemTime.GetSecond(),
            Entry->SystemTime.GetMillisecond()
        ));
    }

    // Session time mode
    const int32 TotalSeconds = static_cast<int32>(Entry->Timestamp.GetTotalSeconds());
    const int32 Hours        = TotalSeconds / 3600;
    const int32 Minutes      = (TotalSeconds % 3600) / 60;
    const int32 Seconds      = TotalSeconds % 60;
    const int32 Milliseconds = Entry->Timestamp.GetFractionMilli();

    return FText::FromString(FString::Printf(
        TEXT("%02d:%02d:%02d.%03d"),
        Hours, Minutes, Seconds, Milliseconds
    ));
}

FText SLogFlowEntryRow::GetSeverityText() const
{
    if (!Entry.IsValid())
    {
        return FText::FromString(TEXT("LOG"));
    }

    switch (Entry->Severity)
    {
        case ELogFlowSeverity::Warning: return FText::FromString(TEXT("WRN"));
        case ELogFlowSeverity::Error:   return FText::FromString(TEXT("ERR"));
        default:                        return FText::FromString(TEXT("LOG"));
    }
}
