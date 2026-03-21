#include "WerewolfStateBillboardComponent.h"

#include "Engine/World.h"
#include "Math/RotationMatrix.h"

UWerewolfStateBillboardComponent::UWerewolfStateBillboardComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bTickInEditor = true;
    CompactSeparatorText = FText::FromString(TEXT(" | "));

    SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetWorldSize(22.0f);
    SetHiddenInGame(false);
}

void UWerewolfStateBillboardComponent::OnRegister()
{
    Super::OnRegister();
    RefreshDisplay();
}

void UWerewolfStateBillboardComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateBillboarding();
}

void UWerewolfStateBillboardComponent::SetDisplayLines(
    FText InHeaderText,
    FText InStateText,
    FText InDetailText,
    FText InStatusText)
{
    HeaderText = MoveTemp(InHeaderText);
    StateText = MoveTemp(InStateText);
    DetailText = MoveTemp(InDetailText);
    StatusText = MoveTemp(InStatusText);
    PresentationMode = EWerewolfBillboardPresentationMode::DebugStack;
    RefreshDisplay();
}

void UWerewolfStateBillboardComponent::SetConversationDisplay(
    FText InSpeakerText,
    FText InConversationText,
    FText InContextText)
{
    HeaderText = MoveTemp(InSpeakerText);
    StateText = MoveTemp(InConversationText);
    DetailText = MoveTemp(InContextText);
    StatusText = FText::GetEmpty();
    PresentationMode = EWerewolfBillboardPresentationMode::OverheadCompact;
    RefreshDisplay();
}

void UWerewolfStateBillboardComponent::SetPresentationMode(EWerewolfBillboardPresentationMode InPresentationMode)
{
    PresentationMode = InPresentationMode;
    RefreshDisplay();
}

void UWerewolfStateBillboardComponent::RefreshDisplay()
{
    const FString DisplayString = BuildDisplayString();
    SetText(FText::FromString(DisplayString));

    const bool bHasContent = !DisplayString.IsEmpty();
    if (bHideWhenEmpty)
    {
        SetVisibility(bHasContent);
        SetHiddenInGame(!bHasContent);
    }
}

void UWerewolfStateBillboardComponent::UpdateBillboarding()
{
    if (!bBillboardToView || !GetWorld() || GetWorld()->ViewLocationsRenderedLastFrame.IsEmpty())
    {
        return;
    }

    FVector ToView = GetWorld()->ViewLocationsRenderedLastFrame[0] - GetComponentLocation();
    ToView.Z = 0.0f;
    if (ToView.IsNearlyZero())
    {
        return;
    }

    const FRotator FacingRotation = FRotationMatrix::MakeFromX(ToView).Rotator();
    SetWorldRotation(FRotator(0.0f, FacingRotation.Yaw, 0.0f));
}

FString UWerewolfStateBillboardComponent::BuildDisplayString() const
{
    TArray<FString> Lines;

    auto AppendLine = [&Lines](const FText& SourceText)
    {
        const FString StringValue = SourceText.ToString();
        if (!StringValue.IsEmpty())
        {
            Lines.Add(StringValue);
        }
    };

    if (PresentationMode == EWerewolfBillboardPresentationMode::OverheadCompact)
    {
        AppendLine(HeaderText);

        TArray<FString> CompactSegments;
        auto AppendSegment = [&CompactSegments](const FText& SourceText)
        {
            const FString StringValue = SourceText.ToString();
            if (!StringValue.IsEmpty())
            {
                CompactSegments.Add(StringValue);
            }
        };

        AppendSegment(StateText);
        AppendSegment(DetailText);
        AppendSegment(StatusText);

        if (CompactSegments.Num() > 0)
        {
            Lines.Add(FString::Join(CompactSegments, *CompactSeparatorText.ToString()));
        }

        return FString::Join(Lines, TEXT("\n"));
    }

    AppendLine(HeaderText);
    AppendLine(StateText);
    AppendLine(DetailText);
    AppendLine(StatusText);

    return FString::Join(Lines, TEXT("\n"));
}
