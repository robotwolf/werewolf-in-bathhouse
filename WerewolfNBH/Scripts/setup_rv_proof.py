import unreal

PROOF_ROOT = "/Game/WerewolfBH/Proofs/RV"
ROOMS_PATH = f"{PROOF_ROOT}/Blueprints/Rooms"
DATA_ROOT = f"{PROOF_ROOT}/Data/Ginny"
ROOM_PROFILES_PATH = f"{DATA_ROOT}/Rooms"
LAYOUTS_PATH = f"{DATA_ROOT}/Layouts"
MASON_PROFILES_PATH = f"{PROOF_ROOT}/Data/Mason/Profiles"
MAPS_PATH = f"{PROOF_ROOT}/Maps"

STANDARD_OPENING_PATH = "/Game/WerewolfBH/Data/Ginny/Openings/DA_GinnyOpening_Standard"
GENERATOR_BP_PATH = "/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator"
GENERATOR_CLASS_PATH = "/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator.BP_RoomGenerator_C"

RV_LANE_BP_PATH = f"{ROOMS_PATH}/BP_Room_RV_Lane"
RV_SINGLE_BP_PATH = f"{ROOMS_PATH}/BP_Room_RV_SingleWide"
RV_LAYOUT_PATH = f"{LAYOUTS_PATH}/DA_GinnyLayout_RVProof"
MASON_MAP_PATH = f"{MAPS_PATH}/Mason_RV_Showcase"
GINNY_MAP_PATH = f"{MAPS_PATH}/Ginny_RV_Proof"

TRON_GLOW = "/Game/WerewolfBH/Materials/Tron_Glow"
HALL_FLOOR = "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Hall_Floor"
HALL_WALL = "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Hall_Wall"
CEILING = "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Ceiling"
SERVICE = "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_ServiceMetal"
WOOD = "/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_WoodBench"
PLANE_MESH = "/Engine/BasicShapes/Plane"


def log(message: str) -> None:
    unreal.log(f"[setup_rv_proof] {message}")


def ensure_folder(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def load_asset(path: str):
    asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def load_class(path: str):
    loaded = unreal.load_class(None, path)
    if not loaded:
        raise RuntimeError(f"Missing class: {path}")
    return loaded


def create_blueprint(asset_name: str, package_path: str):
    asset_path = f"{package_path}/{asset_name}"
    existing = unreal.load_asset(asset_path)
    if existing:
        return existing

    ensure_folder(package_path)
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", unreal.RoomModuleBase)
    blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, package_path, None, factory)
    if not blueprint:
        raise RuntimeError(f"Failed to create blueprint: {asset_path}")
    return blueprint


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

    created = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, package_path, asset_class, factory)
    if not created:
        raise RuntimeError(f"Failed to create data asset: {asset_path}")
    return created


def save_loaded(asset) -> None:
    unreal.EditorAssetLibrary.save_loaded_asset(asset)


def compile_and_save_blueprint(blueprint) -> None:
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)


def get_cdo(blueprint):
    generated = blueprint.generated_class()
    if not generated:
        raise RuntimeError(f"Blueprint has no generated class: {blueprint.get_name()}")
    return unreal.get_default_object(generated)


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
                return obj, handle
    return None, None


