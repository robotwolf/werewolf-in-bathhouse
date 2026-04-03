#include "StagingDemoNPCCharacter.h"

#include "StagingDemoAIController.h"
#include "Animation/AnimInstance.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "RoomGameplayMarkerLibrary.h"
#include "RoomGenerator.h"
#include "StagingQueryLibrary.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "WerewolfGameplayTagLibrary.h"
#include "WerewolfStateBillboardComponent.h"
#include <initializer_list>

DEFINE_LOG_CATEGORY_STATIC(LogStagingDemoNPC, Log, All);

namespace
{
    FString ToStateLabel(EStagingDemoLoopState LoopState)
    {
        switch (LoopState)
        {
        case EStagingDemoLoopState::WaitForLayout:
            return TEXT("WaitForLayout");
        case EStagingDemoLoopState::SelectMarker:
            return TEXT("SelectMarker");
        case EStagingDemoLoopState::MoveToMarker:
            return TEXT("MoveToMarker");
        case EStagingDemoLoopState::PauseAtMarker:
            return TEXT("PauseAtMarker");
        case EStagingDemoLoopState::Retry:
        default:
            return TEXT("Retry");
        }
    }

    FString ToActionLabel(EStagingDemoActionState ActionState)
    {
        switch (ActionState)
        {
        case EStagingDemoActionState::Transit:
            return TEXT("Transit");
        case EStagingDemoActionState::IdleWait:
            return TEXT("IdleWait");
        case EStagingDemoActionState::Observe:
            return TEXT("Observe");
        case EStagingDemoActionState::InspectClue:
            return TEXT("InspectClue");
        case EStagingDemoActionState::Socialize:
            return TEXT("Socialize");
        case EStagingDemoActionState::Hide:
            return TEXT("Hide");
        case EStagingDemoActionState::None:
        default:
            return TEXT("None");
        }
    }

    FString ToGideonModeLabel(EGideonNPCRuntimeMode RuntimeMode)
    {
        switch (RuntimeMode)
        {
        case EGideonNPCRuntimeMode::Queueing:
            return TEXT("Queueing");
        case EGideonNPCRuntimeMode::AwaitingAdmission:
            return TEXT("AwaitingAdmission");
        case EGideonNPCRuntimeMode::Admitted:
            return TEXT("Admitted");
        case EGideonNPCRuntimeMode::Roaming:
            return TEXT("Roaming");
        case EGideonNPCRuntimeMode::Hiding:
            return TEXT("Hiding");
        case EGideonNPCRuntimeMode::Leaving:
            return TEXT("Leaving");
        case EGideonNPCRuntimeMode::Departed:
            return TEXT("Departed");
        case EGideonNPCRuntimeMode::Spawning:
        default:
            return TEXT("Spawning");
        }
    }

    bool ContainsAnyToken(const FString& Value, std::initializer_list<const TCHAR*> Tokens)
    {
        for (const TCHAR* Token : Tokens)
        {
            if (Value.Contains(Token, ESearchCase::IgnoreCase))
            {
                return true;
            }
        }

        return false;
    }

    FString GetProfileDisplayName(const UStagingNPCProfile* Profile)
    {
        if (!Profile)
        {
            return TEXT("NoProfile");
        }

        return Profile->DisplayName.IsEmpty()
            ? Profile->GetName()
            : Profile->DisplayName.ToString();
    }

    FString GetRoomDisplayName(const ARoomModuleBase* Room)
    {
        return Room ? Room->GetName() : TEXT("NoRoom");
    }

    FString GetMarkerDisplayName(const FRoomGameplayMarker& Marker)
    {
        return Marker.MarkerName.IsNone()
            ? TEXT("NoMarker")
            : Marker.MarkerName.ToString();
    }
}

AStagingDemoNPCCharacter::AStagingDemoNPCCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    AIControllerClass = AStagingDemoAIController::StaticClass();
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

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMeshFinder(
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
    if (MannyMeshFinder.Succeeded() && GetMesh())
    {
        GetMesh()->SetSkeletalMesh(MannyMeshFinder.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    static ConstructorHelpers::FClassFinder<UAnimInstance> StagingAnimClassFinder(
        TEXT("/Game/WerewolfBH/Blueprints/NPC/ABP_Manny_StagingNPC"));
    if (StagingAnimClassFinder.Succeeded())
    {
        DefaultAnimationBlueprint = StagingAnimClassFinder.Class;
    }

    ApplyConfiguredAnimationBlueprint();
}

void AStagingDemoNPCCharacter::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyConfiguredAnimationBlueprint();
}

void AStagingDemoNPCCharacter::BeginPlay()
{
    Super::BeginPlay();

    ApplyConfiguredAnimationBlueprint();
    BindToDemoController();
    UpdateDebugText();

    if (bAutoStartOnBeginPlay)
    {
        StartBehaviorLoop();
    }
}

void AStagingDemoNPCCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bUseWorldDebugString || !GetWorld())
    {
        return;
    }

    const FColor DebugColor = NPCProfile
        ? NPCProfile->DebugColor.ToFColor(true)
        : FColor::White;

    DrawDebugString(
        GetWorld(),
        GetActorLocation() + DebugTextWorldOffset,
        BuildDebugDisplayString(),
        nullptr,
        DebugColor,
        0.0f,
        true,
        DebugTextFontScale);
}

void AStagingDemoNPCCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);

    if (AStagingDemoAIController* DemoController = CachedDemoController.Get())
    {
        DemoController->OnStagingMoveCompleted.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

void AStagingDemoNPCCharacter::ApplyConfiguredAnimationBlueprint()
{
    if (!GetMesh() || !DefaultAnimationBlueprint)
    {
        return;
    }

    if (GetMesh()->GetAnimClass() != DefaultAnimationBlueprint.Get())
    {
        GetMesh()->SetAnimInstanceClass(DefaultAnimationBlueprint.Get());
    }
}

void AStagingDemoNPCCharacter::StartBehaviorLoop()
{
    LastFailureReason.Reset();
    GideonStatusReason.Reset();
    CurrentSelection = FStagingNPCMarkerSelection();
    LastSelection = FStagingNPCMarkerSelection();
    CurrentMoveDestination = FVector::ZeroVector;
    CurrentActionState = EStagingDemoActionState::IdleWait;
    CurrentPresentation = FStagingDemoPresentationPayload();
    ConsecutiveRetryCount = 0;
    bDestroyOnGideonMoveArrival = false;
    GideonRuntimeMode = EGideonNPCRuntimeMode::Roaming;
    SetLoopState(EStagingDemoLoopState::WaitForLayout, TEXT("Starting marker-loop demo."));
    ScheduleBehavior(0.05f);
}

void AStagingDemoNPCCharacter::StopBehaviorLoop()
{
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);
    UpdateActionState(EStagingDemoActionState::IdleWait, TEXT("Behavior loop stopped."));

    if (AStagingDemoAIController* DemoController = ResolveDemoController(false))
    {
        DemoController->StopMovement();
    }

    UpdateDebugText();
}

void AStagingDemoNPCCharacter::ConfigureForGideon(
    ARoomGenerator* InTargetGenerator,
    UStagingNPCProfile* InNPCProfile,
    EStagingRunPhase InPhase,
    bool bInTreatAsWerewolf,
    int32 InSelectionSeed)
{
    TargetGenerator = InTargetGenerator;
    NPCProfile = InNPCProfile;
    Phase = InPhase;
    bTreatAsWerewolf = bInTreatAsWerewolf;
    SelectionSeed = InSelectionSeed;
    GideonRuntimeMode = EGideonNPCRuntimeMode::Spawning;
    GideonQueueSlotIndex = INDEX_NONE;
    GideonStatusReason = TEXT("Arriving at the bathhouse.");
    bAutoStartOnBeginPlay = false;
    StopBehaviorLoop();
    UpdateDebugText();
}

bool AStagingDemoNPCCharacter::MoveToQueueLocation(const FVector& QueueLocation, int32 QueueSlotIndex)
{
    GideonQueueSlotIndex = QueueSlotIndex;
    GideonStatusReason = TEXT("Heading to the booth queue.");
    bDestroyOnGideonMoveArrival = false;
    return RequestGideonMove(QueueLocation, EGideonNPCRuntimeMode::Queueing, GideonStatusReason);
}

void AStagingDemoNPCCharacter::SetAwaitingAdmissionState(int32 QueueSlotIndex)
{
    GideonQueueSlotIndex = QueueSlotIndex;
    GideonRuntimeMode = EGideonNPCRuntimeMode::AwaitingAdmission;
    GideonStatusReason = TEXT("Waiting for admission.");
    CurrentMoveDestination = FVector::ZeroVector;

    if (AStagingDemoAIController* DemoController = ResolveDemoController(false))
    {
        DemoController->StopMovement();
    }

    UpdateActionState(EStagingDemoActionState::IdleWait, GideonStatusReason);
    UpdateDebugText();
}

bool AStagingDemoNPCCharacter::AdmitToBathhouse(const FVector& AdmitLocation)
{
    GideonStatusReason = TEXT("Entering the bathhouse.");
    bDestroyOnGideonMoveArrival = false;
    return RequestGideonMove(AdmitLocation, EGideonNPCRuntimeMode::Admitted, GideonStatusReason);
}

bool AStagingDemoNPCCharacter::EnterHideState(ARoomModuleBase* HideRoom)
{
    if (!HideRoom)
    {
        return false;
    }

    GideonRetreatRoom = HideRoom;
    ++GideonScareCount;

    FRoomGameplayMarker HideMarker;
    const FVector HideDestination = ResolveHideDestination(HideRoom, &HideMarker);
    GideonStatusReason = TEXT("Seeking a hiding spot.");
    bDestroyOnGideonMoveArrival = false;

    if (!HideMarker.MarkerName.IsNone())
    {
        CurrentSelection = FStagingNPCMarkerSelection();
        CurrentSelection.bFoundSelection = true;
        CurrentSelection.NPCProfile = NPCProfile;
        CurrentSelection.Phase = Phase;
        CurrentSelection.bIsWerewolfContext = bTreatAsWerewolf;
        CurrentSelection.ActivityTag = UWerewolfGameplayTagLibrary::MakeGameplayTagFromName(TEXT("NPC.Activity.Hide"), false);
        CurrentSelection.Room = HideRoom;
        CurrentSelection.Marker = HideMarker;
        CurrentSelection.Notes = TEXT("Gideon selected a hide anchor.");
    }

    return RequestGideonMove(HideDestination, EGideonNPCRuntimeMode::Hiding, GideonStatusReason);
}

