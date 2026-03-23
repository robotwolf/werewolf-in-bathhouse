#include "StagehandBillboardLabelComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"
#include "UObject/ConstructorHelpers.h"

UStagehandBillboardLabelComponent::UStagehandBillboardLabelComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bTickInEditor = true;

    BackplateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackplateMesh"));
    BackplateMesh->SetupAttachment(this);
    BackplateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BackplateMesh->SetCastShadow(false);
    BackplateMesh->SetHiddenInGame(true);
    BackplateMesh->SetCanEverAffectNavigation(false);
    BackplateMesh->SetMobility(EComponentMobility::Movable);
    BackplateMesh->SetIsVisualizationComponent(true);

    CaptionText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CaptionText"));
    CaptionText->SetupAttachment(this);
    CaptionText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CaptionText->SetCastShadow(false);
    CaptionText->SetHiddenInGame(true);
    CaptionText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    CaptionText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    CaptionText->SetMobility(EComponentMobility::Movable);
    CaptionText->SetIsVisualizationComponent(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeFinder.Succeeded())
    {
        BackplateMesh->SetStaticMesh(CubeFinder.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicShapeMaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (BasicShapeMaterialFinder.Succeeded())
    {
        BackplateBaseMaterial = BasicShapeMaterialFinder.Object;
        BackplateMesh->SetMaterial(0, BackplateBaseMaterial);
    }
}

void UStagehandBillboardLabelComponent::OnRegister()
{
    Super::OnRegister();

    if (!IsTemplate() && BackplateMesh && BackplateBaseMaterial && !BackplateMaterial)
    {
        BackplateMaterial = UMaterialInstanceDynamic::Create(BackplateBaseMaterial, BackplateMesh);
        if (BackplateMaterial)
        {
            BackplateMesh->SetMaterial(0, BackplateMaterial);
        }
    }

    UpdateVisualStyle();
    ApplyVisibility();
    UpdateBillboarding();
}

void UStagehandBillboardLabelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateBillboarding();
}

void UStagehandBillboardLabelComponent::UpdateLabel(const FVector& WorldLocation, const FString& Text, const FLinearColor& InAccentColor)
{
    SetWorldLocation(WorldLocation);
    AccentColor = InAccentColor;
    if (CaptionText)
    {
        CaptionText->SetText(FText::FromString(Text));
    }

    UpdateVisualStyle();
    ApplyVisibility();
    UpdateBillboarding();
}

void UStagehandBillboardLabelComponent::SetLabelVisible(bool bInVisible)
{
    SetVisibility(bInVisible, true);
    ApplyVisibility();
}

void UStagehandBillboardLabelComponent::ApplyVisibility()
{
    const bool bLabelVisible = IsVisible();

    if (BackplateMesh)
    {
        BackplateMesh->SetVisibility(bLabelVisible);
        BackplateMesh->SetHiddenInGame(bHideHelpersInGame || !bLabelVisible);
    }

    if (CaptionText)
    {
        CaptionText->SetVisibility(bLabelVisible);
        CaptionText->SetHiddenInGame(bHideHelpersInGame || !bLabelVisible);
    }
}

void UStagehandBillboardLabelComponent::UpdateBillboarding()
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

void UStagehandBillboardLabelComponent::UpdateVisualStyle()
{
    if (BackplateMesh)
    {
        const FVector CardScale(0.02f, FMath::Max(8.0f, CardWidth) / 100.0f, FMath::Max(4.0f, CardHeight) / 100.0f);
        BackplateMesh->SetRelativeScale3D(CardScale);
    }

    if (CaptionText)
    {
        CaptionText->SetRelativeLocation(CaptionLocalOffset);
        CaptionText->SetWorldSize(LabelWorldSize);
        CaptionText->SetTextRenderColor(TextColor);
    }

    if (BackplateMaterial)
    {
        BackplateMaterial->SetVectorParameterValue(TEXT("Color"), AccentColor);
    }
}
