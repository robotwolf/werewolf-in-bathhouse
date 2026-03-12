#pragma once

#include "RoomModuleBase.h"
#include "PrototypeTestRooms.generated.h"

UCLASS(Blueprintable)
class WEREWOLFNBH_API APrototypeRoomHub : public ARoomModuleBase
{
    GENERATED_BODY()

public:
    APrototypeRoomHub();
};

UCLASS(Blueprintable)
class WEREWOLFNBH_API APrototypeRoomCorridor : public ARoomModuleBase
{
    GENERATED_BODY()

public:
    APrototypeRoomCorridor();
};

UCLASS(Blueprintable)
class WEREWOLFNBH_API APrototypeRoomDeadEnd : public ARoomModuleBase
{
    GENERATED_BODY()

public:
    APrototypeRoomDeadEnd();
};
