#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ButchDecorationMarkerComponent.h"
#include "ButchDecorator.generated.h"

class ARoomGenerator;
class ARoomModuleBase;
class UInstancedStaticMeshComponent;
class USceneComponent;
class UStaticMesh;

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

    void ConfigurePlaceholderMesh(UInstancedStaticMeshComponent* MeshComponent) const;
    void ApplyPlaceholderColors();
    TArray<ARoomModuleBase*> CollectRoomsFromLevel() const;
    void ProcessRooms(const TArray<ARoomModuleBase*>& Rooms);
    void AddMarker(UButchDecorationMarkerComponent* Marker);
};
