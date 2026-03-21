#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GideonAdmissionBooth.generated.h"

class AActor;
class USceneComponent;

UCLASS(Blueprintable)
class WEREWOLFNBH_API AGideonAdmissionBooth : public AActor
{
    GENERATED_BODY()

public:
    AGideonAdmissionBooth();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|Admission")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|Admission")
    TObjectPtr<USceneComponent> BoothWindowPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|Admission")
    TObjectPtr<USceneComponent> InteractionPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|Admission")
    TObjectPtr<USceneComponent> AdmitPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|Admission")
    TObjectPtr<USceneComponent> QueueOriginPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|Admission")
    TArray<TObjectPtr<USceneComponent>> QueueSlotPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Admission", meta=(ClampMin="0.0"))
    float QueueSlotSpacing = 110.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Admission")
    FVector QueueDirection = FVector(0.0f, -1.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Admission")
    bool bPlayerAdmissionRequired = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Admission")
    bool bAutoAdmitInSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Admission", meta=(ClampMin="0.0"))
    float AutoAdmitInterval = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Admission")
    bool bLogQueueChanges = true;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Admission")
    TArray<TObjectPtr<AActor>> AdmissionQueue;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Admission")
    TArray<TObjectPtr<AActor>> RecentlyAdmittedNPCs;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Admission")
    TObjectPtr<AActor> LastAdmittedNPC = nullptr;

    UFUNCTION(BlueprintCallable, Category="Gideon|Admission")
    void SetPlayerAdmissionRequired(bool bRequired);

    UFUNCTION(BlueprintCallable, Category="Gideon|Admission")
    void SetAutoAdmitInSimulation(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="Gideon|Admission")
    bool EnqueueNPC(AActor* NPC);

    UFUNCTION(BlueprintCallable, Category="Gideon|Admission")
    bool RemoveNPCFromQueue(AActor* NPC);

    UFUNCTION(BlueprintCallable, Category="Gideon|Admission")
    bool AdmitFrontNPC();

    UFUNCTION(BlueprintCallable, Category="Gideon|Admission")
    bool AdmitNPC(AActor* NPC);

    UFUNCTION(BlueprintCallable, Category="Gideon|Admission")
    bool RequestPlayerAdmissionForFrontNPC();

    UFUNCTION(BlueprintCallable, Category="Gideon|Admission")
    void ClearQueue();

    UFUNCTION(BlueprintCallable, Category="Gideon|Admission")
    void FlushRecentlyAdmittedNPCs();

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    bool HasQueuedNPCs() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    int32 GetQueuedNPCCount() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    AActor* GetFrontQueuedNPC() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    AActor* GetQueuedNPCAtIndex(int32 Index) const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    int32 GetRecentlyAdmittedNPCCount() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    AActor* GetRecentlyAdmittedNPCAtIndex(int32 Index) const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    bool IsNPCQueued(const AActor* NPC) const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    bool IsGameplayAdmissionRequired() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    bool IsAutoAdmitEnabled() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    FTransform GetBoothWindowTransform() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    FTransform GetInteractionPointTransform() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    FTransform GetAdmitPointTransform() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Admission")
    FTransform GetQueueSlotTransform(int32 Index) const;

protected:
    void RefreshQueueLayout();
    void ProcessAdmissionQueue(float DeltaSeconds);
    bool MoveNPCToTransform(AActor* NPC, const FTransform& TargetTransform) const;
    void LogQueueMessage(const FString& Message) const;

    float AutoAdmitAccumulator = 0.0f;
};
