#include "MagicGameMode.h"

#include "MagicCasterCharacter.h"
#include "MagicPlayerController.h"

AMagicGameMode::AMagicGameMode()
{
    DefaultPawnClass = AMagicCasterCharacter::StaticClass();
    PlayerControllerClass = AMagicPlayerController::StaticClass();
}
