#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StagehandSimulationData.h"
#include "StagehandDemoCoordinator.generated.h"

class ARoomGenerator;
class ARoomModuleBase;
class AStagehandDemoNPCCharacter;
class USceneComponent;
class UStagehandNPCProfile;

UCLASS(Blueprintable)
class WEREWOLFNBH_API AStagehandDemoCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AStagehandDemoCoordinator();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stagehand|Demo")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    TObjectPtr<ARoomGenerator> TargetGenerator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    TObjectPtr<UStagehandNPCProfile> NPCProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    TArray<TObjectPtr<UStagehandNPCProfile>> NPCProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    TSubclassOf<AStagehandDemoNPCCharacter> DemoNPCClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    EStagehandRunPhase Phase = EStagehandRunPhase::OpeningHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    bool bTreatAsWerewolf = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    bool bGenerateLayoutIfMissing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    int32 DemoSeedOffset = 101;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo", meta=(ClampMin="1", ClampMax="16"))
    int32 NumDemoNPCs = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Timing", meta=(ClampMin="0.0"))
    float SpawnSpacing = 140.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Timing", meta=(ClampMin="0.1"))
    float LayoutPollInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Timing", meta=(ClampMin="0.0"))
    float SpawnHeightOffset = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    bool bAutoStartOnBeginPlay = true;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo")
    TObjectPtr<AStagehandDemoNPCCharacter> SpawnedDemoNPC = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo")
    TArray<TObjectPtr<AStagehandDemoNPCCharacter>> SpawnedDemoNPCs;

    UFUNCTION(BlueprintCallable, Category="Stagehand|Demo")
    void StartDemo();

protected:
    void PollForLayoutAndSpawnNPC();
    TArray<ARoomModuleBase*> GatherGeneratorRooms() const;
    ARoomGenerator* FindGeneratorInWorld() const;
    bool SpawnDemoNPCs();
    bool SpawnDemoNPC(int32 NPCIndex, const TArray<ARoomModuleBase*>& Rooms);
    FTransform BuildInitialSpawnTransform(const TArray<ARoomModuleBase*>& Rooms, int32 NPCIndex, int32 TotalNPCs) const;
    UStagehandNPCProfile* ResolveProfileForIndex(int32 NPCIndex);
    int32 ResolveTargetNPCCount() const;

    FTimerHandle DemoPollTimerHandle;
    bool bRequestedLayoutGeneration = false;
};
