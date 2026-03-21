#pragma once

#include "CoreMinimal.h"
#include "CombatFactionAIController.h"
#include "MagicCombatAIController.generated.h"

UCLASS(Blueprintable)
class WEREWOLFNBH_API AMagicCombatAIController : public ACombatFactionAIController
{
    GENERATED_BODY()

public:
    AMagicCombatAIController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    class AMagicCasterCharacter* GetMagicCasterPawn() const;

    bool HasSpellEquipped() const;
    AActor* FindBestHostileTarget() const;
    class AMagicSpellPickup* FindBestSpellPickup() const;
    bool IsTargetWithinDesiredRange(const AActor* Target) const;
    bool TryClaimSpellPickup(class AMagicSpellPickup* Pickup);
    void BeginSeekSpellPickup(class AMagicSpellPickup* Pickup);
    void BeginChaseTarget(AActor* Target);
    void BeginCastTarget(AActor* Target);
    void EnterIdleState();
    float GetStateTreeTickInterval() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<class UAIPerceptionComponent> AIPerception = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<class UAISenseConfig_Sight> SightConfig = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<class UStateTreeAIComponent> StateTreeAIComponent = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float AttackRange = 1200.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float DesiredRange = 850.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float StateTreeTickInterval = 0.2f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float PickupSearchRadius = 5000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float PickupAcceptanceRadius = 30.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    float PickupClaimDistance = 220.0f;

private:
    void AssignDefaultStateTree();
};
