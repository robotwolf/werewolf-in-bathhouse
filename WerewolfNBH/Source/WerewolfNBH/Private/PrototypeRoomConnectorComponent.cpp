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
    UpdateDebugAppearance();
}

void UPrototypeRoomConnectorComponent::OnRegister()
{
    Super::OnRegister();

    if (ArrowComponent)
    {
        UpdateDebugAppearance();
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

void UPrototypeRoomConnectorComponent::UpdateDebugAppearance()
{
    if (!ArrowComponent)
    {
        return;
    }

    ArrowComponent->ArrowColor = GetDebugColor().ToFColor(true);

    switch (ClearanceClass)
    {
    case ERoomConnectorClearanceClass::HumanWide:
        ArrowComponent->ArrowSize = 1.35f;
        ArrowComponent->ArrowLength = 140.0f;
        break;
    case ERoomConnectorClearanceClass::Service:
        ArrowComponent->ArrowSize = 1.15f;
        ArrowComponent->ArrowLength = 110.0f;
        break;
    case ERoomConnectorClearanceClass::Vehicle:
        ArrowComponent->ArrowSize = 1.6f;
        ArrowComponent->ArrowLength = 160.0f;
        break;
    case ERoomConnectorClearanceClass::Any:
        ArrowComponent->ArrowSize = 1.0f;
        ArrowComponent->ArrowLength = 100.0f;
        break;
    case ERoomConnectorClearanceClass::HumanStandard:
    default:
        ArrowComponent->ArrowSize = 1.2f;
        ArrowComponent->ArrowLength = 120.0f;
        break;
    }
}

FLinearColor UPrototypeRoomConnectorComponent::GetDebugColor() const
{
    switch (PassageKind)
    {
    case ERoomConnectorPassageKind::InteriorDoor:
        return FLinearColor(0.15f, 0.85f, 1.0f, 1.0f);
    case ERoomConnectorPassageKind::ExteriorDoor:
        return FLinearColor(0.2f, 1.0f, 0.45f, 1.0f);
    case ERoomConnectorPassageKind::OpenThreshold:
        return FLinearColor(1.0f, 0.85f, 0.2f, 1.0f);
    case ERoomConnectorPassageKind::Footpath:
        return FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    case ERoomConnectorPassageKind::RoadLink:
        return FLinearColor(0.85f, 0.25f, 1.0f, 1.0f);
    case ERoomConnectorPassageKind::StairHandoff:
        return FLinearColor(0.6f, 0.55f, 1.0f, 1.0f);
    case ERoomConnectorPassageKind::ServiceHatch:
        return FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
    case ERoomConnectorPassageKind::Any:
    default:
        return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
    }
}
