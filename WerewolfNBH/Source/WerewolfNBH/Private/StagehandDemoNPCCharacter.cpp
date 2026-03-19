#include "StagehandDemoNPCCharacter.h"

#include "StagehandDemoAIController.h"
#include "Animation/AnimInstance.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "RoomGameplayMarkerLibrary.h"
#include "RoomGenerator.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "WerewolfStateBillboardComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogStagehandDemoNPC, Log, All);

namespace
{
    FString ToStateLabel(EStagehandDemoLoopState LoopState)
    {
        switch (LoopState)
        {
        case EStagehandDemoLoopState::WaitForLayout:
            return TEXT("WaitForLayout");
        case EStagehandDemoLoopState::SelectMarker:
            return TEXT("SelectMarker");
        case EStagehandDemoLoopState::MoveToMarker:
            return TEXT("MoveToMarker");
        case EStagehandDemoLoopState::PauseAtMarker:
            return TEXT("PauseAtMarker");
        case EStagehandDemoLoopState::Retry:
        default:
            return TEXT("Retry");
        }
    }
}

AStagehandDemoNPCCharacter::AStagehandDemoNPCCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    AIControllerClass = AStagehandDemoAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    bUseControllerRotationYaw = false;

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->InitCapsuleSize(42.0f, 96.0f);
    }

    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->bOrientRotationToMovement = true;
        MovementComponent->RotationRate = FRotator(0.0f, 420.0f, 0.0f);
        MovementComponent->MaxWalkSpeed = 190.0f;
    }

    DebugText = CreateDefaultSubobject<UWerewolfStateBillboardComponent>(TEXT("DebugText"));
    DebugText->SetupAttachment(GetRootComponent());
    DebugText->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
    DebugText->SetWorldSize(22.0f);
    DebugText->SetHiddenInGame(false);

    TargetArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("TargetArrow"));
    TargetArrow->SetupAttachment(GetRootComponent());
    TargetArrow->SetRelativeLocation(FVector(0.0f, 0.0f, 92.0f));
    TargetArrow->ArrowColor = FColor(32, 224, 255);
    TargetArrow->ArrowSize = 1.1f;
    TargetArrow->SetHiddenInGame(false);
    TargetArrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> QuinnMeshFinder(
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
    if (QuinnMeshFinder.Succeeded() && GetMesh())
    {
        GetMesh()->SetSkeletalMesh(QuinnMeshFinder.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    static ConstructorHelpers::FClassFinder<UAnimInstance> UnarmedAnimClassFinder(
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
    if (UnarmedAnimClassFinder.Succeeded() && GetMesh())
    {
        GetMesh()->SetAnimInstanceClass(UnarmedAnimClassFinder.Class);
    }
}

void AStagehandDemoNPCCharacter::BeginPlay()
{
    Super::BeginPlay();

    BindToDemoController();
    UpdateDebugText();

    if (bAutoStartOnBeginPlay)
    {
        StartBehaviorLoop();
    }
}

void AStagehandDemoNPCCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);

    if (AStagehandDemoAIController* DemoController = CachedDemoController.Get())
    {
        DemoController->OnStagehandMoveCompleted.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

void AStagehandDemoNPCCharacter::StartBehaviorLoop()
{
    LastFailureReason.Reset();
    CurrentSelection = FStagehandNPCMarkerSelection();
    CurrentMoveDestination = FVector::ZeroVector;
    SetLoopState(EStagehandDemoLoopState::WaitForLayout, TEXT("Starting marker-loop demo."));
    ScheduleBehavior(0.05f);
}

void AStagehandDemoNPCCharacter::StopBehaviorLoop()
{
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);

    if (AStagehandDemoAIController* DemoController = ResolveDemoController(false))
    {
        DemoController->StopMovement();
    }

    UpdateDebugText();
}

bool AStagehandDemoNPCCharacter::SelectNextMarker()
{
    const TArray<ARoomModuleBase*> Rooms = GatherGeneratorRooms();
    if (Rooms.IsEmpty())
    {
        CurrentMoveDestination = FVector::ZeroVector;
        LastFailureReason = TEXT("No generated rooms are available yet.");
        UpdateDebugText();
        return false;
    }

    FStagehandNPCMarkerSelection FirstValidSelection;
    const int32 AttemptCount = FMath::Clamp(MaxSelectionAttempts, 1, 12);
    const int32 LoopSeedBase = HashCombineFast(SelectionSeed, CompletedLoops * 7919 + 37);

    for (int32 AttemptIndex = 0; AttemptIndex < AttemptCount; ++AttemptIndex)
    {
        const FStagehandNPCMarkerSelection CandidateSelection = UStagehandSimulationLibrary::PickMarkerForNPCProfile(
            NPCProfile,
            Rooms,
            Phase,
            bTreatAsWerewolf,
            LoopSeedBase + (AttemptIndex * 101));

        if (!HasUsableSelection(CandidateSelection))
        {
            continue;
        }

        if (!FirstValidSelection.bFoundSelection)
        {
            FirstValidSelection = CandidateSelection;
        }

        if (!MatchesLastSelection(CandidateSelection))
        {
            CurrentSelection = CandidateSelection;
            LastFailureReason.Reset();
            UpdateDebugText();

            if (bDrawDebugMarker)
            {
                URoomGameplayMarkerLibrary::DrawDebugGameplayMarker(
                    this,
                    CurrentSelection.Marker,
                    NPCProfile ? NPCProfile->DebugColor : FLinearColor(0.15f, 0.85f, 1.0f, 1.0f),
                    DebugMarkerDuration,
                    DebugMarkerRadius);
            }

            return true;
        }
    }

    if (HasUsableSelection(FirstValidSelection))
    {
        CurrentSelection = FirstValidSelection;
        LastFailureReason = TEXT("Only the previous marker was available; reusing it.");
        UpdateDebugText();

        if (bDrawDebugMarker)
        {
            URoomGameplayMarkerLibrary::DrawDebugGameplayMarker(
                this,
                CurrentSelection.Marker,
                NPCProfile ? NPCProfile->DebugColor : FLinearColor(0.15f, 0.85f, 1.0f, 1.0f),
                DebugMarkerDuration,
                DebugMarkerRadius);
        }

        return true;
    }

    CurrentSelection = FStagehandNPCMarkerSelection();
    CurrentMoveDestination = FVector::ZeroVector;
    LastFailureReason = NPCProfile
        ? TEXT("No matching NPC marker could be selected.")
        : TEXT("No NPC profile is assigned.");
    UpdateDebugText();
    return false;
}

TArray<ARoomModuleBase*> AStagehandDemoNPCCharacter::GatherGeneratorRooms() const
{
    TArray<ARoomModuleBase*> Rooms;
    if (!TargetGenerator)
    {
        return Rooms;
    }

    for (ARoomModuleBase* Room : TargetGenerator->SpawnedRooms)
    {
        if (Room)
        {
            Rooms.Add(Room);
        }
    }

    return Rooms;
}

AStagehandDemoAIController* AStagehandDemoNPCCharacter::ResolveDemoController(bool bSpawnIfMissing)
{
    if (AStagehandDemoAIController* CachedController = CachedDemoController.Get())
    {
        return CachedController;
    }

    AStagehandDemoAIController* DemoController = Cast<AStagehandDemoAIController>(GetController());
    if (!DemoController && bSpawnIfMissing)
    {
        SpawnDefaultController();
        DemoController = Cast<AStagehandDemoAIController>(GetController());
    }

    if (DemoController)
    {
        CachedDemoController = DemoController;
    }

    return DemoController;
}

void AStagehandDemoNPCCharacter::BindToDemoController()
{
    if (AStagehandDemoAIController* DemoController = ResolveDemoController(true))
    {
        DemoController->OnStagehandMoveCompleted.RemoveAll(this);
        DemoController->OnStagehandMoveCompleted.AddUObject(this, &AStagehandDemoNPCCharacter::HandleMoveCompleted);
    }
}

void AStagehandDemoNPCCharacter::EvaluateBehavior()
{
    switch (LoopState)
    {
    case EStagehandDemoLoopState::WaitForLayout:
        if (GatherGeneratorRooms().IsEmpty())
        {
            LastFailureReason = TEXT("Waiting for Ginny to finish generating rooms.");
            UpdateDebugText();
            ScheduleBehavior(LayoutPollInterval);
            return;
        }

        SetLoopState(EStagehandDemoLoopState::SelectMarker, TEXT("Layout is ready."));
        ScheduleBehavior(0.0f);
        return;

    case EStagehandDemoLoopState::SelectMarker:
        if (SelectNextMarker())
        {
            MoveToCurrentSelection();
            return;
        }

        SetLoopState(EStagehandDemoLoopState::Retry, LastFailureReason);
        ScheduleBehavior(RetryDelay);
        return;

    case EStagehandDemoLoopState::PauseAtMarker:
        LastSelection = CurrentSelection;
        ++CompletedLoops;
        SelectionSeed = HashCombineFast(SelectionSeed, CompletedLoops * 17 + 13);
        SetLoopState(EStagehandDemoLoopState::SelectMarker, TEXT("Selecting a new marker."));
        ScheduleBehavior(0.0f);
        return;

    case EStagehandDemoLoopState::Retry:
        SetLoopState(EStagehandDemoLoopState::SelectMarker, TEXT("Retrying marker selection."));
        ScheduleBehavior(0.0f);
        return;

    case EStagehandDemoLoopState::MoveToMarker:
    default:
        return;
    }
}

void AStagehandDemoNPCCharacter::MoveToCurrentSelection()
{
    if (!HasUsableSelection(CurrentSelection))
    {
        LastFailureReason = TEXT("Move requested without a valid selection.");
        SetLoopState(EStagehandDemoLoopState::Retry, LastFailureReason);
        ScheduleBehavior(RetryDelay);
        return;
    }

    AStagehandDemoAIController* DemoController = ResolveDemoController(true);
    if (!DemoController)
    {
        LastFailureReason = TEXT("No AI controller is available for movement.");
        SetLoopState(EStagehandDemoLoopState::Retry, LastFailureReason);
        ScheduleBehavior(RetryDelay);
        return;
    }

    CurrentMoveDestination = CurrentSelection.Marker.WorldTransform.GetLocation();
    if (UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        FNavLocation ProjectedLocation;
        const FVector ProjectionExtent(120.0f, 120.0f, 240.0f);
        if (NavigationSystem->ProjectPointToNavigation(CurrentMoveDestination, ProjectedLocation, ProjectionExtent))
        {
            CurrentMoveDestination = ProjectedLocation.Location;
        }
        else
        {
            LastFailureReason = TEXT("Target marker is off the navmesh.");
            SetLoopState(EStagehandDemoLoopState::Retry, LastFailureReason);
            ScheduleBehavior(RetryDelay);
            return;
        }
    }

    const EPathFollowingRequestResult::Type MoveResult = DemoController->RequestMoveToLocation(
        CurrentMoveDestination,
        AcceptanceRadius,
        true);

    if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        LastFailureReason = TEXT("Nav move request failed.");
        SetLoopState(EStagehandDemoLoopState::Retry, LastFailureReason);
        ScheduleBehavior(RetryDelay);
        return;
    }

    if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        FaceCurrentMarker();
        SetLoopState(EStagehandDemoLoopState::PauseAtMarker, TEXT("Already at marker."));
        ScheduleBehavior(FMath::FRandRange(PauseDurationMin, PauseDurationMax));
        return;
    }

    SetLoopState(EStagehandDemoLoopState::MoveToMarker, CurrentSelection.Notes);
}

