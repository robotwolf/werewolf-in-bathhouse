import unreal


DATA_ROOT = "/Game/WerewolfBH/Data/Ginny"
OPENINGS_PATH = f"{DATA_ROOT}/Openings"
ROOM_PROFILES_PATH = f"{DATA_ROOT}/Rooms"
LAYOUTS_PATH = f"{DATA_ROOT}/Layouts"
MASON_PROFILES_PATH = "/Game/WerewolfBH/Data/Mason/Profiles"
FLOWS_PATH = "/Game/WerewolfBH/Data/Flo/Flows"

GENERATOR_BP_PATH = "/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator"

ROOM_BLUEPRINT_PATHS = {
    "EntryReception": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_EntryReception",
    "LockerHall": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_LockerHall",
    "PublicHallStraight": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Straight",
    "PublicHallCorner": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Corner",
    "PublicHallStair": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Stair_Up",
    "WashShower": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_WashShower",
    "PoolHall": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PoolHall",
    "Sauna": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Sauna",
    "BoilerService": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_BoilerService",
    "ColdPlunge": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_ColdPlunge",
    "SteamRoom": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_SteamRoom",
    "Toilet": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Toilet",
    "Storage": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Storage",
}

STANDARD_OPENING_ASSET = "DA_GinnyOpening_Standard"
DOUBLE_WIDE_OPENING_ASSET = "DA_GinnyOpening_DoubleWide"
BATHHOUSE_LAYOUT_ASSET = "DA_GinnyLayout_Bathhouse_Default"
BATHHOUSE_FLOW_ASSET = "DA_FloFlow_Bathhouse_Prototype"
STAIR_TRANSITION_TARGET = "SecondFloor_PrivateCubicles"
BATHHOUSE_ROOT_CONFIG = "Bathhouse_PublicCore"

MATERIAL_PATHS = {
    "tron_glow": "/Game/WerewolfBH/Materials/Tron_Glow",
    "entry_floor": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Entry_Floor",
    "entry_wall": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Entry_Wall",
    "locker_floor": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Locker_Floor",
    "locker_wall": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Locker_Wall",
    "hall_floor": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Hall_Floor",
    "hall_wall": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Hall_Wall",
    "stair_floor": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Stair_Floor",
    "stair_wall": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Stair_Wall",
    "stair_ceiling": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Stair_Ceiling",
    "ceiling": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Ceiling",
    "water": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Water",
    "service_metal": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_ServiceMetal",
    "porcelain": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Porcelain",
    "wood_bench": "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_WoodBench",
}

MARKER_REQUIREMENTS = {
    "EntryReception": [
        ("NPC", 2, -1, "Entry should support waiting and gossip beats."),
        ("Clue", 1, -1, "Entry should support at least one clue pocket."),
        ("MissionSocket", 1, -1, "Entry should support at least one mission pocket."),
    ],
    "PublicHallStraight": [
        ("NPC", 1, -1, "Hallway should support a simple idle or observe point."),
        ("Clue", 1, -1, "Hallway should support at least one clue pocket."),
        ("MissionSocket", 1, -1, "Hallway should support one dynamic event pocket."),
    ],
    "LockerHall": [
        ("NPC", 3, -1, "Locker hall is a social and changing hub."),
        ("Task", 1, -1, "Locker hall should support at least one maintenance task."),
        ("Clue", 1, -1, "Locker hall should support at least one clue surface."),
        ("MissionSocket", 1, -1, "Locker hall should support one dynamic scene pocket."),
    ],
    "PoolHall": [
        ("NPC", 2, -1, "Pool hall should support at least two hangout points."),
        ("Clue", 1, -1, "Pool hall should support a clue pocket."),
        ("MissionSocket", 1, -1, "Pool hall should support one mission socket."),
        ("FX", 1, -1, "Pool hall should support at least one FX anchor."),
    ],
    "Sauna": [
        ("NPC", 1, -1, "Sauna should support at least one rest point."),
        ("Clue", 1, -1, "Sauna should support a clue surface."),
        ("MissionSocket", 1, -1, "Sauna should support one mission socket."),
        ("FX", 1, -1, "Sauna should support an FX anchor."),
    ],
    "BoilerService": [
        ("NPC", 1, -1, "Boiler room should support one worker or lurking point."),
        ("Task", 1, -1, "Boiler room should support one maintenance task."),
        ("Clue", 1, -1, "Boiler room should support a clue surface."),
        ("MissionSocket", 1, -1, "Boiler room should support one mission pocket."),
        ("FX", 1, -1, "Boiler room should support an FX anchor."),
    ],
}


