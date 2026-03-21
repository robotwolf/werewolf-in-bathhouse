#include "CombatFactionAIController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Sight.h"

ACombatFactionAIController::ACombatFactionAIController()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACombatFactionAIController::BeginPlay()
{
    Super::BeginPlay();

    ApplyFactionTagToPawn();
    ConfigurePerceptionAffiliations();
}

void ACombatFactionAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ApplyFactionTagToPawn();
    ConfigurePerceptionAffiliations();
}

FGenericTeamId ACombatFactionAIController::GetGenericTeamId() const
{
    return CombatFactionUtilities::ToTeamId(GetEffectiveCombatFaction());
}

ETeamAttitude::Type ACombatFactionAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
    const ECombatFaction EffectiveFaction = GetEffectiveCombatFaction();

    const ECombatFaction OtherFaction = ResolveFactionForActor(&Other);
    if (OtherFaction == ECombatFaction::None)
    {
        return ETeamAttitude::Neutral;
    }

    if (OtherFaction == ECombatFaction::Player)
    {
        return bAttackPlayers ? ETeamAttitude::Hostile : ETeamAttitude::Friendly;
    }

    if (OtherFaction == EffectiveFaction)
    {
        return bAttackSameFaction ? ETeamAttitude::Hostile : ETeamAttitude::Friendly;
    }

    return bAttackOtherCombatFactions ? ETeamAttitude::Hostile : ETeamAttitude::Friendly;
}

ECombatFaction ACombatFactionAIController::GetResolvedFactionForActor(const AActor* Actor) const
{
    return ResolveFactionForActor(Actor);
}

void ACombatFactionAIController::ConfigurePerceptionAffiliations() const
{
    if (UAIPerceptionComponent* LocalPerceptionComponent = FindComponentByClass<UAIPerceptionComponent>())
    {
        if (UAISenseConfig_Sight* Sight = LocalPerceptionComponent->GetSenseConfig<UAISenseConfig_Sight>())
        {
            Sight->DetectionByAffiliation.bDetectEnemies = true;
            Sight->DetectionByAffiliation.bDetectFriendlies = false;
            Sight->DetectionByAffiliation.bDetectNeutrals = false;
            LocalPerceptionComponent->ConfigureSense(*Sight);
        }

        LocalPerceptionComponent->RequestStimuliListenerUpdate();
    }
}

void ACombatFactionAIController::ApplyFactionTagToPawn() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn || CombatFaction == ECombatFaction::None)
    {
        return;
    }

    ControlledPawn->Tags.Remove(CombatFactionTags::PlayerTag);
    ControlledPawn->Tags.Remove(CombatFactionTags::ShooterTag);
    ControlledPawn->Tags.Remove(CombatFactionTags::MagicTag);

    if (const FName FactionTag = CombatFactionUtilities::ToTag(CombatFaction); !FactionTag.IsNone())
    {
        ControlledPawn->Tags.AddUnique(FactionTag);
    }
}

ECombatFaction ACombatFactionAIController::GetEffectiveCombatFaction() const
{
    if (CombatFaction != ECombatFaction::None)
    {
        return CombatFaction;
    }

    return ResolveFactionFromTags(GetPawn());
}

ECombatFaction ACombatFactionAIController::ResolveFactionFromTags(const AActor* Actor)
{
    if (!Actor)
    {
        return ECombatFaction::None;
    }

    if (Actor->Tags.Contains(CombatFactionTags::PlayerTag))
    {
        return ECombatFaction::Player;
    }
    if (Actor->Tags.Contains(CombatFactionTags::ShooterTag))
    {
        return ECombatFaction::Shooter;
    }
    if (Actor->Tags.Contains(CombatFactionTags::MagicTag))
    {
        return ECombatFaction::Magic;
    }

    return ECombatFaction::None;
}

ECombatFaction ACombatFactionAIController::ResolveFactionForActor(const AActor* Actor) const
{
    if (!Actor)
    {
        return ECombatFaction::None;
    }

    if (const APawn* OtherPawn = Cast<APawn>(Actor))
    {
        if (OtherPawn->IsPlayerControlled())
        {
            return ECombatFaction::Player;
        }

        if (const ECombatFaction TaggedFaction = ResolveFactionFromTags(OtherPawn); TaggedFaction != ECombatFaction::None)
        {
            return TaggedFaction;
        }

        if (const AController* OtherController = OtherPawn->GetController())
        {
            if (OtherController == this)
            {
                return GetEffectiveCombatFaction();
            }

            if (const ACombatFactionAIController* OtherFactionController = Cast<ACombatFactionAIController>(OtherController))
            {
                return OtherFactionController->GetEffectiveCombatFaction();
            }

            if (const IGenericTeamAgentInterface* TeamAgent = Cast<const IGenericTeamAgentInterface>(OtherController))
            {
                return CombatFactionUtilities::FromTeamId(TeamAgent->GetGenericTeamId());
            }
        }
    }

    return ResolveFactionFromTags(Actor);
}