bool AStagingDemoNPCCharacter::BeginLeavingBathhouse(const FVector& ExitLocation)
{
    GideonStatusReason = TEXT("Leaving the bathhouse.");
    bDestroyOnGideonMoveArrival = true;
    return RequestGideonMove(ExitLocation, EGideonNPCRuntimeMode::Leaving, GideonStatusReason);
}

void AStagingDemoNPCCharacter::ResumeRoamingFromGideon()
{
    GideonQueueSlotIndex = INDEX_NONE;
    GideonStatusReason = TEXT("Back to the bathhouse loop.");
    bDestroyOnGideonMoveArrival = false;
    StartBehaviorLoop();
}

void AStagingDemoNPCCharacter::SetRunPhaseState(EStagingRunPhase NewPhase)
{
    Phase = NewPhase;
    UpdateDebugText();
}

void AStagingDemoNPCCharacter::SetRetreatRoom(ARoomModuleBase* RetreatRoom)
{
    GideonRetreatRoom = RetreatRoom;
}

void AStagingDemoNPCCharacter::SetTowelTier(EGideonTowelTier NewTowelTier)
{
    GideonTowelTier = NewTowelTier;
}

void AStagingDemoNPCCharacter::AddFear(float FearDelta)
{
    SetFear(CurrentFear + FearDelta);
}

void AStagingDemoNPCCharacter::SetFear(float NewFear)
{
    CurrentFear = FMath::Max(0.0f, NewFear);
    UpdateDebugText();
}

FGideonNPCRuntimeState AStagingDemoNPCCharacter::GetGideonRuntimeState() const
{
    FGideonNPCRuntimeState RuntimeState;
    RuntimeState.NPC = const_cast<AStagingDemoNPCCharacter*>(this);
    RuntimeState.RuntimeMode = GideonRuntimeMode;
    RuntimeState.CurrentRoom = GetCurrentResolvedRoom();
    RuntimeState.RetreatRoom = GideonRetreatRoom;
    RuntimeState.ActivePOIRoom = nullptr;
    RuntimeState.Fear = CurrentFear;
    RuntimeState.ScareCount = GideonScareCount;
    RuntimeState.QueueSlotIndex = GideonQueueSlotIndex;
    RuntimeState.bAdmissionApproved = GideonRuntimeMode == EGideonNPCRuntimeMode::Admitted || GideonRuntimeMode == EGideonNPCRuntimeMode::Roaming;
    RuntimeState.TowelTier = GideonTowelTier;
    return RuntimeState;
}

ARoomModuleBase* AStagingDemoNPCCharacter::GetCurrentResolvedRoom() const
{
    if (CurrentSelection.Room)
    {
        return CurrentSelection.Room;
    }

    if (GideonRuntimeMode == EGideonNPCRuntimeMode::Hiding && GideonRetreatRoom)
    {
        return GideonRetreatRoom;
    }

    return FindNearestGeneratedRoom();
}

bool AStagingDemoNPCCharacter::CanBeForcedToLeave() const
{
    return !NPCProfile || !NPCProfile->bStoryPinned;
}

bool AStagingDemoNPCCharacter::IsGideonAutonomousRoaming() const
{
    return GideonRuntimeMode == EGideonNPCRuntimeMode::Roaming;
}

