#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GideonRuntimeTypes.generated.h"

class ARoomModuleBase;
class AStagingDemoNPCCharacter;

UENUM(BlueprintType)
enum class EGideonNPCRuntimeMode : uint8
{
    Spawning,
    Queueing,
    AwaitingAdmission,
    Admitted,
    Roaming,
    Hiding,
    Leaving,
    Departed
};

UENUM(BlueprintType)
enum class EGideonTowelTier : uint8
{
    CrowdSimple,
    HeroCloth
};

USTRUCT(BlueprintType)
struct FGideonPOISpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|POI")
    TObjectPtr<ARoomModuleBase> OriginRoom = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|POI", meta=(ClampMin="0"))
    int32 RoomsAway = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|POI", meta=(ClampMin="0"))
    int32 AffectRadiusRooms = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|POI", meta=(ClampMin="0.0"))
    float FearDelta = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|POI", meta=(ClampMin="0.0"))
    float LifetimeSeconds = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|POI")
    FGameplayTagContainer EventTags;
};

USTRUCT(BlueprintType)
struct FGideonNPCRuntimeState
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|NPC")
    TObjectPtr<AStagingDemoNPCCharacter> NPC = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|NPC")
    EGideonNPCRuntimeMode RuntimeMode = EGideonNPCRuntimeMode::Spawning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|NPC")
    TObjectPtr<ARoomModuleBase> CurrentRoom = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|NPC")
    TObjectPtr<ARoomModuleBase> RetreatRoom = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|NPC")
    TObjectPtr<ARoomModuleBase> ActivePOIRoom = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|NPC", meta=(ClampMin="0.0"))
    float Fear = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|NPC", meta=(ClampMin="0"))
    int32 ScareCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|NPC")
    int32 QueueSlotIndex = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|NPC")
    bool bAdmissionApproved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|NPC")
    EGideonTowelTier TowelTier = EGideonTowelTier::CrowdSimple;
};
