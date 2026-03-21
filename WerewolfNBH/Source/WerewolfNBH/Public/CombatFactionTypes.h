#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "CombatFactionTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatFaction : uint8
{
    None = 0,
    Player = 1,
    Shooter = 2,
    Magic = 3,
};

namespace CombatFactionTags
{
    extern WEREWOLFNBH_API const FName PlayerTag;
    extern WEREWOLFNBH_API const FName ShooterTag;
    extern WEREWOLFNBH_API const FName MagicTag;
}

namespace CombatFactionUtilities
{
    WEREWOLFNBH_API FGenericTeamId ToTeamId(ECombatFaction Faction);
    WEREWOLFNBH_API ECombatFaction FromTeamId(FGenericTeamId TeamId);
    WEREWOLFNBH_API FName ToTag(ECombatFaction Faction);
}
