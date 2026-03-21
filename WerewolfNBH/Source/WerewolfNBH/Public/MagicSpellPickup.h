#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicSpellWeaponBase.h"
#include "MagicSpellPickup.generated.h"

UCLASS(Blueprintable)
class WEREWOLFNBH_API AMagicSpellPickup : public AActor
{
    GENERATED_BODY()

public:
    AMagicSpellPickup();

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Magic")
    TSubclassOf<AMagicSpellWeaponBase> SpellWeaponClass;

    UFUNCTION(BlueprintCallable, Category = "Magic")
    bool TryGiveToCaster(class AMagicCasterCharacter* Caster);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class USceneComponent> SceneRoot = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class USphereComponent> SphereCollision = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class UStaticMeshComponent> DisplayMesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class UStaticMeshComponent> BasePlate = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic")
    float RotationSpeed = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic")
    float BobAmplitude = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic")
    float BobFrequency = 1.25f;

private:
    UFUNCTION()
    void HandleOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    FVector DisplayStartLocation = FVector::ZeroVector;
    float RunningTime = 0.0f;
};
