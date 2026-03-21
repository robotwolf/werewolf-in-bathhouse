#include "MagicCombatAIController.h"

#include "Components/StateTreeAIComponent.h"
#include "MagicCasterCharacter.h"
#include "MagicSpellPickup.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "StateTree.h"

AMagicCombatAIController::AMagicCombatAIController()
{
    CombatFaction = ECombatFaction::Magic;
    bAttackPlayers = false;
    bAttackSameFaction = false;
    bAttackOtherCombatFactions = true;

    StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
    StateTreeAIComponent->SetStartLogicAutomatically(false);
    BrainComponent = StateTreeAIComponent;

    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*AIPerception);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 3200.0f;
    SightConfig->LoseSightRadius = 3600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 75.0f;
    SightConfig->SetMaxAge(2.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 600.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

    AIPerception->ConfigureSense(*SightConfig);
    AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AMagicCombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    AssignDefaultStateTree();
    if (StateTreeAIComponent)
    {
        StateTreeAIComponent->RestartLogic();
    }
}

void AMagicCombatAIController::OnUnPossess()
{
    if (StateTreeAIComponent)
    {
        StateTreeAIComponent->StopLogic(TEXT("Magic pawn unpossessed"));
    }

    ClearFocus(EAIFocusPriority::Gameplay);
    StopMovement();
    Super::OnUnPossess();
}

AMagicCasterCharacter* AMagicCombatAIController::GetMagicCasterPawn() const
{
    return Cast<AMagicCasterCharacter>(GetPawn());
}

bool AMagicCombatAIController::HasSpellEquipped() const
{
    const AMagicCasterCharacter* MagicPawn = GetMagicCasterPawn();
    return MagicPawn && MagicPawn->HasSpellEquipped();
}

bool AMagicCombatAIController::IsTargetWithinDesiredRange(const AActor* Target) const
{
    const APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn || !IsValid(Target))
    {
        return false;
    }

    return FVector::DistSquared(ControlledPawn->GetActorLocation(), Target->GetActorLocation()) <= FMath::Square(DesiredRange);
}

bool AMagicCombatAIController::TryClaimSpellPickup(AMagicSpellPickup* Pickup)
{
    AMagicCasterCharacter* MagicPawn = GetMagicCasterPawn();
    if (!MagicPawn || !IsValid(Pickup))
    {
        return false;
    }

    const float DistanceToPickup = FVector::Dist(MagicPawn->GetActorLocation(), Pickup->GetActorLocation());
    if (DistanceToPickup > PickupClaimDistance)
    {
        return false;
    }

    return Pickup->TryGiveToCaster(MagicPawn);
}

void AMagicCombatAIController::BeginSeekSpellPickup(AMagicSpellPickup* Pickup)
{
    if (!IsValid(Pickup))
    {
        return;
    }

    SetFocus(Pickup);
    MoveToActor(Pickup, PickupAcceptanceRadius, true, true, true);
}

void AMagicCombatAIController::BeginChaseTarget(AActor* Target)
{
    if (!IsValid(Target))
    {
        return;
    }

    SetFocus(Target);
    MoveToActor(Target, AttackRange * 0.35f, true, true, true);
}

void AMagicCombatAIController::BeginCastTarget(AActor* Target)
{
    AMagicCasterCharacter* MagicPawn = GetMagicCasterPawn();
    if (!MagicPawn)
    {
        return;
    }

    if (IsValid(Target))
    {
        SetFocus(Target);
    }

    StopMovement();
    MagicPawn->TryCastCurrentSpell();
}

void AMagicCombatAIController::EnterIdleState()
{
    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
}

AActor* AMagicCombatAIController::FindBestHostileTarget() const
{
    if (!AIPerception || !GetPawn())
    {
        return nullptr;
    }

    TArray<AActor*> PerceivedActors;
    AIPerception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    AActor* BestTarget = nullptr;
    float BestDistanceSq = TNumericLimits<float>::Max();
    const FVector Origin = GetPawn()->GetActorLocation();

    for (AActor* Actor : PerceivedActors)
    {
        if (!Actor || Actor == GetPawn() || GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile)
        {
            continue;
        }

        const float DistanceSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
        if (DistanceSq < BestDistanceSq)
        {
            BestDistanceSq = DistanceSq;
            BestTarget = Actor;
        }
    }

    return BestTarget;
}

AMagicSpellPickup* AMagicCombatAIController::FindBestSpellPickup() const
{
    if (!GetPawn() || !GetWorld())
    {
        return nullptr;
    }

    TArray<AActor*> PickupActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagicSpellPickup::StaticClass(), PickupActors);

    AMagicSpellPickup* BestPickup = nullptr;
    float BestDistanceSq = TNumericLimits<float>::Max();
    const FVector Origin = GetPawn()->GetActorLocation();
    const float MaxDistanceSq = FMath::Square(PickupSearchRadius);

    for (AActor* Actor : PickupActors)
    {
        AMagicSpellPickup* Pickup = Cast<AMagicSpellPickup>(Actor);
        if (!Pickup || !IsValid(Pickup) || !*Pickup->SpellWeaponClass)
        {
            continue;
        }

        const float DistanceSq = FVector::DistSquared(Origin, Pickup->GetActorLocation());
        if (DistanceSq > MaxDistanceSq || DistanceSq >= BestDistanceSq)
        {
            continue;
        }

        BestDistanceSq = DistanceSq;
        BestPickup = Pickup;
    }

    return BestPickup;
}

float AMagicCombatAIController::GetStateTreeTickInterval() const
{
    return StateTreeTickInterval;
}

void AMagicCombatAIController::AssignDefaultStateTree()
{
    if (!StateTreeAIComponent)
    {
        return;
    }

    static const TCHAR* StateTreePath = TEXT("/Game/Magic/Blueprints/AI/ST_MagicNPC.ST_MagicNPC");
    if (UStateTree* DefaultStateTree = LoadObject<UStateTree>(nullptr, StateTreePath))
    {
        StateTreeAIComponent->SetStateTree(DefaultStateTree);
    }
}