def add_connector(blueprint, name: str, location: unreal.Vector, rotation: unreal.Rotator, direction, passage_kind, boundary_kind, clearance_class, contract_tag: str):
    existing, _ = find_connector(blueprint, name)
    if existing:
        comp = existing
    else:
        subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
        handles = subsys.k2_gather_subobject_data_for_blueprint(blueprint)
        if not handles:
            raise RuntimeError(f"No subobject handles found for {blueprint.get_name()}")

        params = unreal.AddNewSubobjectParams()
        params.set_editor_property("parent_handle", handles[0])
        params.set_editor_property("new_class", unreal.PrototypeRoomConnectorComponent)
        params.set_editor_property("blueprint_context", blueprint)
        params.set_editor_property("conform_transform_to_parent", True)

        new_handle, fail_reason = subsys.add_new_subobject(params)
        if not unreal.SubobjectDataBlueprintFunctionLibrary.is_handle_valid(new_handle):
            raise RuntimeError(f"Failed to add connector {name} to {blueprint.get_name()}: {fail_reason}")

        subsys.rename_subobject(new_handle, name)
        data = subsys.k2_find_subobject_data_from_handle(new_handle)
        comp = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        if not comp:
            raise RuntimeError(f"Failed to resolve connector component {name}")

    comp.set_editor_property("RelativeLocation", location)
    comp.set_editor_property("RelativeRotation", rotation)
    comp.set_editor_property("SocketID", name)
    comp.set_editor_property("Direction", direction)
    comp.set_editor_property("PassageKind", passage_kind)
    comp.set_editor_property("BoundaryKind", boundary_kind)
    comp.set_editor_property("ClearanceClass", clearance_class)
    comp.set_editor_property("ContractTag", contract_tag)


def remove_unwanted_connectors(blueprint, allowed_names):
    subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsys.k2_gather_subobject_data_for_blueprint(blueprint)
    if not handles:
        return

    root_handle = handles[0]
    to_delete = []
    for handle in handles:
        data = subsys.k2_find_subobject_data_from_handle(handle)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        if not obj or not isinstance(obj, unreal.PrototypeRoomConnectorComponent):
            continue
        base_name = obj.get_name().replace("_GEN_VARIABLE", "")
        if base_name.startswith("Conn_") and base_name not in allowed_names:
            to_delete.append(handle)

    if to_delete:
        subsys.delete_subobjects(root_handle, to_delete, blueprint)


def set_room_bounds(cdo, full_size: unreal.Vector):
    room_bounds = cdo.get_editor_property("RoomBoundsBox")
    half_extents = unreal.Vector(full_size.x * 0.5, full_size.y * 0.5, full_size.z * 0.5)
    room_bounds.set_editor_property("BoxExtent", half_extents)
    room_bounds.set_editor_property("RelativeLocation", unreal.Vector(0.0, 0.0, half_extents.z))


def configure_stock_settings(cdo, *, full_size: unreal.Vector, technique, profile_id: str, wall_thickness=24.0, floor_thickness=14.0, ceiling_thickness=12.0, door_width=140.0, door_height=220.0):
    settings = cdo.get_editor_property("StockAssemblySettings")
    settings.set_editor_property("bEnabled", True)
    settings.set_editor_property("FootprintType", unreal.RoomStockFootprintType.RECTANGLE)
    settings.set_editor_property("bOverrideConstructionTechnique", True)
    settings.set_editor_property("ConstructionTechnique", technique)
    settings.set_editor_property("ConstructionProfileId", profile_id)
    settings.set_editor_property("FloorThickness", floor_thickness)
    settings.set_editor_property("WallThickness", wall_thickness)
    settings.set_editor_property("CeilingThickness", ceiling_thickness)
    settings.set_editor_property("DoorWidthMode", unreal.RoomStockDoorWidthMode.STANDARD)
    settings.set_editor_property("DoorWidth", door_width)
    settings.set_editor_property("DoorHeight", door_height)
    cdo.set_editor_property("StockAssemblySettings", settings)
    set_room_bounds(cdo, full_size)


def create_mason_profile(asset_name: str, technique, profile_id: str, floor_thickness: float, wall_thickness: float, ceiling_thickness: float, door_width: float, door_height: float):
    profile = create_data_asset(asset_name, MASON_PROFILES_PATH, unreal.MasonConstructionProfile)
    profile.set_editor_property("ConstructionTechnique", technique)
    profile.set_editor_property("ConstructionProfileId", profile_id)
    profile.set_editor_property("FloorThickness", floor_thickness)
    profile.set_editor_property("WallThickness", wall_thickness)
    profile.set_editor_property("CeilingThickness", ceiling_thickness)
    profile.set_editor_property("DefaultDoorWidth", door_width)
    profile.set_editor_property("DefaultDoorHeight", door_height)
    save_loaded(profile)
    return profile