def log(message: str) -> None:
    unreal.log(f"[sync_ginny_profiles] {message}")


def ensure_folder(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def load_asset(path: str):
    asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def create_data_asset(asset_name: str, package_path: str, asset_class):
    asset_path = f"{package_path}/{asset_name}"
    existing = unreal.load_asset(asset_path)
    if existing:
        return existing

    ensure_folder(package_path)
    factory = unreal.DataAssetFactory()
    configured = False
    for property_name in ("DataAssetClass", "data_asset_class"):
        try:
            factory.set_editor_property(property_name, asset_class)
            configured = True
            break
        except Exception:
            continue

    if not configured:
        raise RuntimeError(f"Could not configure DataAssetFactory for {asset_class}")

    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, package_path, asset_class, factory)
    if not asset:
        raise RuntimeError(f"Failed to create data asset: {asset_path}")
    return asset


def save_asset(asset) -> None:
    unreal.EditorAssetLibrary.save_loaded_asset(asset)


def get_material(key: str):
    return load_asset(MATERIAL_PATHS[key])


def get_room_material_set(profile_name: str):
    ceiling = get_material("ceiling")
    roof = get_material("tron_glow")

    if profile_name == "EntryReception":
        return (None, get_material("entry_floor"), get_material("entry_wall"), ceiling, roof)
    if profile_name == "LockerHall":
        return (None, get_material("locker_floor"), get_material("locker_wall"), ceiling, roof)
    if profile_name == "PublicHallStair":
        return (None, get_material("stair_floor"), get_material("stair_wall"), get_material("stair_ceiling"), roof)
    if profile_name in ("PoolHall", "ColdPlunge"):
        return (None, get_material("hall_floor"), get_material("hall_wall"), ceiling, roof)
    if profile_name in ("WashShower", "Toilet", "SteamRoom"):
        return (None, get_material("porcelain"), get_material("hall_wall"), ceiling, roof)
    if profile_name in ("BoilerService", "Storage"):
        return (None, get_material("service_metal"), get_material("hall_wall"), ceiling, roof)
    if profile_name == "Sauna":
        return (None, get_material("wood_bench"), get_material("hall_wall"), ceiling, roof)

    return (None, get_material("hall_floor"), get_material("hall_wall"), ceiling, roof)


def get_cdo(blueprint):
    generated = blueprint.generated_class()
    if not generated:
        raise RuntimeError(f"Blueprint has no generated class: {blueprint.get_name()}")
    return unreal.get_default_object(generated)


def load_blueprint(path: str):
    blueprint = unreal.load_asset(path)
    if not blueprint:
        raise RuntimeError(f"Missing blueprint: {path}")
    return blueprint


def compile_and_save_blueprint(blueprint) -> None:
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)


def find_connector(blueprint, name: str):
    subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsys.k2_gather_subobject_data_for_blueprint(blueprint)
    for handle in handles:
        data = subsys.k2_find_subobject_data_from_handle(handle)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        if not obj:
            continue
        if isinstance(obj, unreal.PrototypeRoomConnectorComponent):
            obj_name = obj.get_name().replace("_GEN_VARIABLE", "")
            if obj_name == name:
                return obj
    return None


