import unreal


GENERATOR_PATH = "/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator.BP_RoomGenerator"
ASSEMBLER_BP_PATH = "/Game/WerewolfBH/Blueprints/Assembler"
BUTCH_BP_ASSET = "BP_ButchDecorator"
BUTCH_BP_OBJECT_PATH = f"{ASSEMBLER_BP_PATH}/{BUTCH_BP_ASSET}"
BUTCH_BP_PATH = f"{BUTCH_BP_OBJECT_PATH}.{BUTCH_BP_ASSET}"
ENTRY_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_EntryReception.BP_Room_EntryReception_C"
LOCKER_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_LockerHall.BP_Room_LockerHall_C"
WASH_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_WashShower.BP_Room_WashShower_C"
POOL_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PoolHall.BP_Room_PoolHall_C"
SAUNA_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Sauna.BP_Room_Sauna_C"
BOILER_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_BoilerService.BP_Room_BoilerService_C"
PLUNGE_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_ColdPlunge.BP_Room_ColdPlunge_C"
STEAM_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_SteamRoom.BP_Room_SteamRoom_C"
TOILET_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Toilet.BP_Room_Toilet_C"
STORAGE_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Storage.BP_Room_Storage_C"
PUBLIC_HALL_STRAIGHT_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Straight.BP_Room_PublicHall_Straight_C"
PUBLIC_HALL_CORNER_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Corner.BP_Room_PublicHall_Corner_C"
PUBLIC_HALL_LTURN_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_LTurn_E.BP_Room_PublicHall_LTurn_E_C"
PUBLIC_HALL_STAIR_UP_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Stair_Up.BP_Room_PublicHall_Stair_Up_C"


def log(message: str) -> None:
    unreal.log(f"[configure_assembler_blueprints] {message}")


def append_unique(items, new_item):
    if not new_item:
        return
    if new_item not in items:
        items.append(new_item)


def has_editor_property(obj, property_name: str) -> bool:
    try:
        obj.get_editor_property(property_name)
        return True
    except Exception:
        return False


def ensure_blueprint(asset_object_path, asset_name, parent_class):
    asset = unreal.load_asset(asset_object_path)
    if asset:
        return asset

    if not unreal.EditorAssetLibrary.does_directory_exist(ASSEMBLER_BP_PATH):
        unreal.EditorAssetLibrary.make_directory(ASSEMBLER_BP_PATH)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", parent_class)
    created = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, ASSEMBLER_BP_PATH, unreal.Blueprint, factory)
    if not created:
        raise RuntimeError(f"Could not create blueprint {asset_object_path}")
    unreal.BlueprintEditorLibrary.compile_blueprint(created)
    unreal.EditorAssetLibrary.save_loaded_asset(created)
    return created