bool AStagingDemoNPCCharacter::SelectNextMarker()
{
    const TArray<ARoomModuleBase*> Rooms = GatherGeneratorRooms();
    if (Rooms.IsEmpty())
    {
        CurrentMoveDestination = FVector::ZeroVector;
        LastFailureReason = TEXT("No generated rooms are available yet.");
        UpdateDebugText();
        return false;
    }

    FStagingNPCMarkerSelection FirstValidSelection;
    const int32 AttemptCount = FMath::Clamp(MaxSelectionAttempts, 1, 12);
    const int32 LoopSeedBase = HashCombineFast(SelectionSeed, CompletedLoops * 7919 + 37);

    for (int32 AttemptIndex = 0; AttemptIndex < AttemptCount; ++AttemptIndex)
    {
        const FStagingNPCMarkerSelection CandidateSelection = UStagingSimulationLibrary::PickMarkerForNPCProfile(
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
            ConsecutiveRetryCount = 0;
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
        if (IsSelectionNearCurrentLocation(FirstValidSelection))
        {
            CurrentSelection = FStagingNPCMarkerSelection();
            CurrentMoveDestination = FVector::ZeroVector;
            LastFailureReason = TEXT("Only the previous marker was available, and we're already standing on it.");
            UpdateDebugText();
            return false;
        }

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

    CurrentSelection = FStagingNPCMarkerSelection();
    CurrentMoveDestination = FVector::ZeroVector;
    LastFailureReason = NPCProfile
        ? TEXT("No matching NPC marker could be selected.")
        : TEXT("No NPC profile is assigned.");
    UpdateDebugText();
    return false;
}

TArray<ARoomModuleBase*> AStagingDemoNPCCharacter::GatherGeneratorRooms() const
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

AStagingDemoAIController* AStagingDemoNPCCharacter::ResolveDemoController(bool bSpawnIfMissing)
{
    if (AStagingDemoAIController* CachedController = CachedDemoController.Get())
    {
        return CachedController;
    }

    AStagingDemoAIController* DemoController = Cast<AStagingDemoAIController>(GetController());
    if (!DemoController && bSpawnIfMissing)
    {
        SpawnDefaultController();
        DemoController = Cast<AStagingDemoAIController>(GetController());
    }

    if (DemoController)
    {
        CachedDemoController = DemoController;
    }

    return DemoController;
}

void AStagingDemoNPCCharacter::BindToDemoController()
{
    if (AStagingDemoAIController* DemoController = ResolveDemoController(true))
    {
        DemoController->OnStagingMoveCompleted.RemoveAll(this);
        DemoController->OnStagingMoveCompleted.AddUObject(this, &AStagingDemoNPCCharacter::HandleMoveCompleted);
    }
}

void AStagingDemoNPCCharacter::EvaluateBehavior()
{
    switch (LoopState)
    {
    case EStagingDemoLoopState::WaitForLayout:
        if (GatherGeneratorRooms().IsEmpty())
        {
            LastFailureReason = TEXT("Waiting for Ginny to finish generating rooms.");
            UpdateActionState(EStagingDemoActionState::IdleWait, LastFailureReason);
            UpdateDebugText();
            ScheduleBehavior(LayoutPollInterval);
            return;
        }

        SetLoopState(EStagingDemoLoopState::SelectMarker, TEXT("Layout is ready."));
        ScheduleBehavior(0.0f);
        return;

    case EStagingDemoLoopState::SelectMarker:
        if (SelectNextMarker())
        {
            MoveToCurrentSelection();
            return;
        }

        SetLoopState(EStagingDemoLoopState::Retry, LastFailureReason);
        ScheduleBehavior(RetryDelay);
        return;

    case EStagingDemoLoopState::PauseAtMarker:
        LastSelection = CurrentSelection;
        ConsecutiveRetryCount = 0;
        ++CompletedLoops;
        SelectionSeed = HashCombineFast(SelectionSeed, CompletedLoops * 17 + 13);
        UpdateActionState(EStagingDemoActionState::IdleWait, TEXT("Selecting a new marker."));
        SetLoopState(EStagingDemoLoopState::SelectMarker, TEXT("Selecting a new marker."));
        ScheduleBehavior(0.0f);
        return;

    case EStagingDemoLoopState::Retry:
        ++ConsecutiveRetryCount;
        SelectionSeed = HashCombineFast(SelectionSeed, ConsecutiveRetryCount * 97 + 11);
        UpdateActionState(EStagingDemoActionState::IdleWait, TEXT("Retrying marker selection."));
        SetLoopState(EStagingDemoLoopState::SelectMarker, TEXT("Retrying marker selection."));
        ScheduleBehavior(0.0f);
        return;

    case EStagingDemoLoopState::MoveToMarker:
    default:
        return;
    }
}

void AStagingDemoNPCCharacter::MoveToCurrentSelection()
{
    if (!HasUsableSelection(CurrentSelection))
    {
        LastFailureReason = TEXT("Move requested without a valid selection.");
        SetLoopState(EStagingDemoLoopState::Retry, LastFailureReason);
        ScheduleBehavior(RetryDelay);
        return;
    }

    AStagingDemoAIController* DemoController = ResolveDemoController(true);
    if (!DemoController)
    {
        LastFailureReason = TEXT("No AI controller is available for movement.");
        SetLoopState(EStagingDemoLoopState::Retry, LastFailureReason);
        ScheduleBehavior(RetryDelay);
        return;
    }

    UpdateActionState(EStagingDemoActionState::Transit, CurrentSelection.Notes);
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
            UpdateActionState(EStagingDemoActionState::IdleWait, LastFailureReason);
            SetLoopState(EStagingDemoLoopState::Retry, LastFailureReason);
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
        UpdateActionState(EStagingDemoActionState::IdleWait, LastFailureReason);
        SetLoopState(EStagingDemoLoopState::Retry, LastFailureReason);
        ScheduleBehavior(RetryDelay);
        return;
    }

    if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        FaceCurrentMarker();
        UpdateActionState(ResolveActionStateForCurrentSelection(), TEXT("Already at marker."));
        SetLoopState(EStagingDemoLoopState::PauseAtMarker, TEXT("Already at marker."));
        ScheduleBehavior(FMath::FRandRange(PauseDurationMin, PauseDurationMax));
        return;
    }

    SetLoopState(EStagingDemoLoopState::MoveToMarker, CurrentSelection.Notes);
}

void AStagingDemoNPCCharacter::HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type ResultCode)
{
    if (GideonRuntimeMode != EGideonNPCRuntimeMode::Roaming &&
        GideonRuntimeMode != EGideonNPCRuntimeMode::Spawning)
    {
        HandleGideonMoveCompleted(ResultCode);
        return;
    }

    if (LoopState != EStagingDemoLoopState::MoveToMarker)
    {
        return;
    }

    if (ResultCode == EPathFollowingResult::Success)
    {
        FaceCurrentMarker();
        UpdateActionState(ResolveActionStateForCurrentSelection(), TEXT("Reached marker."));
        SetLoopState(EStagingDemoLoopState::PauseAtMarker, TEXT("Reached marker."));
        ScheduleBehavior(FMath::FRandRange(PauseDurationMin, PauseDurationMax));
        return;
    }

    LastFailureReason = FString::Printf(TEXT("Move failed with path result %d."), static_cast<int32>(ResultCode));
    UpdateActionState(EStagingDemoActionState::IdleWait, LastFailureReason);
    SetLoopState(EStagingDemoLoopState::Retry, LastFailureReason);
    ScheduleBehavior(RetryDelay);
}

