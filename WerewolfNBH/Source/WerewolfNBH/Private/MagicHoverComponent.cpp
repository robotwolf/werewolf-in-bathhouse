#include "MagicHoverComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

UMagicHoverComponent::UMagicHoverComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UMagicHoverComponent::BeginPlay()
{
    Super::BeginPlay();

    if (const ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner()))
    {
        CachedMesh = CharacterOwner->GetMesh();
    }

    if (CachedMesh)
    {
        BaseRelativeLocation = CachedMesh->GetRelativeLocation();
    }
}

void UMagicHoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!CachedMesh)
    {
        return;
    }

    RunningTime += DeltaTime;
    const float BobOffset = FMath::Sin(RunningTime * HoverBobFrequency * 2.0f * PI) * HoverBobAmplitude;
    FVector UpdatedLocation = BaseRelativeLocation;
    UpdatedLocation.Z += HoverOffset + BobOffset;
    CachedMesh->SetRelativeLocation(UpdatedLocation);
}
