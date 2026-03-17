#include "PrototypeRoomConnectorComponent.h"

#include "Components/ArrowComponent.h"
#include "RoomModuleBase.h"

namespace
{
    bool IsPassageCompatible(ERoomConnectorPassageKind A, ERoomConnectorPassageKind B)
    {
        if (A == ERoomConnectorPassageKind::Any || B == ERoomConnectorPassageKind::Any || A == B)
        {
            return true;
        }

        const bool bAOpen = A == ERoomConnectorPassageKind::OpenThreshold;
        const bool bBOpen = B == ERoomConnectorPassageKind::OpenThreshold;
        if (bAOpen || bBOpen)
        {
            const ERoomConnectorPassageKind Other = bAOpen ? B : A;
            return Other != ERoomConnectorPassageKind::RoadLink;
        }

        const bool bDoorToPath =
            (A == ERoomConnectorPassageKind::ExteriorDoor && B == ERoomConnectorPassageKind::Footpath) ||
            (B == ERoomConnectorPassageKind::ExteriorDoor && A == ERoomConnectorPassageKind::Footpath);
        if (bDoorToPath)
        {
            return true;
        }

        return false;
    }

    bool IsBoundaryCompatible(ERoomConnectorBoundaryKind A, ERoomConnectorBoundaryKind B)
    {
        return A == ERoomConnectorBoundaryKind::Any ||
            B == ERoomConnectorBoundaryKind::Any ||
            A == B;
    }

    bool IsClearanceCompatible(ERoomConnectorClearanceClass A, ERoomConnectorClearanceClass B)
    {
        if (A == ERoomConnectorClearanceClass::Any || B == ERoomConnectorClearanceClass::Any || A == B)
        {
            return true;
        }

        const bool bHumanMix =
            (A == ERoomConnectorClearanceClass::HumanStandard && B == ERoomConnectorClearanceClass::HumanWide) ||
            (B == ERoomConnectorClearanceClass::HumanStandard && A == ERoomConnectorClearanceClass::HumanWide);
        return bHumanMix;
    }
}

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
    return IsCompatibleWith(Other, nullptr);
}

bool UPrototypeRoomConnectorComponent::IsCompatibleWith(const UPrototypeRoomConnectorComponent* Other, FString* OutReason) const
{
    if (!Other || Other == this || bOccupied || Other->bOccupied)
    {
        if (OutReason)
        {
            *OutReason = TEXT("Occupied");
        }
        return false;
    }

    const bool bTypeMatch =
        ConnectionType == ERoomConnectionType::Any ||
        Other->ConnectionType == ERoomConnectionType::Any ||
        ConnectionType == Other->ConnectionType;

    if (!bTypeMatch)
    {
        if (OutReason)
        {
            *OutReason = TEXT("ConnectionType");
        }
        return false;
    }

    if (!IsPassageCompatible(PassageKind, Other->PassageKind))
    {
        if (OutReason)
        {
            *OutReason = TEXT("PassageKind");
        }
        return false;
    }

    if (!IsBoundaryCompatible(BoundaryKind, Other->BoundaryKind))
    {
        if (OutReason)
        {
            *OutReason = TEXT("BoundaryKind");
        }
        return false;
    }

    if (!IsClearanceCompatible(ClearanceClass, Other->ClearanceClass))
    {
        if (OutReason)
        {
            *OutReason = TEXT("ClearanceClass");
        }
        return false;
    }

    if (ContractTag != NAME_None && Other->ContractTag != NAME_None && ContractTag != Other->ContractTag)
    {
        if (OutReason)
        {
            *OutReason = TEXT("ContractTag");
        }
        return false;
    }

    switch (Other->ConnectionType)
    {
    case ERoomConnectionType::Public:
        if (!bCanConnectToPublic)
        {
            if (OutReason)
            {
                *OutReason = TEXT("PublicAccess");
            }
            return false;
        }
        return true;
    case ERoomConnectionType::Staff:
        if (!bCanConnectToStaff)
        {
            if (OutReason)
            {
                *OutReason = TEXT("StaffAccess");
            }
            return false;
        }
        return true;
    case ERoomConnectionType::Hidden:
        if (!bCanConnectToHidden)
        {
            if (OutReason)
            {
                *OutReason = TEXT("HiddenAccess");
            }
            return false;
        }
        return true;
    case ERoomConnectionType::Any:
    default:
        return true;
    }
}

ARoomModuleBase* UPrototypeRoomConnectorComponent::GetOwningRoom() const
{
    return Cast<ARoomModuleBase>(GetOwner());
}