def main() -> None:
    generator_bp = unreal.load_asset(GENERATOR_PATH)
    if not generator_bp:
        raise RuntimeError(f"Could not load generator blueprint: {GENERATOR_PATH}")

    generated_class = generator_bp.generated_class()
    if not generated_class:
        raise RuntimeError("Generator blueprint has no generated class")

    cdo = unreal.get_default_object(generated_class)
    butch_bp = ensure_blueprint(BUTCH_BP_OBJECT_PATH, BUTCH_BP_ASSET, unreal.ButchDecorator)

    entry_class = unreal.load_class(None, ENTRY_PATH)
    locker_class = unreal.load_class(None, LOCKER_PATH)
    wash_class = unreal.load_class(None, WASH_PATH)
    pool_class = unreal.load_class(None, POOL_PATH)
    sauna_class = unreal.load_class(None, SAUNA_PATH)
    boiler_class = unreal.load_class(None, BOILER_PATH)
    plunge_class = unreal.load_class(None, PLUNGE_PATH)
    steam_class = unreal.load_class(None, STEAM_PATH)
    toilet_class = unreal.load_class(None, TOILET_PATH)
    storage_class = unreal.load_class(None, STORAGE_PATH)
    public_hall_straight_class = unreal.load_class(None, PUBLIC_HALL_STRAIGHT_PATH)
    public_hall_corner_class = unreal.load_class(None, PUBLIC_HALL_CORNER_PATH)
    public_hall_lturn_class = unreal.load_class(None, PUBLIC_HALL_LTURN_PATH)
    public_hall_stair_up_class = unreal.load_class(None, PUBLIC_HALL_STAIR_UP_PATH)

    if not entry_class or not locker_class or not wash_class or not pool_class or not sauna_class or not boiler_class or not plunge_class or not steam_class or not toilet_class or not storage_class or not public_hall_straight_class or not public_hall_corner_class or not public_hall_lturn_class:
        raise RuntimeError("Could not load one or more bathhouse room blueprint classes")

    primary_hall_class = public_hall_straight_class

    cdo.set_editor_property("StartRoomClass", entry_class)
    cdo.set_editor_property("DeadEndRoomClass", None)
    available_rooms = []
    append_unique(available_rooms, locker_class)
    append_unique(available_rooms, wash_class)
    append_unique(available_rooms, pool_class)
    append_unique(available_rooms, sauna_class)
    append_unique(available_rooms, boiler_class)
    append_unique(available_rooms, plunge_class)
    append_unique(available_rooms, steam_class)
    append_unique(available_rooms, toilet_class)
    append_unique(available_rooms, storage_class)
    append_unique(available_rooms, public_hall_stair_up_class)
    append_unique(available_rooms, primary_hall_class)
    append_unique(available_rooms, public_hall_corner_class)
    append_unique(available_rooms, public_hall_lturn_class)
    cdo.set_editor_property("AvailableRooms", available_rooms)
    if has_editor_property(cdo, "ConnectorFallbackRooms"):
        fallback_rooms = []
        append_unique(fallback_rooms, primary_hall_class)
        append_unique(fallback_rooms, public_hall_corner_class)
        append_unique(fallback_rooms, public_hall_lturn_class)
        cdo.set_editor_property("ConnectorFallbackRooms", fallback_rooms)
    if has_editor_property(cdo, "bEnableHallwayChains"):
        cdo.set_editor_property("bEnableHallwayChains", True)
    if has_editor_property(cdo, "MaxHallwayChainSegments"):
        cdo.set_editor_property("MaxHallwayChainSegments", 3)
    if has_editor_property(cdo, "bUseIntentionalHallApproaches"):
        cdo.set_editor_property("bUseIntentionalHallApproaches", True)
    if has_editor_property(cdo, "MinHallwayApproachSegments"):
        cdo.set_editor_property("MinHallwayApproachSegments", 1)
    if has_editor_property(cdo, "MaxHallwayApproachSegments"):
        cdo.set_editor_property("MaxHallwayApproachSegments", 3)
    if has_editor_property(cdo, "HallwayExtraSegmentChance"):
        cdo.set_editor_property("HallwayExtraSegmentChance", 0.55)
    if has_editor_property(cdo, "bAllowIntentionalApproachesOnMainPath"):
        cdo.set_editor_property("bAllowIntentionalApproachesOnMainPath", True)
    if has_editor_property(cdo, "bAllowIntentionalApproachesOnBranches"):
        cdo.set_editor_property("bAllowIntentionalApproachesOnBranches", True)
    if has_editor_property(cdo, "StraightHallWeight"):
        cdo.set_editor_property("StraightHallWeight", 1.0)
    if has_editor_property(cdo, "CornerHallWeight"):
        cdo.set_editor_property("CornerHallWeight", 0.85)
    if has_editor_property(cdo, "LTurnHallWeight"):
        cdo.set_editor_property("LTurnHallWeight", 1.35)
    if has_editor_property(cdo, "RequiredMainPathRooms"):
        cdo.set_editor_property("RequiredMainPathRooms", [primary_hall_class, locker_class, wash_class, primary_hall_class, pool_class])
    if has_editor_property(cdo, "RequiredBranchRooms"):
        cdo.set_editor_property("RequiredBranchRooms", [sauna_class, boiler_class])
    if has_editor_property(cdo, "bRunButchAfterGeneration"):
        cdo.set_editor_property("bRunButchAfterGeneration", False)
    if has_editor_property(cdo, "bSpawnButchIfMissing"):
        cdo.set_editor_property("bSpawnButchIfMissing", False)
    if has_editor_property(cdo, "ButchDecoratorClass"):
        cdo.set_editor_property("ButchDecoratorClass", butch_bp.generated_class())
    cdo.set_editor_property("RunSeed", 1337)
    cdo.set_editor_property("MaxRooms", 10)
    cdo.set_editor_property("AttemptsPerDoor", 5)
    if has_editor_property(cdo, "MaxLayoutAttempts"):
        cdo.set_editor_property("MaxLayoutAttempts", 5)
    if has_editor_property(cdo, "VerticalSnapSize"):
        cdo.set_editor_property("VerticalSnapSize", 10.0)
    if has_editor_property(cdo, "bAllowVerticalTransitions"):
        cdo.set_editor_property("bAllowVerticalTransitions", False)
    if has_editor_property(cdo, "MaxVerticalDisplacement"):
        cdo.set_editor_property("MaxVerticalDisplacement", 420.0)
    cdo.set_editor_property("bUseNewSeedOnGenerate", False)
    cdo.set_editor_property("bDebugDrawBounds", True)
    cdo.set_editor_property("bDebugDrawDoors", True)
    cdo.set_editor_property("bPrintDebugMessages", True)
    if has_editor_property(cdo, "bOverrideRoomSliceDebug"):
        cdo.set_editor_property("bOverrideRoomSliceDebug", False)
    if has_editor_property(cdo, "bGlobalSliceDebugEnabled"):
        cdo.set_editor_property("bGlobalSliceDebugEnabled", False)
    if has_editor_property(cdo, "GlobalSliceDebugDuration"):
        cdo.set_editor_property("GlobalSliceDebugDuration", 8.0)
    cdo.set_editor_property("bGenerateOnBeginPlay", True)

    if hasattr(unreal, "RoomClassEntry") and has_editor_property(cdo, "RoomClassPool"):
        pool_entries = unreal.Array(unreal.RoomClassEntry)

        locker_entry = unreal.RoomClassEntry()
        locker_entry.set_editor_property("RoomClass", locker_class)
        locker_entry.set_editor_property("Weight", 0.65)
        locker_entry.set_editor_property("MinRoomsBetweenUses", 1)

        wash_entry = unreal.RoomClassEntry()
        wash_entry.set_editor_property("RoomClass", wash_class)
        wash_entry.set_editor_property("Weight", 0.7)
        wash_entry.set_editor_property("MinRoomsBetweenUses", 2)

        pool_entry = unreal.RoomClassEntry()
        pool_entry.set_editor_property("RoomClass", pool_class)
        pool_entry.set_editor_property("Weight", 0.55)
        pool_entry.set_editor_property("MinRoomsBetweenUses", 3)

        sauna_entry = unreal.RoomClassEntry()
        sauna_entry.set_editor_property("RoomClass", sauna_class)
        sauna_entry.set_editor_property("Weight", 0.45)
        sauna_entry.set_editor_property("MinRoomsBetweenUses", 2)

        boiler_entry = unreal.RoomClassEntry()
        boiler_entry.set_editor_property("RoomClass", boiler_class)
        boiler_entry.set_editor_property("Weight", 0.25)
        boiler_entry.set_editor_property("MinRoomsBetweenUses", 3)

        plunge_entry = unreal.RoomClassEntry()
        plunge_entry.set_editor_property("RoomClass", plunge_class)
        plunge_entry.set_editor_property("Weight", 0.38)
        plunge_entry.set_editor_property("MinRoomsBetweenUses", 3)

        steam_entry = unreal.RoomClassEntry()
        steam_entry.set_editor_property("RoomClass", steam_class)
        steam_entry.set_editor_property("Weight", 0.36)
        steam_entry.set_editor_property("MinRoomsBetweenUses", 3)

        toilet_entry = unreal.RoomClassEntry()
        toilet_entry.set_editor_property("RoomClass", toilet_class)
        toilet_entry.set_editor_property("Weight", 0.28)
        toilet_entry.set_editor_property("MinRoomsBetweenUses", 2)

        storage_entry = unreal.RoomClassEntry()
        storage_entry.set_editor_property("RoomClass", storage_class)
        storage_entry.set_editor_property("Weight", 0.24)
        storage_entry.set_editor_property("MinRoomsBetweenUses", 2)

        stair_entry = unreal.RoomClassEntry()
        stair_entry.set_editor_property("RoomClass", public_hall_stair_up_class)
        stair_entry.set_editor_property("Weight", 0.18)
        stair_entry.set_editor_property("MinRoomsBetweenUses", 4)

        hall_entry = unreal.RoomClassEntry()
        hall_entry.set_editor_property("RoomClass", primary_hall_class)
        hall_entry.set_editor_property("Weight", 1.4)
        hall_entry.set_editor_property("MinRoomsBetweenUses", 0)

        corner_entry = unreal.RoomClassEntry()
        corner_entry.set_editor_property("RoomClass", public_hall_corner_class)
        corner_entry.set_editor_property("Weight", 1.0)
        corner_entry.set_editor_property("MinRoomsBetweenUses", 0)

        lturn_entry = unreal.RoomClassEntry()
        lturn_entry.set_editor_property("RoomClass", public_hall_lturn_class)
        lturn_entry.set_editor_property("Weight", 0.95)
        lturn_entry.set_editor_property("MinRoomsBetweenUses", 0)

        pool_entries.append(locker_entry)
        pool_entries.append(wash_entry)
        pool_entries.append(pool_entry)
        pool_entries.append(sauna_entry)
        pool_entries.append(boiler_entry)
        pool_entries.append(plunge_entry)
        pool_entries.append(steam_entry)
        pool_entries.append(toilet_entry)
        pool_entries.append(storage_entry)
        pool_entries.append(stair_entry)
        pool_entries.append(hall_entry)
        pool_entries.append(corner_entry)
        pool_entries.append(lturn_entry)

        cdo.set_editor_property("RoomClassPool", pool_entries)
        readback = cdo.get_editor_property("RoomClassPool")
        debug_classes = []
        for entry in readback:
            room_class = entry.get_editor_property("RoomClass")
            debug_classes.append(room_class.get_path_name() if room_class else "None")
        log(f"Configured RoomClassPool with bathhouse rooms only: {debug_classes}")

    unreal.BlueprintEditorLibrary.compile_blueprint(generator_bp)
    unreal.EditorAssetLibrary.save_asset("/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator")
    unreal.BlueprintEditorLibrary.compile_blueprint(butch_bp)
    unreal.EditorAssetLibrary.save_asset(BUTCH_BP_OBJECT_PATH)
    log("Configured BP_RoomGenerator defaults to use bathhouse room subclasses.")


if __name__ == "__main__":
    main()
