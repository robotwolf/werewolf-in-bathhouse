import unreal


GENERATOR_PATH = "/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator.BP_RoomGenerator"
ENTRY_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_EntryReception.BP_Room_EntryReception_C"
LOCKER_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_LockerHall.BP_Room_LockerHall_C"
PUBLIC_HALL_STRAIGHT_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Straight.BP_Room_PublicHall_Straight_C"
PUBLIC_HALL_CORNER_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Corner.BP_Room_PublicHall_Corner_C"
PUBLIC_HALL_STAIR_UP_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Stair_Up.BP_Room_PublicHall_Stair_Up_C"
LTURN_E_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_LTurn_E.BP_Room_PublicHall_LTurn_E_C"
LTURN_W_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_LTurn_W.BP_Room_PublicHall_LTurn_W_C"


def log(message: str) -> None:
    unreal.log(f"[configure_assembler_blueprints] {message}")


def append_unique(items, new_item):
    if not new_item:
        return
    if new_item not in items:
        items.append(new_item)


def main() -> None:
    generator_bp = unreal.load_asset(GENERATOR_PATH)
    if not generator_bp:
        raise RuntimeError(f"Could not load generator blueprint: {GENERATOR_PATH}")

    generated_class = generator_bp.generated_class()
    if not generated_class:
        raise RuntimeError("Generator blueprint has no generated class")

    cdo = unreal.get_default_object(generated_class)

    entry_class = unreal.load_class(None, ENTRY_PATH)
    locker_class = unreal.load_class(None, LOCKER_PATH)
    public_hall_straight_class = unreal.load_class(None, PUBLIC_HALL_STRAIGHT_PATH)
    public_hall_corner_class = unreal.load_class(None, PUBLIC_HALL_CORNER_PATH)
    public_hall_stair_up_class = unreal.load_class(None, PUBLIC_HALL_STAIR_UP_PATH)
    lturn_e_class = unreal.load_class(None, LTURN_E_PATH)
    lturn_w_class = unreal.load_class(None, LTURN_W_PATH)

    if not entry_class or not locker_class or not public_hall_straight_class or not public_hall_corner_class or not public_hall_stair_up_class:
        raise RuntimeError("Could not load one or more bathhouse room blueprint classes")

    primary_hall_class = public_hall_straight_class

    cdo.set_editor_property("StartRoomClass", entry_class)
    cdo.set_editor_property("DeadEndRoomClass", None)
    available_rooms = []
    append_unique(available_rooms, locker_class)
    append_unique(available_rooms, primary_hall_class)
    append_unique(available_rooms, public_hall_corner_class)
    append_unique(available_rooms, public_hall_stair_up_class)
    cdo.set_editor_property("AvailableRooms", available_rooms)
    if hasattr(cdo, "ConnectorFallbackRooms"):
        fallback_rooms = []
        append_unique(fallback_rooms, primary_hall_class)
        append_unique(fallback_rooms, public_hall_corner_class)
        cdo.set_editor_property("ConnectorFallbackRooms", fallback_rooms)
    if hasattr(cdo, "bEnableHallwayChains"):
        cdo.set_editor_property("bEnableHallwayChains", True)
    if hasattr(cdo, "MaxHallwayChainSegments"):
        cdo.set_editor_property("MaxHallwayChainSegments", 3)
    if hasattr(cdo, "bRunButchAfterGeneration"):
        cdo.set_editor_property("bRunButchAfterGeneration", True)
    if hasattr(cdo, "bSpawnButchIfMissing"):
        cdo.set_editor_property("bSpawnButchIfMissing", True)
    if hasattr(cdo, "ButchDecoratorClass"):
        cdo.set_editor_property("ButchDecoratorClass", unreal.ButchDecorator)
    cdo.set_editor_property("RunSeed", 1337)
    cdo.set_editor_property("MaxRooms", 6)
    cdo.set_editor_property("AttemptsPerDoor", 5)
    if hasattr(cdo, "VerticalSnapSize"):
        cdo.set_editor_property("VerticalSnapSize", 10.0)
    if hasattr(cdo, "bAllowVerticalTransitions"):
        cdo.set_editor_property("bAllowVerticalTransitions", True)
    if hasattr(cdo, "MaxVerticalDisplacement"):
        cdo.set_editor_property("MaxVerticalDisplacement", 420.0)
    cdo.set_editor_property("bUseNewSeedOnGenerate", True)
    cdo.set_editor_property("bDebugDrawBounds", True)
    cdo.set_editor_property("bDebugDrawDoors", True)
    cdo.set_editor_property("bPrintDebugMessages", True)
    if hasattr(cdo, "bOverrideRoomSliceDebug"):
        cdo.set_editor_property("bOverrideRoomSliceDebug", False)
    if hasattr(cdo, "bGlobalSliceDebugEnabled"):
        cdo.set_editor_property("bGlobalSliceDebugEnabled", False)
    if hasattr(cdo, "GlobalSliceDebugDuration"):
        cdo.set_editor_property("GlobalSliceDebugDuration", 8.0)
    cdo.set_editor_property("bGenerateOnBeginPlay", True)

    if hasattr(unreal, "RoomClassEntry") and hasattr(cdo, "RoomClassPool"):
        pool_entries = unreal.Array(unreal.RoomClassEntry)

        locker_entry = unreal.RoomClassEntry()
        locker_entry.set_editor_property("RoomClass", locker_class)
        locker_entry.set_editor_property("Weight", 1.0)
        locker_entry.set_editor_property("MinRoomsBetweenUses", 1)

        hall_entry = unreal.RoomClassEntry()
        hall_entry.set_editor_property("RoomClass", primary_hall_class)
        hall_entry.set_editor_property("Weight", 1.3)
        hall_entry.set_editor_property("MinRoomsBetweenUses", 0)

        corner_entry = unreal.RoomClassEntry()
        corner_entry.set_editor_property("RoomClass", public_hall_corner_class)
        corner_entry.set_editor_property("Weight", 0.9)
        corner_entry.set_editor_property("MinRoomsBetweenUses", 0)

        stair_entry = unreal.RoomClassEntry()
        stair_entry.set_editor_property("RoomClass", public_hall_stair_up_class)
        stair_entry.set_editor_property("Weight", 0.35)
        stair_entry.set_editor_property("MinRoomsBetweenUses", 2)

        pool_entries.append(locker_entry)
        pool_entries.append(hall_entry)
        pool_entries.append(corner_entry)
        pool_entries.append(stair_entry)

        cdo.set_editor_property("RoomClassPool", pool_entries)
        readback = cdo.get_editor_property("RoomClassPool")
        debug_classes = []
        for entry in readback:
            room_class = entry.get_editor_property("RoomClass")
            debug_classes.append(room_class.get_path_name() if room_class else "None")
        log(f"Configured RoomClassPool with bathhouse rooms only: {debug_classes}")

    unreal.BlueprintEditorLibrary.compile_blueprint(generator_bp)
    unreal.EditorAssetLibrary.save_asset("/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator")
    log("Configured BP_RoomGenerator defaults to use bathhouse room subclasses.")


if __name__ == "__main__":
    main()
