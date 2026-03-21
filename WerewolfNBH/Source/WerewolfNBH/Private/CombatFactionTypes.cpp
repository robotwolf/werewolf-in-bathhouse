#include "CombatFactionTypes.h"

const FName CombatFactionTags::PlayerTag(TEXT("CombatFaction.Player"));
const FName CombatFactionTags::ShooterTag(TEXT("CombatFaction.Shooter"));
const FName CombatFactionTags::MagicTag(TEXT("CombatFaction.Magic"));

FGenericTeamId CombatFactionUtilities::ToTeamId(ECombatFaction Faction)
{
    return FGenericTeamId(static_cast<uint8>(Faction));
}

ECombatFaction CombatFactionUtilities::FromTeamId(FGenericTeamId TeamId)
{
    switch (TeamId.GetId())
    {
    case static_cast<uint8>(ECombatFaction::Player):
        return ECombatFaction::Player;
    case static_cast<uint8>(ECombatFaction::Shooter):
        return ECombatFaction::Shooter;
    case static_cast<uint8>(ECombatFaction::Magic):
        return ECombatFaction::Magic;
    default:
        return ECombatFaction::None;
    }
}

FName CombatFactionUtilities::ToTag(ECombatFaction Faction)
{
    switch (Faction)
    {
    case ECombatFaction::Player:
        return CombatFactionTags::PlayerTag;
    case ECombatFaction::Shooter:
        return CombatFactionTags::ShooterTag;
    case ECombatFaction::Magic:
        return CombatFactionTags::MagicTag;
    default:
        return NAME_None;
    }
}
