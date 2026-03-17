#include "RoomSignageComponent.h"

#include "Components/BillboardComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "Math/RotationMatrix.h"

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

    MarkerBillboard->SetRelativeLocation(InteriorLocation);
    InteriorLabel->SetRelativeLocation(InteriorLocation);
    MarkerLight->SetRelativeLocation(InteriorLocation);
    ExteriorRoofLabel->SetRelativeLocation(RoofLocation);

    InteriorLabel->SetText(DisplayText);
    ExteriorRoofLabel->SetText(DisplayText);
    InteriorLabel->SetWorldSize(LabelWorldSize);
    ExteriorRoofLabel->SetWorldSize(LabelWorldSize);
    InteriorLabel->SetTextRenderColor(LabelColor);
    ExteriorRoofLabel->SetTextRenderColor(LabelColor);
    MarkerLight->SetIntensity(MarkerLightIntensity);
    MarkerLight->SetAttenuationRadius(MarkerLightRadius);
    MarkerLight->SetLightColor(MarkerLightColor);

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
    MarkerBillboard->SetHiddenInGame(!bShowMarkerBillboard);

    InteriorLabel->SetVisibility(bShowInteriorLabel);
    InteriorLabel->SetHiddenInGame(!bShowInteriorLabel);

    ExteriorRoofLabel->SetVisibility(bShowExteriorRoofLabel);
    ExteriorRoofLabel->SetHiddenInGame(!bShowExteriorRoofLabel);

    MarkerLight->SetVisibility(bShowMarkerLight);
    MarkerLight->SetHiddenInGame(!bShowMarkerLight);
}

void URoomSignageComponent::UpdateBillboarding()
{
    EnsureHelperComponents();
    if (!InteriorLabel || !ExteriorRoofLabel)
    {
        return;
    }

    if (!bBillboardLabelsToView || !GetWorld() || GetWorld()->ViewLocationsRenderedLastFrame.IsEmpty())
    {
        return;
    }

    const FVector ViewLocation = GetWorld()->ViewLocationsRenderedLastFrame[0];

    auto FaceView = [ViewLocation](UTextRenderComponent* Label)
    {
        if (!Label || !Label->IsVisible())
        {
            return;
        }

        FVector ToView = ViewLocation - Label->GetComponentLocation();
        ToView.Z = 0.0f;
        if (ToView.IsNearlyZero())
        {
            return;
        }

        const FRotator FacingRotation = FRotationMatrix::MakeFromX(ToView).Rotator();
        Label->SetWorldRotation(FRotator(0.0f, FacingRotation.Yaw, 0.0f));
    };

    FaceView(InteriorLabel);
    FaceView(ExteriorRoofLabel);
}

void URoomSignageComponent::EnsureHelperComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner || IsTemplate())
    {
        return;
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
        InteriorLabel = NewObject<UTextRenderComponent>(Owner, TEXT("InteriorLabel"));
        InteriorLabel->SetupAttachment(this);
        InteriorLabel->CreationMethod = EComponentCreationMethod::Instance;
        Owner->AddInstanceComponent(InteriorLabel);
        InteriorLabel->RegisterComponent();
    }

    if (!ExteriorRoofLabel)
    {
        ExteriorRoofLabel = NewObject<UTextRenderComponent>(Owner, TEXT("ExteriorRoofLabel"));
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

    MarkerBillboard->SetHiddenInGame(false);
    MarkerBillboard->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    InteriorLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    InteriorLabel->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    InteriorLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ExteriorRoofLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    ExteriorRoofLabel->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    ExteriorRoofLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    MarkerLight->SetMobility(EComponentMobility::Movable);
    MarkerLight->SetCastShadows(false);
    MarkerLight->SetIntensityUnits(ELightUnits::Candelas);
}
