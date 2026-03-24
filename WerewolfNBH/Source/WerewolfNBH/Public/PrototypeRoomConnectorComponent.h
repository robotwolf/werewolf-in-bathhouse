#pragma once

#include "Components/SceneComponent.h"
#include "PrototypeRoomConnectorComponent.generated.h"

class UArrowComponent;
class ARoomModuleBase;
class UGinnyOpeningProfile;

UENUM(BlueprintType)
enum class ERoomConnectorDirection : uint8
{
    North,
    South,
    East,
    West,
    Up,
    Down
};

UENUM(BlueprintType)
enum class ERoomConnectionType : uint8
{
    Public,
    Staff,
    Hidden,
    Any
};

UENUM(BlueprintType)
enum class ERoomConnectorPassageKind : uint8
{
    InteriorDoor,
    ExteriorDoor,
    OpenThreshold,
    Footpath,
    RoadLink,
    StairHandoff,
    ServiceHatch,
    Any
};

UENUM(BlueprintType)
enum class ERoomConnectorBoundaryKind : uint8
{
    Interior,
    Exterior,
    Transition,
    Any
};

UENUM(BlueprintType)
enum class ERoomConnectorClearanceClass : uint8
{
    HumanStandard,
    HumanWide,
    Service,
    Vehicle,
    Any
};

UCLASS(ClassGroup=(Staging), meta=(BlueprintSpawnableComponent))
class WEREWOLFNBH_API UPrototypeRoomConnectorComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UPrototypeRoomConnectorComponent();
    virtual void OnRegister() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Connector")
    TObjectPtr<UArrowComponent> ArrowComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector|Debug")
    bool bHideArrowInGame = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector")
    FName SocketID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector")
    ERoomConnectorDirection Direction = ERoomConnectorDirection::North;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector")
    ERoomConnectionType ConnectionType = ERoomConnectionType::Public;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector|Contract")
    ERoomConnectorPassageKind PassageKind = ERoomConnectorPassageKind::InteriorDoor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector|Contract")
    ERoomConnectorBoundaryKind BoundaryKind = ERoomConnectorBoundaryKind::Interior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector|Contract")
    ERoomConnectorClearanceClass ClearanceClass = ERoomConnectorClearanceClass::HumanStandard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector|Contract")
    FName ContractTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector")
    bool bOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector")
    bool bCanConnectToPublic = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector")
    bool bCanConnectToStaff = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector")
    bool bCanConnectToHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector")
    bool bAllowDeadEndFallback = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector")
    bool bCanMoonriseShift = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Connector|Openings")
    TObjectPtr<UGinnyOpeningProfile> OpeningProfileOverride = nullptr;

    UFUNCTION(BlueprintPure, Category="Staging|Connector")
    bool IsCompatibleWith(const UPrototypeRoomConnectorComponent* Other) const;

    bool IsCompatibleWith(const UPrototypeRoomConnectorComponent* Other, FString* OutReason) const;

    UFUNCTION(BlueprintPure, Category="Staging|Connector")
    ARoomModuleBase* GetOwningRoom() const;

    UFUNCTION(BlueprintCallable, Category="Staging|Connector|Debug")
    void UpdateDebugAppearance();

    UFUNCTION(BlueprintPure, Category="Staging|Connector|Debug")
    FLinearColor GetDebugColor() const;
};