def configure_opening_profiles():
    standard = create_data_asset(STANDARD_OPENING_ASSET, OPENINGS_PATH, unreal.GinnyOpeningProfile)
    standard.set_editor_property("OpeningWidthMode", unreal.RoomStockDoorWidthMode.STANDARD)
    standard.set_editor_property("CustomOpeningWidth", 240.0)
    standard.set_editor_property("OpeningHeight", 260.0)
    standard.set_editor_property("bGenerateFramePieces", True)
    standard.set_editor_property("FrameThickness", 12.0)
    standard.set_editor_property("FrameDepth", 18.0)
    standard.set_editor_property("bGenerateThresholdPiece", False)
    standard.set_editor_property("ThresholdHeight", 8.0)
    save_asset(standard)

    double_wide = create_data_asset(DOUBLE_WIDE_OPENING_ASSET, OPENINGS_PATH, unreal.GinnyOpeningProfile)
    double_wide.set_editor_property("OpeningWidthMode", unreal.RoomStockDoorWidthMode.DOUBLE_WIDE)
    double_wide.set_editor_property("CustomOpeningWidth", 400.0)
    double_wide.set_editor_property("OpeningHeight", 300.0)
    double_wide.set_editor_property("bGenerateFramePieces", True)
    double_wide.set_editor_property("FrameThickness", 16.0)
    double_wide.set_editor_property("FrameDepth", 24.0)
    double_wide.set_editor_property("bGenerateThresholdPiece", False)
    double_wide.set_editor_property("ThresholdHeight", 8.0)
    save_asset(double_wide)

    return standard, double_wide


def configure_mason_profiles():
    ensure_folder(MASON_PROFILES_PATH)

    box = create_data_asset("DA_Mason_Bathhouse_BoxShell", MASON_PROFILES_PATH, unreal.MasonConstructionProfile)
    box.set_editor_property("ConstructionTechnique", unreal.MasonConstructionTechnique.BOX_SHELL)
    box.set_editor_property("ConstructionProfileId", "BathhouseBoxShell")
    box.set_editor_property("FloorThickness", 20.0)
    box.set_editor_property("WallThickness", 30.0)
    box.set_editor_property("CeilingThickness", 20.0)
    box.set_editor_property("DefaultDoorWidth", 200.0)
    box.set_editor_property("DefaultDoorHeight", 260.0)
    save_asset(box)

    corner = create_data_asset("DA_Mason_Bathhouse_CornerShell", MASON_PROFILES_PATH, unreal.MasonConstructionProfile)
    corner.set_editor_property("ConstructionTechnique", unreal.MasonConstructionTechnique.SLICE_FOOTPRINT)
    corner.set_editor_property("ConstructionProfileId", "BathhouseCorner")
    corner.set_editor_property("FloorThickness", 20.0)
    corner.set_editor_property("WallThickness", 30.0)
    corner.set_editor_property("CeilingThickness", 20.0)
    corner.set_editor_property("DefaultDoorWidth", 200.0)
    corner.set_editor_property("DefaultDoorHeight", 260.0)
    save_asset(corner)

    stair = create_data_asset("DA_Mason_Bathhouse_PublicStair", MASON_PROFILES_PATH, unreal.MasonConstructionProfile)
    stair.set_editor_property("ConstructionTechnique", unreal.MasonConstructionTechnique.PUBLIC_STAIR_SHELL)
    stair.set_editor_property("ConstructionProfileId", "BathhousePublicStair")
    stair.set_editor_property("FloorThickness", 20.0)
    stair.set_editor_property("WallThickness", 30.0)
    stair.set_editor_property("CeilingThickness", 20.0)
    stair.set_editor_property("DefaultDoorWidth", 240.0)
    stair.set_editor_property("DefaultDoorHeight", 300.0)
    stair.set_editor_property("StairWalkWidth", 880.0)
    stair.set_editor_property("StairLowerLandingDepth", 360.0)
    stair.set_editor_property("StairUpperLandingDepth", 360.0)
    stair.set_editor_property("StairStepCount", 14)
    stair.set_editor_property("StairRiseHeight", 420.0)
    stair.set_editor_property("StairSideInset", 0.0)
    stair.set_editor_property("bCreateStairLandingSideOpenings", True)
    stair.set_editor_property("StairLandingSideOpeningWidth", 320.0)
    stair.set_editor_property("StairLandingSideOpeningHeight", 260.0)
    save_asset(stair)

    return {
        "box": box,
        "corner": corner,
        "stair": stair,
    }


def get_mason_profile_for_room(profile_name: str, mason_profiles):
    if profile_name == "PublicHallCorner":
        return mason_profiles["corner"]
    if profile_name == "PublicHallStair":
        return mason_profiles["stair"]
    return mason_profiles["box"]