def set_placement_rules(cdo, role, allow_main, allow_branch, can_terminate, min_depth=0, max_depth=-1, max_instances=-1):
    rules = cdo.get_editor_property("PlacementRules")
    rules.set_editor_property("PlacementRole", role)
    rules.set_editor_property("bAllowOnMainPath", allow_main)
    rules.set_editor_property("bAllowOnBranch", allow_branch)
    rules.set_editor_property("bCanTerminatePath", can_terminate)
    rules.set_editor_property("MinDepthFromStart", min_depth)
    rules.set_editor_property("MaxDepthFromStart", max_depth)
    rules.set_editor_property("MaxInstances", max_instances)
    cdo.set_editor_property("PlacementRules", rules)


def configure_lane_blueprint():
    blueprint = create_blueprint("BP_Room_RV_Lane", ROOMS_PATH)
    cdo = get_cdo(blueprint)
    cdo.set_editor_property("RoomID", "RVParkLane")
    cdo.set_editor_property("RoomType", "RVParkLane")
    cdo.set_editor_property("Weight", 1.4)
    cdo.set_editor_property("MinConnections", 1)
    cdo.set_editor_property("MaxConnections", 3)
    cdo.set_editor_property("bRequired", True)
    cdo.set_editor_property("bExpandGeneration", True)
    cdo.set_editor_property("AllowedNeighborRoomTypes", ["RVParkLane", "RVSingleWide"])
    cdo.set_editor_property("TransitionType", unreal.RoomTransitionType.NONE)
    set_placement_rules(cdo, unreal.RoomPlacementRole.MAIN_PATH, True, True, True, 0, -1, -1)
    configure_stock_settings(
        cdo,
        full_size=unreal.Vector(900.0, 700.0, 220.0),
        technique=unreal.MasonConstructionTechnique.OPEN_LOT,
        profile_id="RVLotLane",
        wall_thickness=20.0,
        floor_thickness=12.0,
        ceiling_thickness=0.0,
        door_width=220.0,
        door_height=220.0,
    )
    cdo.set_editor_property("RoomProfile", None)
    cdo.set_editor_property("FloorMaterialOverride", None)
    cdo.set_editor_property("WallMaterialOverride", None)
    cdo.set_editor_property("CeilingMaterialOverride", None)
    cdo.set_editor_property("RoofMaterialOverride", None)

    half_x = 450.0
    half_y = 350.0
    passage = unreal.RoomConnectorPassageKind.FOOTPATH
    boundary = unreal.RoomConnectorBoundaryKind.EXTERIOR
    clearance = unreal.RoomConnectorClearanceClass.HUMAN_WIDE
    contract_tag = "RVParkFootpath"
    add_connector(blueprint, "Conn_N", unreal.Vector(0.0, half_y, 110.0), unreal.Rotator(0.0, 90.0, 0.0), unreal.RoomConnectorDirection.NORTH, passage, boundary, clearance, contract_tag)
    add_connector(blueprint, "Conn_E", unreal.Vector(half_x, 0.0, 110.0), unreal.Rotator(0.0, 0.0, 0.0), unreal.RoomConnectorDirection.EAST, passage, boundary, clearance, contract_tag)
    add_connector(blueprint, "Conn_W", unreal.Vector(-half_x, 0.0, 110.0), unreal.Rotator(0.0, 180.0, 0.0), unreal.RoomConnectorDirection.WEST, passage, boundary, clearance, contract_tag)
    remove_unwanted_connectors(blueprint, {"Conn_N", "Conn_E", "Conn_W"})
    compile_and_save_blueprint(blueprint)
    return blueprint


