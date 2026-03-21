#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GenericTeamAgentInterface.h"
#include "CombatFactionTypes.h"
#include "CombatFactionAIController.generated.h"

UCLASS(Blueprintable)
class WEREWOLFNBH_API ACombatFactionAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatFactionAIController();

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    virtual FGenericTeamId GetGenericTeamId() const override;
    virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    ECombatFaction CombatFaction = ECombatFaction::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    bool bAttackPlayers = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    bool bAttackSameFaction = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    bool bAttackOtherCombatFactions = true;

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatFaction GetResolvedFactionForActor(const AActor* Actor) const;

    void ConfigurePerceptionAffiliations() const;
    void ApplyFactionTagToPawn() const;

private:
    ECombatFaction GetEffectiveCombatFaction() const;
    static ECombatFaction ResolveFactionFromTags(const AActor* Actor);
    ECombatFaction ResolveFactionForActor(const AActor* Actor) const;
};