void AStagingDemoNPCCharacter::FaceCurrentMarker()
{
    if (!HasUsableSelection(CurrentSelection))
    {
        return;
    }

    SetActorRotation(CurrentSelection.Marker.WorldTransform.GetRotation());
}

bool AStagingDemoNPCCharacter::RequestGideonMove(const FVector& TargetLocation, EGideonNPCRuntimeMode TargetMode, const FString& DebugReason)
{
    StopBehaviorLoop();

    AStagingDemoAIController* DemoController = ResolveDemoController(true);
    if (!DemoController)
    {
        LastFailureReason = TEXT("No AI controller is available for Gideon movement.");
        UpdateActionState(EStagingDemoActionState::IdleWait, LastFailureReason);
        return false;
    }

    FVector ProjectedDestination = TargetLocation;
    if (UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        FNavLocation ProjectedLocation;
        if (NavigationSystem->ProjectPointToNavigation(TargetLocation, ProjectedLocation, FVector(180.0f, 180.0f, 260.0f)))
        {
            ProjectedDestination = ProjectedLocation.Location;
        }
    }

    GideonRuntimeMode = TargetMode;
    GideonStatusReason = DebugReason;
    CurrentMoveDestination = ProjectedDestination;
    UpdateActionState(
        TargetMode == EGideonNPCRuntimeMode::Hiding ? EStagingDemoActionState::Hide : EStagingDemoActionState::Transit,
        DebugReason);

    const EPathFollowingRequestResult::Type MoveResult = DemoController->RequestMoveToLocation(
        ProjectedDestination,
        AcceptanceRadius,
        true);

    if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        LastFailureReason = TEXT("Gideon move request failed.");
        UpdateActionState(EStagingDemoActionState::IdleWait, LastFailureReason);
        return false;
    }

    if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        HandleGideonMoveCompleted(EPathFollowingResult::Success);
    }

    return true;
}

void AStagingDemoNPCCharacter::HandleGideonMoveCompleted(EPathFollowingResult::Type ResultCode)
{
    if (ResultCode != EPathFollowingResult::Success)
    {
        LastFailureReason = FString::Printf(TEXT("Gideon move failed with path result %d."), static_cast<int32>(ResultCode));
        UpdateActionState(EStagingDemoActionState::IdleWait, LastFailureReason);

        if (GideonRuntimeMode == EGideonNPCRuntimeMode::Leaving && bDestroyOnGideonMoveArrival)
        {
            GideonRuntimeMode = EGideonNPCRuntimeMode::Departed;
            Destroy();
        }
        return;
    }

    switch (GideonRuntimeMode)
    {
    case EGideonNPCRuntimeMode::Queueing:
        SetAwaitingAdmissionState(GideonQueueSlotIndex);
        break;

    case EGideonNPCRuntimeMode::Admitted:
        GideonRuntimeMode = EGideonNPCRuntimeMode::Roaming;
        ResumeRoamingFromGideon();
        break;

    case EGideonNPCRuntimeMode::Hiding:
        UpdateActionState(EStagingDemoActionState::Hide, TEXT("Reached hide destination."));
        CurrentMoveDestination = FVector::ZeroVector;
        UpdateDebugText();
        break;

    case EGideonNPCRuntimeMode::Leaving:
        GideonRuntimeMode = EGideonNPCRuntimeMode::Departed;
        if (bDestroyOnGideonMoveArrival)
        {
            Destroy();
        }
        break;

    default:
        break;
    }
}

FVector AStagingDemoNPCCharacter::ResolveHideDestination(ARoomModuleBase* HideRoom, FRoomGameplayMarker* OutMarker) const
{
    if (OutMarker)
    {
        *OutMarker = FRoomGameplayMarker();
    }

    if (!HideRoom)
    {
        return GetActorLocation();
    }

    FStagingMarkerQuery MarkerQuery;
    MarkerQuery.bRestrictToMarkerFamily = true;
    MarkerQuery.MarkerFamily = ERoomGameplayMarkerFamily::NPC;
    if (const FGameplayTag HideTag = UWerewolfGameplayTagLibrary::MakeGameplayTagFromName(TEXT("NPC.Activity.Hide"), false); HideTag.IsValid())
    {
        MarkerQuery.TagQuery.PreferredTags.AddTag(HideTag);
    }

    const FStagingMarkerSelection MarkerSelection = UStagingQueryLibrary::PickBestMarkerInRoom(
        HideRoom,
        MarkerQuery,
        HashCombineFast(SelectionSeed, 7001));

    if (MarkerSelection.bFoundSelection && !MarkerSelection.Marker.MarkerName.IsNone())
    {
        if (OutMarker)
        {
            *OutMarker = MarkerSelection.Marker;
        }
        return MarkerSelection.Marker.WorldTransform.GetLocation();
    }

    return HideRoom->GetActorLocation();
}