def configure_rv_blueprint():
    blueprint = create_blueprint("BP_Room_RV_SingleWide", ROOMS_PATH)
    cdo = get_cdo(blueprint)
    cdo.set_editor_property("RoomID", "RVSingleWide")
    cdo.set_editor_property("RoomType", "RVSingleWide")
    cdo.set_editor_property("Weight", 0.7)
    cdo.set_editor_property("MinConnections", 1)
    cdo.set_editor_property("MaxConnections", 1)
    cdo.set_editor_property("bRequired", False)
    cdo.set_editor_property("bExpandGeneration", True)
    cdo.set_editor_property("AllowedNeighborRoomTypes", ["RVParkLane"])
    cdo.set_editor_property("TransitionType", unreal.RoomTransitionType.NONE)
    set_placement_rules(cdo, unreal.RoomPlacementRole.BRANCH, False, True, True, 1, -1, 3)
    configure_stock_settings(
        cdo,
        full_size=unreal.Vector(1200.0, 360.0, 260.0),
        technique=unreal.MasonConstructionTechnique.OBJECT_SHELL,
        profile_id="RVSingleWide",
        wall_thickness=22.0,
        floor_thickness=16.0,
        ceiling_thickness=12.0,
        door_width=110.0,
        door_height=220.0,
    )
    cdo.set_editor_property("RoomProfile", None)
    cdo.set_editor_property("FloorMaterialOverride", None)
    cdo.set_editor_property("WallMaterialOverride", None)
    cdo.set_editor_property("CeilingMaterialOverride", None)
    cdo.set_editor_property("RoofMaterialOverride", None)

    half_y = 180.0
    add_connector(
        blueprint,
        "Conn_S",
        unreal.Vector(0.0, -half_y, 110.0),
        unreal.Rotator(0.0, -90.0, 0.0),
        unreal.RoomConnectorDirection.SOUTH,
        unreal.RoomConnectorPassageKind.EXTERIOR_DOOR,
        unreal.RoomConnectorBoundaryKind.EXTERIOR,
        unreal.RoomConnectorClearanceClass.HUMAN_STANDARD,
        "RVParkFootpath",
    )
    remove_unwanted_connectors(blueprint, {"Conn_S"})
    compile_and_save_blueprint(blueprint)
    return blueprint


def create_room_profile(asset_name: str, room_id: str, room_type: str, weight: float, min_conn: int, max_conn: int, allowed_neighbors, placement_role, allow_main: bool, allow_branch: bool, can_terminate: bool, max_instances: int, stock_settings, construction_profile, floor_material, wall_material, ceiling_material, roof_material):
    profile = create_data_asset(asset_name, ROOM_PROFILES_PATH, unreal.GinnyRoomProfile)
    profile.set_editor_property("RoomID", room_id)
    profile.set_editor_property("RoomType", room_type)
    profile.set_editor_property("Weight", weight)
    profile.set_editor_property("MinConnections", min_conn)
    profile.set_editor_property("MaxConnections", max_conn)
    profile.set_editor_property("bRequired", False)
    rules = profile.get_editor_property("PlacementRules")
    rules.set_editor_property("PlacementRole", placement_role)
    rules.set_editor_property("bAllowOnMainPath", allow_main)
    rules.set_editor_property("bAllowOnBranch", allow_branch)
    rules.set_editor_property("bCanTerminatePath", can_terminate)
    rules.set_editor_property("MinDepthFromStart", 0 if allow_main else 1)
    rules.set_editor_property("MaxDepthFromStart", -1)
    rules.set_editor_property("MaxInstances", max_instances)
    profile.set_editor_property("PlacementRules", rules)
    profile.set_editor_property("AllowedNeighborRoomTypes", list(allowed_neighbors))
    profile.set_editor_property("TransitionType", unreal.RoomTransitionType.NONE)
    profile.set_editor_property("TransitionTargetConfigId", "")
    profile.set_editor_property("StockAssemblySettings", stock_settings)
    profile.set_editor_property("ConstructionProfile", construction_profile)
    profile.set_editor_property("LegacyRoomMaterial", None)
    profile.set_editor_property("FloorMaterial", floor_material)
    profile.set_editor_property("WallMaterial", wall_material)
    profile.set_editor_property("CeilingMaterial", ceiling_material)
    profile.set_editor_property("RoofMaterial", roof_material)
    profile.set_editor_property("DefaultOpeningProfile", load_asset(STANDARD_OPENING_PATH))
    save_loaded(profile)
    return profile


