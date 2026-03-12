import unreal


GENERATOR_PATH = "/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator.BP_RoomGenerator"
HUB_PATH = "/Game/WerewolfBH/Blueprints/Assembler/BP_Room_Hub.BP_Room_Hub_C"
CORRIDOR_PATH = "/Game/WerewolfBH/Blueprints/Assembler/BP_Room_Corridor.BP_Room_Corridor_C"
DEADEND_PATH = "/Game/WerewolfBH/Blueprints/Assembler/BP_Room_DeadEnd.BP_Room_DeadEnd_C"
LTURN_E_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_LTurn_E.BP_Room_PublicHall_LTurn_E_C"
LTURN_W_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_LTurn_W.BP_Room_PublicHall_LTurn_W_C"


def log(message: str) -> None:
    unreal.log(f"[configure_assembler_blueprints] {message}")


def main() -> None:
    generator_bp = unreal.load_asset(GENERATOR_PATH)
    if not generator_bp:
        raise RuntimeError(f"Could not load generator blueprint: {GENERATOR_PATH}")

    generated_class = generator_bp.generated_class()
    if not generated_class:
        raise RuntimeError("Generator blueprint has no generated class")

    cdo = unreal.get_default_object(generated_class)

    hub_class = unreal.load_class(None, HUB_PATH)
    corridor_class = unreal.load_class(None, CORRIDOR_PATH)
    deadend_class = unreal.load_class(None, DEADEND_PATH)
    lturn_e_class = unreal.load_class(None, LTURN_E_PATH)
    lturn_w_class = unreal.load_class(None, LTURN_W_PATH)

    if not hub_class or not corridor_class or not deadend_class:
        raise RuntimeError("Could not load one or more room blueprint classes")

    cdo.set_editor_property("StartRoomClass", hub_class)
    cdo.set_editor_property("DeadEndRoomClass", deadend_class)
    available_rooms = [corridor_class, deadend_class]
    if lturn_e_class:
        available_rooms.append(lturn_e_class)
    if lturn_w_class:
        available_rooms.append(lturn_w_class)
    cdo.set_editor_property("AvailableRooms", available_rooms)
    if hasattr(cdo, "ConnectorFallbackRooms"):
        cdo.set_editor_property("ConnectorFallbackRooms", [corridor_class])
    cdo.set_editor_property("RunSeed", 1337)
    cdo.set_editor_property("MaxRooms", 6)
    cdo.set_editor_property("AttemptsPerDoor", 5)
    cdo.set_editor_property("bUseNewSeedOnGenerate", True)
    cdo.set_editor_property("bDebugDrawBounds", True)
    cdo.set_editor_property("bDebugDrawDoors", True)
    cdo.set_editor_property("bPrintDebugMessages", True)
    cdo.set_editor_property("bGenerateOnBeginPlay", True)

    if hasattr(unreal, "RoomClassEntry") and hasattr(cdo, "RoomClassPool"):
        corridor_entry = unreal.RoomClassEntry()
        corridor_entry.set_editor_property("RoomClass", corridor_class)
        corridor_entry.set_editor_property("Weight", 1.0)
        corridor_entry.set_editor_property("MinRoomsBetweenUses", 0)

        deadend_entry = unreal.RoomClassEntry()
        deadend_entry.set_editor_property("RoomClass", deadend_class)
        deadend_entry.set_editor_property("Weight", 0.6)
        deadend_entry.set_editor_property("MinRoomsBetweenUses", 1)

        pool_entries = [corridor_entry, deadend_entry]

        if lturn_e_class:
            lturn_e_entry = unreal.RoomClassEntry()
            lturn_e_entry.set_editor_property("RoomClass", lturn_e_class)
            lturn_e_entry.set_editor_property("Weight", 0.9)
            lturn_e_entry.set_editor_property("MinRoomsBetweenUses", 0)
            pool_entries.append(lturn_e_entry)

        if lturn_w_class:
            lturn_w_entry = unreal.RoomClassEntry()
            lturn_w_entry.set_editor_property("RoomClass", lturn_w_class)
            lturn_w_entry.set_editor_property("Weight", 0.9)
            lturn_w_entry.set_editor_property("MinRoomsBetweenUses", 0)
            pool_entries.append(lturn_w_entry)

        cdo.set_editor_property("RoomClassPool", pool_entries)
        log("Configured RoomClassPool with corridor + dead end + L-turn entries.")

    unreal.EditorAssetLibrary.save_asset("/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator")
    log("Configured BP_RoomGenerator defaults to use Blueprint room subclasses.")


if __name__ == "__main__":
    main()