void AStagehandDemoNPCCharacter::HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type ResultCode)
{
    if (LoopState != EStagehandDemoLoopState::MoveToMarker)
    {
        return;
    }

    if (ResultCode == EPathFollowingResult::Success)
    {
        FaceCurrentMarker();
        SetLoopState(EStagehandDemoLoopState::PauseAtMarker, TEXT("Reached marker."));
        ScheduleBehavior(FMath::FRandRange(PauseDurationMin, PauseDurationMax));
        return;
    }

    LastFailureReason = FString::Printf(TEXT("Move failed with path result %d."), static_cast<int32>(ResultCode));
    SetLoopState(EStagehandDemoLoopState::Retry, LastFailureReason);
    ScheduleBehavior(RetryDelay);
}

void AStagehandDemoNPCCharacter::FaceCurrentMarker()
{
    if (!HasUsableSelection(CurrentSelection))
    {
        return;
    }

    SetActorRotation(CurrentSelection.Marker.WorldTransform.GetRotation());
}

void AStagehandDemoNPCCharacter::ScheduleBehavior(float DelaySeconds)
{
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);

    if (DelaySeconds <= KINDA_SMALL_NUMBER)
    {
        FTimerDelegate NextTickDelegate;
        NextTickDelegate.BindUObject(this, &AStagehandDemoNPCCharacter::EvaluateBehavior);
        GetWorldTimerManager().SetTimerForNextTick(NextTickDelegate);
        return;
    }

    GetWorldTimerManager().SetTimer(
        BehaviorTimerHandle,
        this,
        &AStagehandDemoNPCCharacter::EvaluateBehavior,
        DelaySeconds,
        false);
}

