#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicSpellTypes.h"
#include "MagicProjectileBase.generated.h"

UCLASS()
class WEREWOLFNBH_API AMagicProjectileBase : public AActor
{
    GENERATED_BODY()

public:
    AMagicProjectileBase();

    virtual void BeginPlay() override;

    void InitializeFromConfig(const FMagicProjectileConfig& InConfig, AController* InInstigatorController);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class USphereComponent> CollisionSphere = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class UStaticMeshComponent> DisplayMesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class UPointLightComponent> PointLight = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class UProjectileMovementComponent> ProjectileMovement = nullptr;

private:
    UFUNCTION()
    void OnProjectileOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnProjectileHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit);

    void HandleImpact(const FHitResult& HitResult, AActor* OtherActor);
    void ApplyRadialImpulse(const FVector& Origin) const;
    void ApplyCharacterForce(const FVector& Origin) const;

    FMagicProjectileConfig Config;
    TWeakObjectPtr<AController> InstigatorController;
    bool bHasImpacted = false;
};