ARoomModuleBase* AStagingDemoNPCCharacter::FindNearestGeneratedRoom() const
{
    const TArray<ARoomModuleBase*> Rooms = GatherGeneratorRooms();
    ARoomModuleBase* BestRoom = nullptr;
    float BestDistanceSquared = TNumericLimits<float>::Max();
    const FVector ActorLocation = GetActorLocation();

    for (ARoomModuleBase* Room : Rooms)
    {
        if (!Room)
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared(Room->GetActorLocation(), ActorLocation);
        if (DistanceSquared < BestDistanceSquared)
        {
            BestDistanceSquared = DistanceSquared;
            BestRoom = Room;
        }
    }

    return BestRoom;
}

void AStagingDemoNPCCharacter::UpdateActionState(EStagingDemoActionState NewActionState, const FString& DebugReason)
{
    CurrentActionState = NewActionState;

    if (bLogStateChanges)
    {
        UE_LOG(
            LogStagingDemoNPC,
            Log,
            TEXT("%s -> Action %s (%s)"),
            *GetName(),
            *ToActionLabel(CurrentActionState),
            DebugReason.IsEmpty() ? TEXT("No reason") : *DebugReason);
    }

    RefreshPresentationPayload(DebugReason);
}

void AStagingDemoNPCCharacter::RefreshPresentationPayload(const FString& DebugReason)
{
    const FString ProfileName = GetProfileDisplayName(NPCProfile);
    const FString RoomName = GetRoomDisplayName(CurrentSelection.Room);
    const FString MarkerName = GetMarkerDisplayName(CurrentSelection.Marker);
    const FString ActivityName = CurrentSelection.ActivityTag.IsValid()
        ? CurrentSelection.ActivityTag.ToString()
        : TEXT("NoActivity");
    const AStagingDemoNPCCharacter* ConversationPartner = CurrentActionState == EStagingDemoActionState::Socialize
        ? FindConversationPartner()
        : nullptr;
    const bool bClueContext = HasClueContext();

    FString DetailLine = FString::Printf(
        TEXT("Loop %s | Gideon %s | %s / %s | %s"),
        *ToStateLabel(LoopState),
        *ToGideonModeLabel(GideonRuntimeMode),
        *RoomName,
        *MarkerName,
        *ActivityName);

    if (ConversationPartner)
    {
        DetailLine += FString::Printf(TEXT(" | With %s"), *GetProfileDisplayName(ConversationPartner->NPCProfile));
    }

    if (bClueContext)
    {
        DetailLine += TEXT(" | Clue");
    }

    DetailLine += FString::Printf(TEXT(" | Fear %.2f"), CurrentFear);

    FString StatusLine = BuildPlaceholderDialogueText().ToString();
    if (StatusLine.IsEmpty())
    {
        StatusLine = TEXT("-");
    }

    if (!DebugReason.IsEmpty() && CurrentActionState != EStagingDemoActionState::Socialize && CurrentActionState != EStagingDemoActionState::InspectClue)
    {
        StatusLine = FString::Printf(TEXT("%s | %s"), *StatusLine, *DebugReason);
    }
    else if (!GideonStatusReason.IsEmpty() &&
        GideonRuntimeMode != EGideonNPCRuntimeMode::Roaming &&
        GideonRuntimeMode != EGideonNPCRuntimeMode::Departed)
    {
        StatusLine = FString::Printf(TEXT("%s | %s"), *StatusLine, *GideonStatusReason);
    }

    CurrentPresentation.ActionState = CurrentActionState;
    CurrentPresentation.HeaderText = FText::FromString(ProfileName);
    CurrentPresentation.StateText = BuildActionLabelText(CurrentActionState);
    CurrentPresentation.DetailText = FText::FromString(DetailLine);
    CurrentPresentation.StatusText = FText::FromString(StatusLine);
    CurrentPresentation.RoomName = CurrentSelection.Room ? CurrentSelection.Room->GetFName() : NAME_None;
    CurrentPresentation.MarkerName = CurrentSelection.Marker.MarkerName;
    CurrentPresentation.SocialPartnerName = ConversationPartner && ConversationPartner->NPCProfile
        ? ConversationPartner->NPCProfile->GetFName()
        : (ConversationPartner ? ConversationPartner->GetFName() : NAME_None);
    CurrentPresentation.bHasConversationPartner = ConversationPartner != nullptr;
    CurrentPresentation.bHasClueContext = bClueContext;
}

EStagingDemoActionState AStagingDemoNPCCharacter::ResolveActionStateForCurrentSelection() const
{
    if (!HasUsableSelection(CurrentSelection))
    {
        return EStagingDemoActionState::IdleWait;
    }

    const FString ActivityName = CurrentSelection.ActivityTag.ToString();
    const bool bHasConversationPartner = FindConversationPartner() != nullptr;
    const bool bClueContext = HasClueContext();

    if (ContainsAnyToken(ActivityName, {TEXT("Hide"), TEXT("Sneak"), TEXT("Conceal")}))
    {
        return EStagingDemoActionState::Hide;
    }

    if (ContainsAnyToken(ActivityName, {TEXT("Gossip"), TEXT("Social"), TEXT("Talk"), TEXT("Conversation")}))
    {
        return bHasConversationPartner ? EStagingDemoActionState::Socialize : EStagingDemoActionState::IdleWait;
    }

    if (ContainsAnyToken(ActivityName, {TEXT("Observe"), TEXT("Watch"), TEXT("Scan")}))
    {
        return bClueContext ? EStagingDemoActionState::InspectClue : EStagingDemoActionState::Observe;
    }

    if (ContainsAnyToken(ActivityName, {TEXT("Wait"), TEXT("Relax"), TEXT("Pause"), TEXT("Idle"), TEXT("Clean")}))
    {
        return EStagingDemoActionState::IdleWait;
    }

    if (bClueContext)
    {
        return EStagingDemoActionState::InspectClue;
    }

    if (bHasConversationPartner)
    {
        return EStagingDemoActionState::Socialize;
    }

    return EStagingDemoActionState::Observe;
}

