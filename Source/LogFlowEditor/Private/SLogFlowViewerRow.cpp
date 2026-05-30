#include "SLogFlowViewerRow.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"

#define LOCTEXT_NAMESPACE "SLogFlowViewerRow"

void SLogFlowViewerRow::Construct(
    const FArguments& InArgs,
    const TSharedRef<STableViewBase>& InOwnerTable)
{
    Line           = InArgs._Line;
    OnCopyRequested = InArgs._OnCopyRequested;

    STableRow<TSharedPtr<FLogFlowViewerLine>>::Construct(
        STableRow<TSharedPtr<FLogFlowViewerLine>>::FArguments()
        .Padding(FMargin(4.0f, 1.0f)),
        InOwnerTable
    );

    ChildSlot
    [
        SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(InArgs._RowColor)
        .Padding(FMargin(2.0f, 1.0f))
        [
            SNew(STextBlock)
            .Text(Line.IsValid()
                ? FText::FromString(Line->RawText)
                : FText::GetEmpty())
            .Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
            .ColorAndOpacity(FSlateColor(FLinearColor::White))
        ]
    ];
}

FReply SLogFlowViewerRow::OnMouseButtonUp(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        FMenuBuilder MenuBuilder(true, nullptr);

        MenuBuilder.AddMenuEntry(
            LOCTEXT("CopyLine", "Copy"),
            LOCTEXT("CopyLineTooltip", "Copy this line to the clipboard."),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([this]()
            {
                OnCopyRequested.ExecuteIfBound();
            }))
        );

        FSlateApplication::Get().PushMenu(
            AsShared(),
            FWidgetPath(),
            MenuBuilder.MakeWidget(),
            FSlateApplication::Get().GetCursorPos(),
            FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
        );

        return FReply::Handled();
    }

    return STableRow<TSharedPtr<FLogFlowViewerLine>>::OnMouseButtonUp(
        MyGeometry, MouseEvent);
}

#undef LOCTEXT_NAMESPACE