#include "RoomSignageComponent.h"

#include "Components/BillboardComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "StagehandBillboardLabelComponent.h"

URoomSignageComponent::URoomSignageComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bTickInEditor = true;
}

void URoomSignageComponent::OnRegister()
{
    Super::OnRegister();
    EnsureHelperComponents();
    ApplyVisibility();
}

void URoomSignageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateBillboarding();
}

void URoomSignageComponent::UpdateFromRoom(const FText& DisplayText, const FVector& BoundsCenter, const FVector& BoundsExtent)
{
    EnsureHelperComponents();
    if (!MarkerBillboard || !InteriorLabel || !ExteriorRoofLabel || !MarkerLight)
    {
        return;
    }

    CachedBoundsCenter = BoundsCenter;
    CachedBoundsExtent = BoundsExtent;

    const FVector InteriorLocation = CachedBoundsCenter + InteriorLabelOffset;
    const FVector RoofLocation = FVector(
        CachedBoundsCenter.X + ExteriorRoofLabelOffset.X,
        CachedBoundsCenter.Y + ExteriorRoofLabelOffset.Y,
        CachedBoundsCenter.Z + CachedBoundsExtent.Z + ExteriorRoofLabelOffset.Z);
    const FVector InteriorWorldLocation = GetComponentTransform().TransformPosition(InteriorLocation);
    const FVector RoofWorldLocation = GetComponentTransform().TransformPosition(RoofLocation);

    MarkerBillboard->SetRelativeLocation(InteriorLocation);
    MarkerLight->SetRelativeLocation(InteriorLocation);
    MarkerLight->SetIntensity(MarkerLightIntensity);
    MarkerLight->SetAttenuationRadius(MarkerLightRadius);
    MarkerLight->SetLightColor(MarkerLightColor);

    InteriorLabel->LabelWorldSize = LabelWorldSize * 0.45f;
    InteriorLabel->CardWidth = FMath::Max(64.0f, LabelWorldSize * 2.2f);
    InteriorLabel->CardHeight = FMath::Max(18.0f, LabelWorldSize * 0.55f);
    InteriorLabel->TextColor = LabelColor;
    InteriorLabel->UpdateLabel(InteriorWorldLocation, DisplayText.ToString(), FLinearColor(0.18f, 0.46f, 0.95f, 1.0f));

    ExteriorRoofLabel->LabelWorldSize = LabelWorldSize * 0.42f;
    ExteriorRoofLabel->CardWidth = FMath::Max(64.0f, LabelWorldSize * 2.4f);
    ExteriorRoofLabel->CardHeight = FMath::Max(18.0f, LabelWorldSize * 0.55f);
    ExteriorRoofLabel->TextColor = LabelColor;
    ExteriorRoofLabel->UpdateLabel(RoofWorldLocation, DisplayText.ToString(), FLinearColor(0.14f, 0.22f, 0.52f, 1.0f));

    ApplyVisibility();
    UpdateBillboarding();
}

void URoomSignageComponent::ApplyVisibility()
{
    EnsureHelperComponents();
    if (!MarkerBillboard || !InteriorLabel || !ExteriorRoofLabel || !MarkerLight)
    {
        return;
    }

    MarkerBillboard->SetVisibility(bShowMarkerBillboard);
    MarkerBillboard->SetHiddenInGame(bHideHelpersInGame || !bShowMarkerBillboard);

    InteriorLabel->bHideHelpersInGame = bHideHelpersInGame;
    InteriorLabel->SetLabelVisible(bShowInteriorLabel);

    ExteriorRoofLabel->bHideHelpersInGame = bHideHelpersInGame;
    ExteriorRoofLabel->SetLabelVisible(bShowExteriorRoofLabel);

    MarkerLight->SetVisibility(bShowMarkerLight);
    MarkerLight->SetHiddenInGame(bHideHelpersInGame || !bShowMarkerLight);
}

void URoomSignageComponent::UpdateBillboarding()
{
    EnsureHelperComponents();
    if (!InteriorLabel || !ExteriorRoofLabel)
    {
        return;
    }

    InteriorLabel->bBillboardToView = bBillboardLabelsToView;
    ExteriorRoofLabel->bBillboardToView = bBillboardLabelsToView;
    InteriorLabel->UpdateBillboarding();
    ExteriorRoofLabel->UpdateBillboarding();
}

void URoomSignageComponent::EnsureHelperComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner || IsTemplate())
    {
        return;
    }

    TInlineComponentArray<UTextRenderComponent*> LegacyTextComponents(Owner);
    for (UTextRenderComponent* LegacyLabel : LegacyTextComponents)
    {
        if (!LegacyLabel || LegacyLabel->GetAttachParent() != this)
        {
            continue;
        }

        const FName LegacyName = LegacyLabel->GetFName();
        if (LegacyName == TEXT("InteriorLabel") || LegacyName == TEXT("ExteriorRoofLabel"))
        {
            LegacyLabel->DestroyComponent();
        }
    }

    if (!MarkerBillboard)
    {
        MarkerBillboard = NewObject<UBillboardComponent>(Owner, TEXT("MarkerBillboard"));
        MarkerBillboard->SetupAttachment(this);
        MarkerBillboard->CreationMethod = EComponentCreationMethod::Instance;
        Owner->AddInstanceComponent(MarkerBillboard);
        MarkerBillboard->RegisterComponent();
    }

    if (!InteriorLabel)
    {
        InteriorLabel = NewObject<UStagehandBillboardLabelComponent>(Owner, TEXT("InteriorBillboardLabel"));
        InteriorLabel->SetupAttachment(this);
        InteriorLabel->CreationMethod = EComponentCreationMethod::Instance;
        Owner->AddInstanceComponent(InteriorLabel);
        InteriorLabel->RegisterComponent();
    }

    if (!ExteriorRoofLabel)
    {
        ExteriorRoofLabel = NewObject<UStagehandBillboardLabelComponent>(Owner, TEXT("ExteriorRoofBillboardLabel"));
        ExteriorRoofLabel->SetupAttachment(this);
        ExteriorRoofLabel->CreationMethod = EComponentCreationMethod::Instance;
        Owner->AddInstanceComponent(ExteriorRoofLabel);
        ExteriorRoofLabel->RegisterComponent();
    }

    if (!MarkerLight)
    {
        MarkerLight = NewObject<UPointLightComponent>(Owner, TEXT("MarkerLight"));
        MarkerLight->SetupAttachment(this);
        MarkerLight->CreationMethod = EComponentCreationMethod::Instance;
        Owner->AddInstanceComponent(MarkerLight);
        MarkerLight->RegisterComponent();
    }

    MarkerBillboard->SetHiddenInGame(bHideHelpersInGame);
    MarkerBillboard->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    InteriorLabel->bHideHelpersInGame = bHideHelpersInGame;
    ExteriorRoofLabel->bHideHelpersInGame = bHideHelpersInGame;

    MarkerLight->SetMobility(EComponentMobility::Movable);
    MarkerLight->SetCastShadows(false);
    MarkerLight->SetIntensityUnits(ELightUnits::Candelas);
}