def build_marker_requirements(profile_name: str):
    family_map = {
        "NPC": unreal.RoomGameplayMarkerFamily.NPC,
        "Task": unreal.RoomGameplayMarkerFamily.TASK,
        "Clue": unreal.RoomGameplayMarkerFamily.CLUE,
        "MissionSocket": unreal.RoomGameplayMarkerFamily.MISSION_SOCKET,
        "FX": unreal.RoomGameplayMarkerFamily.FX,
    }

    requirements = unreal.Array(unreal.RoomGameplayMarkerRequirement)
    for family_name, min_count, max_count, notes in MARKER_REQUIREMENTS.get(profile_name, []):
        requirement = unreal.RoomGameplayMarkerRequirement()
        requirement.set_editor_property("MarkerFamily", family_map[family_name])
        requirement.set_editor_property("MinCount", min_count)
        requirement.set_editor_property("MaxCount", max_count)
        requirement.set_editor_property("Notes", notes)
        requirements.append(requirement)

    return requirements


def sync_room_profiles(default_opening_profile, stair_opening_profile, mason_profiles):
    room_blueprints = {}

    for profile_name, blueprint_path in ROOM_BLUEPRINT_PATHS.items():
        blueprint = load_blueprint(blueprint_path)
        room_blueprints[profile_name] = blueprint
        cdo = get_cdo(blueprint)

        profile = create_data_asset(f"DA_GinnyRoom_{profile_name}", ROOM_PROFILES_PATH, unreal.GinnyRoomProfile)
        profile.set_editor_property("RoomID", cdo.get_editor_property("RoomID"))
        profile.set_editor_property("RoomType", cdo.get_editor_property("RoomType"))
        profile.set_editor_property("Weight", cdo.get_editor_property("Weight"))
        profile.set_editor_property("MinConnections", cdo.get_editor_property("MinConnections"))
        profile.set_editor_property("MaxConnections", cdo.get_editor_property("MaxConnections"))
        profile.set_editor_property("bRequired", cdo.get_editor_property("bRequired"))
        profile.set_editor_property("PlacementRules", cdo.get_editor_property("PlacementRules"))
        profile.set_editor_property("AllowedNeighborRoomTypes", list(cdo.get_editor_property("AllowedNeighborRoomTypes")))
        profile.set_editor_property("TransitionType", cdo.get_editor_property("TransitionType"))
        profile.set_editor_property("TransitionTargetConfigId", cdo.get_editor_property("TransitionTargetConfigId"))
        profile.set_editor_property("GameplayMarkerRequirements", build_marker_requirements(profile_name))

        stock_settings = cdo.get_editor_property("StockAssemblySettings")
        stock_settings.set_editor_property("DoorWidthMode", unreal.RoomStockDoorWidthMode.STANDARD)
        profile.set_editor_property("StockAssemblySettings", stock_settings)
        profile.set_editor_property("ConstructionProfile", get_mason_profile_for_room(profile_name, mason_profiles))
        legacy_material, floor_material, wall_material, ceiling_material, roof_material = get_room_material_set(profile_name)
        profile.set_editor_property("LegacyRoomMaterial", legacy_material)
        profile.set_editor_property("FloorMaterial", floor_material)
        profile.set_editor_property("WallMaterial", wall_material)
        profile.set_editor_property("CeilingMaterial", ceiling_material)
        profile.set_editor_property("RoofMaterial", roof_material)
        profile.set_editor_property("DefaultOpeningProfile", default_opening_profile)
        save_asset(profile)

        cdo.set_editor_property("RoomProfile", profile)
        compile_and_save_blueprint(blueprint)

    stair_blueprint = room_blueprints["PublicHallStair"]
    stair_cdo = get_cdo(stair_blueprint)
    stair_profile = stair_cdo.get_editor_property("RoomProfile")
    stair_profile.set_editor_property("TransitionType", unreal.RoomTransitionType.CONFIG_HANDOFF)
    stair_profile.set_editor_property("TransitionTargetConfigId", STAIR_TRANSITION_TARGET)
    stair_profile.set_editor_property("DefaultOpeningProfile", default_opening_profile)
    save_asset(stair_profile)

    for connector_name in ("Conn_S", "Conn_N"):
        connector = find_connector(stair_blueprint, connector_name)
        if connector:
            connector.set_editor_property("OpeningProfileOverride", stair_opening_profile)

    compile_and_save_blueprint(stair_blueprint)
    return room_blueprints


