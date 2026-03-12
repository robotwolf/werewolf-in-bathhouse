#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrototypeRoomConnectorComponent.h"
#include "RoomModuleBase.generated.h"

class UBillboardComponent;
class UBoxComponent;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FRoomConnectionRecord
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Connection")
    TObjectPtr<UPrototypeRoomConnectorComponent> ThisConnector = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Connection")
    TObjectPtr<UPrototypeRoomConnectorComponent> OtherConnector = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Connection")
    TObjectPtr<class ARoomModuleBase> OtherRoom = nullptr;
};

UCLASS(Blueprintable)
class WEREWOLFNBH_API ARoomModuleBase : public AActor
{
    GENERATED_BODY()

public:
    ARoomModuleBase();

    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UStaticMeshComponent> RoomMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UBoxComponent> RoomBoundsBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UBillboardComponent> DebugBillboard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    FName RoomID = "Room";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    FName RoomType = "Custom";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    int32 MinConnections = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    int32 MaxConnections = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    bool bRequired = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    bool bDebugDrawBounds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    bool bExpandGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    FLinearColor DebugColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    TArray<FName> AllowedNeighborRoomTypes;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Room")
    TArray<TObjectPtr<UPrototypeRoomConnectorComponent>> DoorSockets;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    FVector RoomExtent = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    FVector RoomCenter = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TArray<FRoomConnectionRecord> ConnectedRooms;

    UFUNCTION(BlueprintCallable, Category="Room")
    void RefreshConnectorCache();

    UFUNCTION(BlueprintPure, Category="Room")
    TArray<UPrototypeRoomConnectorComponent*> GetOpenConnectors() const;

    UFUNCTION(BlueprintPure, Category="Room")
    bool AllowsNeighborType(FName CandidateRoomType) const;

    UFUNCTION(BlueprintCallable, Category="Room")
    void RegisterConnection(UPrototypeRoomConnectorComponent* ThisConnector, UPrototypeRoomConnectorComponent* OtherConnector, ARoomModuleBase* OtherRoom);

    UFUNCTION(BlueprintCallable, Category="Room")
    void SetGrayboxDimensions(const FVector& FullSize);

    FBox GetWorldBounds(float ShrinkBy = 0.0f) const;

protected:
    UPROPERTY()
    TObjectPtr<UStaticMesh> DefaultCubeMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> DefaultMaterial;

    UPrototypeRoomConnectorComponent* CreateConnector(const FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, ERoomConnectorDirection Direction);

    void UpdateGrayboxMeshScale();
};