def configure_profiles(lane_blueprint, rv_blueprint):
    lane_cdo = get_cdo(lane_blueprint)
    rv_cdo = get_cdo(rv_blueprint)
    lane_mason = create_mason_profile(
        "DA_Mason_RV_Lane",
        unreal.MasonConstructionTechnique.OPEN_LOT,
        "RVLotLane",
        12.0,
        20.0,
        0.0,
        220.0,
        220.0,
    )
    rv_mason = create_mason_profile(
        "DA_Mason_RV_SingleWide",
        unreal.MasonConstructionTechnique.OBJECT_SHELL,
        "RVSingleWide",
        16.0,
        22.0,
        12.0,
        110.0,
        220.0,
    )

    lane_profile = create_room_profile(
        "DA_GinnyRoom_RVParkLane",
        "RVParkLane",
        "RVParkLane",
        1.4,
        1,
        3,
        ["RVParkLane", "RVSingleWide"],
        unreal.RoomPlacementRole.MAIN_PATH,
        True,
        True,
        True,
        -1,
        lane_cdo.get_editor_property("StockAssemblySettings"),
        lane_mason,
        load_asset(HALL_FLOOR),
        load_asset(SERVICE),
        None,
        load_asset(TRON_GLOW),
    )

    rv_profile = create_room_profile(
        "DA_GinnyRoom_RVSingleWide",
        "RVSingleWide",
        "RVSingleWide",
        0.7,
        1,
        1,
        ["RVParkLane"],
        unreal.RoomPlacementRole.BRANCH,
        False,
        True,
        True,
        3,
        rv_cdo.get_editor_property("StockAssemblySettings"),
        rv_mason,
        load_asset(WOOD),
        load_asset(HALL_WALL),
        load_asset(CEILING),
        load_asset(TRON_GLOW),
    )

    lane_cdo.set_editor_property("RoomProfile", lane_profile)
    rv_cdo.set_editor_property("RoomProfile", rv_profile)
    compile_and_save_blueprint(lane_blueprint)
    compile_and_save_blueprint(rv_blueprint)
    return lane_profile, rv_profile


def configure_layout_profile(lane_blueprint, rv_blueprint):
    layout = create_data_asset("DA_GinnyLayout_RVProof", LAYOUTS_PATH, unreal.GinnyLayoutProfile)
    lane_class = lane_blueprint.generated_class()
    rv_class = rv_blueprint.generated_class()

    layout.set_editor_property("StartRoomClass", lane_class)
    layout.set_editor_property("DeadEndRoomClass", None)
    layout.set_editor_property("AvailableRooms", [lane_class, rv_class])
    layout.set_editor_property("ConnectorFallbackRooms", [lane_class])
    layout.set_editor_property("RequiredMainPathRooms", [lane_class, lane_class])
    layout.set_editor_property("RequiredBranchRooms", [rv_class])
    layout.set_editor_property("MaxRooms", 5)
    layout.set_editor_property("AttemptsPerDoor", 5)
    layout.set_editor_property("VerticalSnapSize", 10.0)
    layout.set_editor_property("bAllowVerticalTransitions", False)
    layout.set_editor_property("MaxVerticalDisplacement", 0.0)
    layout.set_editor_property("MaxLayoutAttempts", 8)
    layout.set_editor_property("bEnableHallwayChains", True)
    layout.set_editor_property("MaxHallwayChainSegments", 2)
    layout.set_editor_property("bRunButchAfterGeneration", False)
    layout.set_editor_property("bSpawnButchIfMissing", False)

    if hasattr(unreal, "RoomClassEntry"):
        entries = unreal.Array(unreal.RoomClassEntry)
        lane_entry = unreal.RoomClassEntry()
        lane_entry.set_editor_property("RoomClass", lane_class)
        lane_entry.set_editor_property("Weight", 1.6)
        lane_entry.set_editor_property("MinRoomsBetweenUses", 0)
        rv_entry = unreal.RoomClassEntry()
        rv_entry.set_editor_property("RoomClass", rv_class)
        rv_entry.set_editor_property("Weight", 0.8)
        rv_entry.set_editor_property("MinRoomsBetweenUses", 1)
        entries.append(lane_entry)
        entries.append(rv_entry)
        layout.set_editor_property("RoomClassPool", entries)

    save_loaded(layout)
    return layout