void AStagehandDemoNPCCharacter::SetLoopState(EStagehandDemoLoopState NewState, const FString& DebugReason)
{
    LoopState = NewState;

    if (bLogStateChanges)
    {
        UE_LOG(
            LogStagehandDemoNPC,
            Log,
            TEXT("%s -> %s (%s)"),
            *GetName(),
            *ToStateLabel(LoopState),
            DebugReason.IsEmpty() ? TEXT("No reason") : *DebugReason);
    }

    UpdateDebugText();
}

void AStagehandDemoNPCCharacter::UpdateDebugText()
{
    const FString ProfileName = NPCProfile
        ? NPCProfile->DisplayName.IsEmpty() ? NPCProfile->GetName() : NPCProfile->DisplayName.ToString()
        : TEXT("NoProfile");
    const FString ActivityName = CurrentSelection.ActivityTag.IsValid()
        ? CurrentSelection.ActivityTag.ToString()
        : TEXT("NoActivity");
    const FString MarkerName = CurrentSelection.Marker.MarkerName.IsNone()
        ? TEXT("NoMarker")
        : CurrentSelection.Marker.MarkerName.ToString();
    const FString RoomName = CurrentSelection.Room
        ? CurrentSelection.Room->GetName()
        : TEXT("NoRoom");
    const FString DestinationName = CurrentMoveDestination.IsNearlyZero()
        ? TEXT("NoDestination")
        : CurrentMoveDestination.ToCompactString();
    const FString FailureText = LastFailureReason.IsEmpty()
        ? TEXT("-")
        : LastFailureReason;

    if (DebugText)
    {
        const FColor DebugColor = NPCProfile
            ? NPCProfile->DebugColor.ToFColor(true)
            : FColor::White;
        DebugText->SetTextRenderColor(DebugColor);
        DebugText->SetDisplayLines(
            FText::FromString(ProfileName),
            FText::FromString(ToStateLabel(LoopState)),
            FText::FromString(FString::Printf(
                TEXT("%s / %s | %s"),
                *RoomName,
                *MarkerName,
                *ActivityName)),
            FText::FromString(FString::Printf(
                TEXT("Move %s | Loop %d | %s"),
                *DestinationName,
                CompletedLoops,
                *FailureText)));
    }

    if (TargetArrow)
    {
        const bool bHasSelection = HasUsableSelection(CurrentSelection);
        TargetArrow->SetVisibility(bHasSelection);
        TargetArrow->SetHiddenInGame(!bHasSelection);

        if (bHasSelection)
        {
            const FVector ToTarget = CurrentSelection.Marker.WorldTransform.GetLocation() - GetActorLocation();
            if (!ToTarget.IsNearlyZero())
            {
                TargetArrow->SetWorldRotation(ToTarget.Rotation());
            }
        }
    }
}

bool AStagehandDemoNPCCharacter::MatchesLastSelection(const FStagehandNPCMarkerSelection& Selection) const
{
    return LastSelection.Room == Selection.Room &&
        LastSelection.Marker.MarkerName == Selection.Marker.MarkerName &&
        !Selection.Marker.MarkerName.IsNone();
}

bool AStagehandDemoNPCCharacter::HasUsableSelection(const FStagehandNPCMarkerSelection& Selection) const
{
    return Selection.bFoundSelection &&
        Selection.Room != nullptr &&
        !Selection.Marker.MarkerName.IsNone();
}
