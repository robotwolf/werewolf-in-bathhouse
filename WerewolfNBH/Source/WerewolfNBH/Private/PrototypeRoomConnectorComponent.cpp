#include "PrototypeRoomConnectorComponent.h"

#include "Components/ArrowComponent.h"
#include "RoomModuleBase.h"

UPrototypeRoomConnectorComponent::UPrototypeRoomConnectorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
    ArrowComponent->SetupAttachment(this);
    ArrowComponent->ArrowColor = FColor::Cyan;
    ArrowComponent->ArrowLength = 120.0f;
    ArrowComponent->bIsScreenSizeScaled = true;
    ArrowComponent->SetHiddenInGame(bHideArrowInGame);
    ArrowComponent->SetVisibility(true);
}

void UPrototypeRoomConnectorComponent::OnRegister()
{
    Super::OnRegister();

    if (ArrowComponent)
    {
        ArrowComponent->SetHiddenInGame(bHideArrowInGame);
        ArrowComponent->SetVisibility(true);
    }
}

bool UPrototypeRoomConnectorComponent::IsCompatibleWith(const UPrototypeRoomConnectorComponent* Other) const
{
    if (!Other || Other == this || bOccupied || Other->bOccupied)
    {
        return false;
    }

    const bool bTypeMatch =
        ConnectionType == ERoomConnectionType::Any ||
        Other->ConnectionType == ERoomConnectionType::Any ||
        ConnectionType == Other->ConnectionType;

    if (!bTypeMatch)
    {
        return false;
    }

    switch (Other->ConnectionType)
    {
    case ERoomConnectionType::Public:
        return bCanConnectToPublic;
    case ERoomConnectionType::Staff:
        return bCanConnectToStaff;
    case ERoomConnectionType::Hidden:
        return bCanConnectToHidden;
    case ERoomConnectionType::Any:
    default:
        return true;
    }
}

ARoomModuleBase* UPrototypeRoomConnectorComponent::GetOwningRoom() const
{
    return Cast<ARoomModuleBase>(GetOwner());
}