def clear_world(world):
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = actor_subsystem.get_all_level_actors()
    for actor in actors:
        try:
            actor_subsystem.destroy_actor(actor)
        except Exception:
            continue


def spawn_basic_environment():
    directional = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 500.0), unreal.Rotator(-45.0, 30.0, 0.0))
    directional.set_actor_label("DirectionalLight")
    skylight = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
    skylight.set_actor_label("SkyLight")
    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
    sky.set_actor_label("SkyAtmosphere")
    fog = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
    fog.set_actor_label("ExponentialHeightFog")

    floor_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
    floor_actor.set_actor_label("ProofFloor")
    floor_component = floor_actor.get_component_by_class(unreal.StaticMeshComponent)
    floor_component.set_static_mesh(load_asset(PLANE_MESH))
    floor_component.set_editor_property("RelativeScale3D", unreal.Vector(80.0, 80.0, 1.0))

    nav = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(0.0, 0.0, 200.0), unreal.Rotator(0.0, 0.0, 0.0))
    nav.set_actor_label("NavMeshBoundsVolume")
    nav_component = nav.get_editor_property("brush_component")
    if nav_component:
        nav_component.set_editor_property("RelativeScale3D", unreal.Vector(16.0, 16.0, 4.0))

    player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-900.0, 0.0, 120.0), unreal.Rotator(0.0, 0.0, 0.0))
    player_start.set_actor_label("PlayerStart")


def ensure_map(map_path: str):
    ensure_folder(MAPS_PATH)
    if not unreal.EditorAssetLibrary.does_asset_exist(map_path):
        level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        if not level_subsystem.new_level(map_path):
            raise RuntimeError(f"Failed to create map: {map_path}")
    world = unreal.EditorLoadingAndSavingUtils.load_map(map_path)
    if not world:
        raise RuntimeError(f"Failed to load map: {map_path}")
    clear_world(world)
    spawn_basic_environment()
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    return world


def build_mason_showcase_map(lane_blueprint, rv_blueprint):
    ensure_map(MASON_MAP_PATH)
    lane_class = lane_blueprint.generated_class()
    rv_class = rv_blueprint.generated_class()

    lane_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(lane_class, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
    lane_actor.set_actor_label("MasonLane")

    rv_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(rv_class, unreal.Vector(0.0, 1100.0, 0.0), unreal.Rotator(0.0, -90.0, 0.0))
    rv_actor.set_actor_label("MasonRV")

    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)


def build_ginny_proof_map(layout_profile):
    ensure_map(GINNY_MAP_PATH)
    generator_class = load_class(GENERATOR_CLASS_PATH)
    generator = unreal.EditorLevelLibrary.spawn_actor_from_class(generator_class, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
    generator.set_actor_label("Ginny_RVGenerator")
    generator.set_editor_property("LayoutProfile", layout_profile)
    generator.set_editor_property("bGenerateOnBeginPlay", False)
    generator.set_editor_property("bUseNewSeedOnGenerate", False)
    generator.set_editor_property("RunSeed", 2112)
    generator.set_editor_property("bDebugDrawBounds", False)
    generator.set_editor_property("bDebugDrawDoors", False)
    generator.set_editor_property("bPrintDebugMessages", True)
    generator.clear_generated_layout()
    generator.generate_layout()
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)


def main():
    for folder in (PROOF_ROOT, ROOMS_PATH, DATA_ROOT, ROOM_PROFILES_PATH, LAYOUTS_PATH, MASON_PROFILES_PATH, MAPS_PATH):
        ensure_folder(folder)

    lane_blueprint = configure_lane_blueprint()
    rv_blueprint = configure_rv_blueprint()
    configure_profiles(lane_blueprint, rv_blueprint)
    layout_profile = configure_layout_profile(lane_blueprint, rv_blueprint)
    build_mason_showcase_map(lane_blueprint, rv_blueprint)
    build_ginny_proof_map(layout_profile)
    log("Created RV proof blueprints, profiles, layout, and showcase maps.")


if __name__ == "__main__":
    main()