AStagingDemoNPCCharacter* AStagingDemoNPCCharacter::FindConversationPartner() const
{
    if (!GetWorld() || !HasUsableSelection(CurrentSelection) || !CurrentSelection.Room)
    {
        return nullptr;
    }

    const float SearchRadiusSquared = FMath::Square(FMath::Max(0.0f, SocialPartnerSearchRadius));
    const FVector MyLocation = GetActorLocation();
    AStagingDemoNPCCharacter* BestPartner = nullptr;
    float BestDistanceSquared = TNumericLimits<float>::Max();

    for (TActorIterator<AStagingDemoNPCCharacter> It(GetWorld()); It; ++It)
    {
        AStagingDemoNPCCharacter* Other = *It;
        if (!Other || Other == this || Other->TargetGenerator != TargetGenerator)
        {
            continue;
        }

        if (Other->CurrentActionState == EStagingDemoActionState::Transit || Other->CurrentActionState == EStagingDemoActionState::None)
        {
            continue;
        }

        if (!Other->HasUsableSelection(Other->CurrentSelection) || Other->CurrentSelection.Room != CurrentSelection.Room)
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared(MyLocation, Other->GetActorLocation());
        if (DistanceSquared > SearchRadiusSquared || DistanceSquared >= BestDistanceSquared)
        {
            continue;
        }

        BestDistanceSquared = DistanceSquared;
        BestPartner = Other;
    }

    return BestPartner;
}

bool AStagingDemoNPCCharacter::HasClueContext() const
{
    return CurrentSelection.Room && CurrentSelection.Room->GetClueMarkers().Num() > 0;
}

FText AStagingDemoNPCCharacter::BuildPlaceholderDialogueText() const
{
    switch (GideonRuntimeMode)
    {
    case EGideonNPCRuntimeMode::Queueing:
        return FText::FromString(TEXT("Queueing politely."));
    case EGideonNPCRuntimeMode::AwaitingAdmission:
        return FText::FromString(TEXT("Waiting for admission."));
    case EGideonNPCRuntimeMode::Leaving:
        return FText::FromString(TEXT("Leaving the bathhouse."));
    case EGideonNPCRuntimeMode::Departed:
        return FText::FromString(TEXT("Gone."));
    default:
        break;
    }

    const AStagingDemoNPCCharacter* ConversationPartner = CurrentActionState == EStagingDemoActionState::Socialize
        ? FindConversationPartner()
        : nullptr;
    const UStagingNPCProfile* OtherProfile = ConversationPartner ? ConversationPartner->NPCProfile.Get() : nullptr;
    const int32 DialogueSeed = HashCombineFast(
        SelectionSeed,
        CompletedLoops * 313 + static_cast<int32>(CurrentActionState) * 17 + ConsecutiveRetryCount);

    FGameplayTagContainer ContextTags;
    if (CurrentSelection.Room)
    {
        ContextTags.AppendTags(CurrentSelection.Room->GetResolvedRoomTags());
        ContextTags.AppendTags(CurrentSelection.Room->GetResolvedActivityTags());
    }

    if (CurrentSelection.ActivityTag.IsValid())
    {
        ContextTags.AddTag(CurrentSelection.ActivityTag);
    }

    ContextTags.AppendTags(CurrentSelection.Marker.GameplayTags);

    EStagingConversationLineKind LineKind = EStagingConversationLineKind::Idle;

    switch (CurrentActionState)
    {
    case EStagingDemoActionState::Socialize:
        LineKind = EStagingConversationLineKind::Social;
        break;

    case EStagingDemoActionState::InspectClue:
        LineKind = EStagingConversationLineKind::Clue;
        break;

    case EStagingDemoActionState::Hide:
        LineKind = bTreatAsWerewolf
            ? EStagingConversationLineKind::Werewolf
            : EStagingConversationLineKind::Idle;
        break;

    case EStagingDemoActionState::Observe:
    case EStagingDemoActionState::IdleWait:
        LineKind = EStagingConversationLineKind::Idle;
        break;

    case EStagingDemoActionState::Transit:
        return FText::FromString(TEXT("Heading over there."));

    case EStagingDemoActionState::None:
    default:
        return FText::GetEmpty();
    }

    const FStagingConversationLineSelection LineSelection = UStagingSimulationLibrary::PickConversationLineForNPCProfile(
        NPCProfile,
        Phase,
        bTreatAsWerewolf,
        DialogueSeed,
        LineKind,
        ContextTags,
        OtherProfile);

    if (!LineSelection.LineText.IsEmpty())
    {
        return LineSelection.LineText;
    }

    return UStagingSimulationLibrary::BuildPlaceholderConversationLine(LineKind, NPCProfile, OtherProfile);
}

