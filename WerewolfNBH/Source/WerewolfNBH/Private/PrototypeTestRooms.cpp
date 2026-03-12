#include "PrototypeTestRooms.h"

APrototypeRoomHub::APrototypeRoomHub()
{
    RoomID = "Hub";
    RoomType = "Hub";
    Weight = 1.0f;
    MinConnections = 2;
    MaxConnections = 4;
    DebugColor = FLinearColor(0.1f, 0.8f, 0.2f);
    AllowedNeighborRoomTypes = { "Corridor", "DeadEnd" };

    SetGrayboxDimensions(FVector(800.0f, 800.0f, 340.0f));

    CreateConnector(TEXT("Conn_N"), FVector(0.0f, 400.0f, 150.0f), FRotator(0.0f, 90.0f, 0.0f), ERoomConnectorDirection::North);
    CreateConnector(TEXT("Conn_S"), FVector(0.0f, -400.0f, 150.0f), FRotator(0.0f, -90.0f, 0.0f), ERoomConnectorDirection::South);
    CreateConnector(TEXT("Conn_E"), FVector(400.0f, 0.0f, 150.0f), FRotator(0.0f, 0.0f, 0.0f), ERoomConnectorDirection::East);
    CreateConnector(TEXT("Conn_W"), FVector(-400.0f, 0.0f, 150.0f), FRotator(0.0f, 180.0f, 0.0f), ERoomConnectorDirection::West);
}

APrototypeRoomCorridor::APrototypeRoomCorridor()
{
    RoomID = "Corridor";
    RoomType = "Corridor";
    Weight = 2.0f;
    MinConnections = 2;
    MaxConnections = 2;
    DebugColor = FLinearColor(0.2f, 0.5f, 1.0f);
    AllowedNeighborRoomTypes = { "Hub", "Corridor", "DeadEnd" };

    SetGrayboxDimensions(FVector(400.0f, 1000.0f, 340.0f));

    CreateConnector(TEXT("Conn_N"), FVector(0.0f, 500.0f, 150.0f), FRotator(0.0f, 90.0f, 0.0f), ERoomConnectorDirection::North);
    CreateConnector(TEXT("Conn_S"), FVector(0.0f, -500.0f, 150.0f), FRotator(0.0f, -90.0f, 0.0f), ERoomConnectorDirection::South);
}

APrototypeRoomDeadEnd::APrototypeRoomDeadEnd()
{
    RoomID = "DeadEnd";
    RoomType = "DeadEnd";
    Weight = 1.0f;
    MinConnections = 1;
    MaxConnections = 1;
    DebugColor = FLinearColor(1.0f, 0.85f, 0.2f);
    AllowedNeighborRoomTypes = { "Hub", "Corridor" };
    bExpandGeneration = false;

    SetGrayboxDimensions(FVector(600.0f, 600.0f, 340.0f));

    CreateConnector(TEXT("Conn_S"), FVector(0.0f, -300.0f, 150.0f), FRotator(0.0f, -90.0f, 0.0f), ERoomConnectorDirection::South);
}
