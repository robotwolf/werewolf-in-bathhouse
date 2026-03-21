#include "MagicNPCCharacter.h"

#include "CombatFactionTypes.h"
#include "MagicCombatAIController.h"

AMagicNPCCharacter::AMagicNPCCharacter()
{
    AIControllerClass = AMagicCombatAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    Tags.AddUnique(CombatFactionTags::MagicTag);
}
