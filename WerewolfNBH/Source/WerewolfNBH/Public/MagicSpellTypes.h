#pragma once

#include "CoreMinimal.h"
#include "MagicSpellTypes.generated.h"

class AMagicHazardField;

USTRUCT(BlueprintType)
struct FMagicProjectileConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    FLinearColor SpellColor = FLinearColor(0.35f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float ProjectileSpeed = 2400.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float ProjectileLifeSeconds = 4.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float ProjectileScale = 0.2f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float DirectDamage = 20.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float ImpactForce = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float ImpactRadius = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float LightIntensity = 1200.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float LightRadius = 250.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float LingeringDuration = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float LingeringDamagePerTick = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float LingeringForce = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float LingeringRadius = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float LingeringTickInterval = 0.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    TSubclassOf<AMagicHazardField> LingeringFieldClass;
};
