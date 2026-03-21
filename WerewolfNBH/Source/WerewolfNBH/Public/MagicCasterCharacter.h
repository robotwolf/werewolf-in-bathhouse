#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MagicCasterCharacter.generated.h"

UCLASS(Blueprintable)
class WEREWOLFNBH_API AMagicCasterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AMagicCasterCharacter();

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "Magic")
    bool EquipSpell(TSubclassOf<class AMagicSpellWeaponBase> NewSpellClass);

    UFUNCTION(BlueprintPure, Category = "Magic")
    FText GetCurrentSpellName() const;

    UFUNCTION(BlueprintPure, Category = "Magic")
    float GetCurrentSpellCooldownRemaining() const;

    UFUNCTION(BlueprintPure, Category = "Magic")
    float GetCurrentSpellCooldownAlpha() const;

    UFUNCTION(BlueprintCallable, Category = "Magic")
    bool TryCastCurrentSpell();

    UFUNCTION(BlueprintPure, Category = "Magic")
    bool HasSpellEquipped() const;

    bool PlayCastSequence(class UAnimSequenceBase* Sequence, float PlayRate);
    FTransform GetSpellSpawnTransform() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<class USpringArmComponent> CameraBoom = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<class UCameraComponent> FollowCamera = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class UMagicHoverComponent> HoverComponent = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    TObjectPtr<class UAnimSequenceBase> IdleLevitationAnimation = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    FName SpellSocketName = TEXT("hand_r");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    FVector SpellSpawnOffset = FVector(16.0f, 0.0f, 0.0f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    TSubclassOf<class AMagicSpellWeaponBase> StartingSpellClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TSoftObjectPtr<class UInputMappingContext> DefaultInputMapping;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TSoftObjectPtr<class UInputMappingContext> MouseLookInputMapping;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TSoftObjectPtr<class UInputMappingContext> WeaponInputMapping;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TSoftObjectPtr<class UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TSoftObjectPtr<class UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TSoftObjectPtr<class UInputAction> JumpActionAsset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TSoftObjectPtr<class UInputAction> ShootAction;

private:
    void Move(const struct FInputActionValue& Value);
    void Look(const struct FInputActionValue& Value);
    void StartCast();
    void RestoreIdleAnimation();
    void AddMappingIfValid(const TSoftObjectPtr<class UInputMappingContext>& Mapping, int32 Priority) const;

    TObjectPtr<class AMagicSpellWeaponBase> CurrentSpell = nullptr;
    FTimerHandle IdleRestoreTimerHandle;
};
