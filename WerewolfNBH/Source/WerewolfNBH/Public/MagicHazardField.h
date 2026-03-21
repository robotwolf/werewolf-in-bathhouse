#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicSpellTypes.h"
#include "MagicHazardField.generated.h"

UCLASS()
class WEREWOLFNBH_API AMagicHazardField : public AActor
{
    GENERATED_BODY()

public:
    AMagicHazardField();

    virtual void BeginPlay() override;

    void InitializeFromConfig(const FMagicProjectileConfig& InConfig, AController* InInstigatorController);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class USphereComponent> DamageSphere = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class UPointLightComponent> PointLight = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float DefaultRadius = 250.0f;

private:
    void ApplyHazardTick();

    FMagicProjectileConfig Config;
    TWeakObjectPtr<AController> InstigatorController;
    FTimerHandle TickTimerHandle;
};
