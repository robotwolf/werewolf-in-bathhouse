#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicSpellTypes.h"
#include "MagicSpellWeaponBase.generated.h"

UCLASS(Blueprintable)
class WEREWOLFNBH_API AMagicSpellWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    AMagicSpellWeaponBase();

    UFUNCTION(BlueprintCallable, Category = "Magic")
    bool TryCast(class AMagicCasterCharacter* Caster);

    UFUNCTION(BlueprintPure, Category = "Magic")
    float GetCooldownRemaining(const UWorld* World) const;

    UFUNCTION(BlueprintPure, Category = "Magic")
    float GetCooldownAlpha(const UWorld* World) const;

    UFUNCTION(BlueprintPure, Category = "Magic")
    const FText& GetSpellDisplayName() const { return SpellDisplayName; }

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    FText SpellDisplayName = FText::FromString(TEXT("Spell"));

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class UAnimSequenceBase> CastAnimation = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float CastPlayRate = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float CastLeadTime = 0.2f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float CooldownSeconds = 0.6f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    TSubclassOf<class AMagicProjectileBase> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    FMagicProjectileConfig ProjectileConfig;

private:
    void FinishCast();

    TWeakObjectPtr<class AMagicCasterCharacter> PendingCaster;
    FTimerHandle CastTimerHandle;
    float LastCastWorldTime = -1000.0f;
};
