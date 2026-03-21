#pragma once

#include "Components/ActorComponent.h"
#include "MagicHoverComponent.generated.h"

UCLASS(ClassGroup = (Magic), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class WEREWOLFNBH_API UMagicHoverComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMagicHoverComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover")
    float HoverOffset = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover")
    float HoverBobAmplitude = 7.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover")
    float HoverBobFrequency = 1.5f;

private:
    TObjectPtr<class USkeletalMeshComponent> CachedMesh = nullptr;
    FVector BaseRelativeLocation = FVector::ZeroVector;
    float RunningTime = 0.0f;
};
