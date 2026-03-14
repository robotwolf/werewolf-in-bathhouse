#pragma once

#include "Components/SceneComponent.h"
#include "PrototypeRoomConnectorComponent.generated.h"

class UArrowComponent;
class ARoomModuleBase;

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

UCLASS(ClassGroup=(WerewolfBH), meta=(BlueprintSpawnableComponent))
class WEREWOLFNBH_API UPrototypeRoomConnectorComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UPrototypeRoomConnectorComponent();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Connector")
    TObjectPtr<UArrowComponent> ArrowComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Connector")
    FName SocketID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Connector")
    ERoomConnectorDirection Direction = ERoomConnectorDirection::North;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Connector")
    ERoomConnectionType ConnectionType = ERoomConnectionType::Public;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Connector")
    bool bOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Connector")
    bool bCanConnectToPublic = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Connector")
    bool bCanConnectToStaff = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Connector")
    bool bCanConnectToHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Connector")
    bool bAllowDeadEndFallback = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Connector")
    bool bCanMoonriseShift = false;

    UFUNCTION(BlueprintPure, Category="Connector")
    bool IsCompatibleWith(const UPrototypeRoomConnectorComponent* Other) const;

    UFUNCTION(BlueprintPure, Category="Connector")
    ARoomModuleBase* GetOwningRoom() const;
};