FText AStagingDemoNPCCharacter::BuildActionLabelText(EStagingDemoActionState ActionState) const
{
    return FText::FromString(ToActionLabel(ActionState));
}

FString AStagingDemoNPCCharacter::BuildDebugDisplayString() const
{
    const FString HeaderLine = CurrentPresentation.HeaderText.IsEmpty()
        ? GetProfileDisplayName(NPCProfile)
        : CurrentPresentation.HeaderText.ToString();
    const FString StateLine = CurrentPresentation.StateText.IsEmpty()
        ? ToActionLabel(CurrentActionState)
        : CurrentPresentation.StateText.ToString();
    const FString DetailLine = CurrentPresentation.DetailText.IsEmpty()
        ? TEXT("-")
        : CurrentPresentation.DetailText.ToString();
    const FString StatusLine = CurrentPresentation.StatusText.IsEmpty()
        ? TEXT("-")
        : CurrentPresentation.StatusText.ToString();
    const FString DestinationName = CurrentMoveDestination.IsNearlyZero()
        ? TEXT("NoDestination")
        : CurrentMoveDestination.ToCompactString();

    return FString::Printf(
        TEXT("%s\n%s\n%s\n%s | Move %s | Loop %d | Retry %d"),
        *HeaderLine,
        *StateLine,
        *DetailLine,
        *StatusLine,
        *DestinationName,
        CompletedLoops,
        ConsecutiveRetryCount);
}

void AStagingDemoNPCCharacter::ScheduleBehavior(float DelaySeconds)
{
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);

    if (DelaySeconds <= KINDA_SMALL_NUMBER)
    {
        FTimerDelegate NextTickDelegate;
        NextTickDelegate.BindUObject(this, &AStagingDemoNPCCharacter::EvaluateBehavior);
        GetWorldTimerManager().SetTimerForNextTick(NextTickDelegate);
        return;
    }

    GetWorldTimerManager().SetTimer(
        BehaviorTimerHandle,
        this,
        &AStagingDemoNPCCharacter::EvaluateBehavior,
        DelaySeconds,
        false);
}

void AStagingDemoNPCCharacter::SetLoopState(EStagingDemoLoopState NewState, const FString& DebugReason)
{
    LoopState = NewState;

    if (bLogStateChanges)
    {
        UE_LOG(
            LogStagingDemoNPC,
            Log,
            TEXT("%s -> %s (%s)"),
        *GetName(),
        *ToStateLabel(LoopState),
        DebugReason.IsEmpty() ? TEXT("No reason") : *DebugReason);
    }

    UpdateDebugText();
}

void AStagingDemoNPCCharacter::UpdateDebugText()
{
    RefreshPresentationPayload(LastFailureReason);

    const FString DestinationName = CurrentMoveDestination.IsNearlyZero()
        ? TEXT("NoDestination")
        : CurrentMoveDestination.ToCompactString();

    if (DebugText)
    {
        DebugText->SetVisibility(bUseLegacyBillboardDebug && !bUseWorldDebugString);
        DebugText->SetHiddenInGame(!bUseLegacyBillboardDebug || bUseWorldDebugString);

        if (bUseLegacyBillboardDebug && !bUseWorldDebugString)
        {
            const FColor DebugColor = NPCProfile
                ? NPCProfile->DebugColor.ToFColor(true)
                : FColor::White;
            DebugText->SetTextRenderColor(DebugColor);
            if (CurrentActionState == EStagingDemoActionState::Socialize || CurrentActionState == EStagingDemoActionState::InspectClue)
            {
                DebugText->SetConversationDisplay(
                    CurrentPresentation.HeaderText,
                    CurrentPresentation.StatusText,
                    CurrentPresentation.DetailText);
            }
            else
            {
                const FString StatusLine = CurrentPresentation.StatusText.IsEmpty()
                    ? TEXT("-")
                    : CurrentPresentation.StatusText.ToString();
                DebugText->SetDisplayLines(
                    CurrentPresentation.HeaderText,
                    CurrentPresentation.StateText,
                    CurrentPresentation.DetailText,
                    FText::FromString(FString::Printf(
                        TEXT("%s | Move %s | Loop %d | Retry %d"),
                        *StatusLine,
                        *DestinationName,
                        CompletedLoops,
                        ConsecutiveRetryCount)));
            }
        }
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

bool AStagingDemoNPCCharacter::IsSelectionNearCurrentLocation(const FStagingNPCMarkerSelection& Selection) const
{
    if (!HasUsableSelection(Selection))
    {
        return false;
    }

    const FVector MarkerLocation = Selection.Marker.WorldTransform.GetLocation();
    const float DistanceToMarker = FVector::Dist2D(GetActorLocation(), MarkerLocation);
    return DistanceToMarker <= FMath::Max(10.0f, AcceptanceRadius * 0.9f);
}

bool AStagingDemoNPCCharacter::MatchesLastSelection(const FStagingNPCMarkerSelection& Selection) const
{
    return LastSelection.Room == Selection.Room &&
        LastSelection.Marker.MarkerName == Selection.Marker.MarkerName &&
        !Selection.Marker.MarkerName.IsNone();
}

bool AStagingDemoNPCCharacter::HasUsableSelection(const FStagingNPCMarkerSelection& Selection) const
{
    return Selection.bFoundSelection &&
        Selection.Room != nullptr &&
        !Selection.Marker.MarkerName.IsNone();
}