def sync_layout_profile():
    generator_bp = load_blueprint(GENERATOR_BP_PATH)
    generator_cdo = get_cdo(generator_bp)

    layout = create_data_asset(BATHHOUSE_LAYOUT_ASSET, LAYOUTS_PATH, unreal.GinnyLayoutProfile)
    layout.set_editor_property("LayoutConfigId", BATHHOUSE_ROOT_CONFIG)
    for property_name in (
        "StartRoomClass",
        "DeadEndRoomClass",
        "AvailableRooms",
        "RoomClassPool",
        "ConnectorFallbackRooms",
        "RequiredMainPathRooms",
        "RequiredBranchRooms",
        "MaxRooms",
        "AttemptsPerDoor",
        "VerticalSnapSize",
        "bAllowVerticalTransitions",
        "MaxVerticalDisplacement",
        "MaxLayoutAttempts",
        "bEnableHallwayChains",
        "MaxHallwayChainSegments",
        "bRunButchAfterGeneration",
        "bSpawnButchIfMissing",
    ):
        layout.set_editor_property(property_name, generator_cdo.get_editor_property(property_name))
    save_asset(layout)

    generator_cdo.set_editor_property("LayoutProfile", layout)
    compile_and_save_blueprint(generator_bp)
    return layout


def sync_flo_flow_profile(layout_profile):
    ensure_folder(FLOWS_PATH)
    flow = create_data_asset(BATHHOUSE_FLOW_ASSET, FLOWS_PATH, unreal.FloFlowProfile)
    flow.set_editor_property("RootConfigId", BATHHOUSE_ROOT_CONFIG)

    config_nodes = unreal.Array(unreal.FloConfigNode)
    public_core = unreal.FloConfigNode()
    public_core.set_editor_property("ConfigId", BATHHOUSE_ROOT_CONFIG)
    public_core.set_editor_property("LayoutProfile", layout_profile)
    public_core.set_editor_property("bRequired", True)
    public_core.set_editor_property("bImplemented", True)
    public_core.set_editor_property("Notes", "Current healthy 2D bathhouse baseline generated by Ginny.")
    config_nodes.append(public_core)

    cubicles = unreal.FloConfigNode()
    cubicles.set_editor_property("ConfigId", STAIR_TRANSITION_TARGET)
    cubicles.set_editor_property("LayoutProfile", None)
    cubicles.set_editor_property("bRequired", False)
    cubicles.set_editor_property("bImplemented", False)
    cubicles.set_editor_property("Notes", "Reserved future upstairs private-cubicle regime.")
    config_nodes.append(cubicles)
    flow.set_editor_property("ConfigNodes", config_nodes)

    transition_rules = unreal.Array(unreal.FloTransitionRule)
    up_rule = unreal.FloTransitionRule()
    up_rule.set_editor_property("FromConfigId", BATHHOUSE_ROOT_CONFIG)
    up_rule.set_editor_property("ToConfigId", STAIR_TRANSITION_TARGET)
    up_rule.set_editor_property("TransitionType", unreal.RoomTransitionType.CONFIG_HANDOFF)
    up_rule.set_editor_property("TransitionTargetConfigId", STAIR_TRANSITION_TARGET)
    up_rule.set_editor_property("bRequired", True)
    up_rule.set_editor_property("bAllowReturn", True)
    up_rule.set_editor_property("bVisibleAsNormalArchitecture", True)
    up_rule.set_editor_property("Notes", "Current stair handoff seam. Destination config intentionally not implemented yet.")
    transition_rules.append(up_rule)
    flow.set_editor_property("TransitionRules", transition_rules)
    save_asset(flow)


def main():
    ensure_folder(DATA_ROOT)
    ensure_folder(OPENINGS_PATH)
    ensure_folder(ROOM_PROFILES_PATH)
    ensure_folder(LAYOUTS_PATH)
    ensure_folder(MASON_PROFILES_PATH)
    ensure_folder(FLOWS_PATH)

    standard_opening, double_wide_opening = configure_opening_profiles()
    mason_profiles = configure_mason_profiles()
    sync_room_profiles(standard_opening, double_wide_opening, mason_profiles)
    layout_profile = sync_layout_profile()
    sync_flo_flow_profile(layout_profile)
    log("Synchronized Ginny room, layout, opening, Mason construction, and Flo flow profiles.")


if __name__ == "__main__":
    main()
