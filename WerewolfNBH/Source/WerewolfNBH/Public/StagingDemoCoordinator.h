#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StagingSimulationData.h"
#include "StagingDemoCoordinator.generated.h"

class ARoomGenerator;
class ARoomModuleBase;
class AStagingDemoNPCCharacter;
class USceneComponent;
class UStagingNPCProfile;

UCLASS(Blueprintable)
class WEREWOLFNBH_API AStagingDemoCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AStagingDemoCoordinator();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Demo")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    TObjectPtr<ARoomGenerator> TargetGenerator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    TObjectPtr<UStagingNPCProfile> NPCProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    TArray<TObjectPtr<UStagingNPCProfile>> NPCProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    TSubclassOf<AStagingDemoNPCCharacter> DemoNPCClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    EStagingRunPhase Phase = EStagingRunPhase::OpeningHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    bool bTreatAsWerewolf = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    bool bGenerateLayoutIfMissing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    int32 DemoSeedOffset = 101;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo", meta=(ClampMin="1", ClampMax="16"))
    int32 NumDemoNPCs = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Timing", meta=(ClampMin="0.0"))
    float SpawnSpacing = 140.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Timing", meta=(ClampMin="0.1"))
    float LayoutPollInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Timing", meta=(ClampMin="0.0"))
    float SpawnHeightOffset = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    bool bAutoStartOnBeginPlay = true;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo")
    TObjectPtr<AStagingDemoNPCCharacter> SpawnedDemoNPC = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo")
    TArray<TObjectPtr<AStagingDemoNPCCharacter>> SpawnedDemoNPCs;

    UFUNCTION(BlueprintCallable, Category="Staging|Demo")
    void StartDemo();

protected:
    void PollForLayoutAndSpawnNPC();
    TArray<ARoomModuleBase*> GatherGeneratorRooms() const;
    ARoomGenerator* FindGeneratorInWorld() const;
    bool SpawnDemoNPCs();
    bool SpawnDemoNPC(int32 NPCIndex, const TArray<ARoomModuleBase*>& Rooms);
    FTransform BuildInitialSpawnTransform(const TArray<ARoomModuleBase*>& Rooms, int32 NPCIndex, int32 TotalNPCs) const;
    UStagingNPCProfile* ResolveProfileForIndex(int32 NPCIndex);
    int32 ResolveTargetNPCCount() const;

    FTimerHandle DemoPollTimerHandle;
    bool bRequestedLayoutGeneration = false;
};
