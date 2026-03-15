#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ButchDecorationMarkerComponent.h"
#include "ButchDecorator.generated.h"

class ARoomGenerator;
class ARoomModuleBase;
class UAudioComponent;
class UInstancedStaticMeshComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USceneComponent;
class UStaticMesh;
class USoundBase;

UCLASS(Blueprintable)
class WEREWOLFNBH_API AButchDecorator : public AActor
{
    GENERATED_BODY()

public:
    AButchDecorator();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Butch")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Butch")
    TObjectPtr<UInstancedStaticMeshComponent> GenericMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Butch")
    TObjectPtr<UInstancedStaticMeshComponent> PipeMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Butch")
    TObjectPtr<UInstancedStaticMeshComponent> LeakMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Butch")
    TObjectPtr<UInstancedStaticMeshComponent> AudioMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Butch")
    TObjectPtr<UInstancedStaticMeshComponent> WindowMarkerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch")
    bool bDecorateOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch")
    bool bSpawnPlaceholderMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch")
    bool bDebugDrawMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch", meta=(ClampMin="0.1", ClampMax="120.0"))
    float DebugDrawDuration = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch")
    FVector MarkerScaleMultiplier = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch|Pipes", meta=(ClampMin="0.0", ClampMax="100.0"))
    float CeilingPipeDrop = 14.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch|Pipes", meta=(ClampMin="0.0", ClampMax="100.0"))
    float WallPipeInset = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch|FX")
    bool bSpawnSteamFx = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch|FX")
    bool bSpawnAudioFx = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch|FX", meta=(EditCondition="bSpawnSteamFx"))
    TObjectPtr<UParticleSystem> DefaultSteamParticle = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch|FX", meta=(EditCondition="bSpawnSteamFx", ClampMin="0.05", ClampMax="5.0"))
    float SteamScale = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch|FX", meta=(EditCondition="bSpawnSteamFx", ClampMin="-100.0", ClampMax="100.0"))
    float SteamVerticalOffset = -8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Butch|FX", meta=(EditCondition="bSpawnAudioFx"))
    TObjectPtr<USoundBase> DefaultAmbientSound = nullptr;

    UFUNCTION(BlueprintCallable, CallInEditor, Category="Butch")
    void DecorateCurrentLevel();

    UFUNCTION(BlueprintCallable, Category="Butch")
    void DecorateFromGenerator(ARoomGenerator* Generator);

    UFUNCTION(BlueprintCallable, CallInEditor, Category="Butch")
    void ClearDecor();

protected:
    UPROPERTY()
    TObjectPtr<UStaticMesh> DefaultCubeMesh;

    UPROPERTY()
    TObjectPtr<UStaticMesh> DefaultCylinderMesh;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UParticleSystemComponent>> SpawnedSteamComponents;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UAudioComponent>> SpawnedAudioComponents;

    void ConfigurePlaceholderMesh(UInstancedStaticMeshComponent* MeshComponent) const;
    void ApplyPlaceholderColors();
    TArray<ARoomModuleBase*> CollectRoomsFromLevel() const;
    void ProcessRooms(const TArray<ARoomModuleBase*>& Rooms);
    void AddMarker(UButchDecorationMarkerComponent* Marker);
};
