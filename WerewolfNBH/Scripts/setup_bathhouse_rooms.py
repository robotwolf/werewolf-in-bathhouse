import unreal


ROOMS_PATH = "/Game/WerewolfBH/Blueprints/Rooms"
MATERIALS_PATH = "/Game/WerewolfBH/Materials/Assembler"
WALL_THICKNESS = 30.0
FLOOR_THICKNESS = 20.0
CEILING_THICKNESS = 20.0
DOOR_WIDTH = 200.0
DOOR_HEIGHT = 260.0

FLOOR_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Floor"
WALL_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Wall"
CEILING_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Ceiling"
ENTRY_FLOOR_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Entry_Floor"
ENTRY_WALL_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Entry_Wall"
LOCKER_FLOOR_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Locker_Floor"
LOCKER_WALL_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Locker_Wall"
HALL_FLOOR_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Hall_Floor"
HALL_WALL_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Hall_Wall"
STAIR_FLOOR_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Stair_Floor"
STAIR_WALL_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Stair_Wall"
STAIR_CEILING_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Stair_Ceiling"
WATER_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Water"
PORCELAIN_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_Porcelain"
SERVICE_METAL_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_ServiceMetal"
WOOD_BENCH_MATERIAL_PATH = f"{MATERIALS_PATH}/M_Assembler_Test_WoodBench"

ENGINE_CUBE_PATH = "/Engine/BasicShapes/Cube"
ENGINE_CYLINDER_PATH = "/Engine/BasicShapes/Cylinder"


def log(message: str) -> None:
    unreal.log(f"[setup_bathhouse_rooms] {message}")


def ensure_folder(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        log(f"Created folder: {path}")


def get_cdo(blueprint):
    generated = blueprint.generated_class()
    if not generated:
        raise RuntimeError(f"Blueprint has no generated class: {blueprint.get_name()}")
    return unreal.get_default_object(generated)


def load_required_asset(asset_path: str):
    asset = unreal.load_asset(asset_path)
    if not asset:
        raise RuntimeError(f"Missing required asset: {asset_path}")
    return asset


def find_connector(blueprint, name: str):
    subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsys.k2_gather_subobject_data_for_blueprint(blueprint)
    for handle in handles:
        data = subsys.k2_find_subobject_data_from_handle(handle)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        if not obj:
            continue
        if isinstance(obj, unreal.PrototypeRoomConnectorComponent):
            obj_name = obj.get_name()
            if obj_name == name or obj_name == f"{name}_GEN_VARIABLE":
                return obj, handle
    return None, None


def find_marker(blueprint, name: str):
    subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsys.k2_gather_subobject_data_for_blueprint(blueprint)
    for handle in handles:
        data = subsys.k2_find_subobject_data_from_handle(handle)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        if not obj:
            continue
        if isinstance(obj, unreal.ButchDecorationMarkerComponent):
            obj_name = obj.get_name()
            if obj_name == name or obj_name == f"{name}_GEN_VARIABLE":
                return obj, handle
    return None, None


def find_mesh_component(blueprint, name: str):
    subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsys.k2_gather_subobject_data_for_blueprint(blueprint)
    for handle in handles:
        data = subsys.k2_find_subobject_data_from_handle(handle)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        if not obj:
            continue
        if isinstance(obj, unreal.StaticMeshComponent):
            obj_name = obj.get_name()
            if obj_name == name or obj_name == f"{name}_GEN_VARIABLE":
                return obj, handle
    return None, None


def create_blueprint(asset_name: str):
    asset_path = f"{ROOMS_PATH}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return unreal.load_asset(asset_path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", unreal.RoomModuleBase)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    blueprint = asset_tools.create_asset(asset_name, ROOMS_PATH, None, factory)
    if not blueprint:
        raise RuntimeError(f"Failed to create blueprint: {asset_path}")
    return blueprint


def add_feature_mesh(
    blueprint,
    name: str,
    mesh_asset,
    location: unreal.Vector,
    rotation: unreal.Rotator,
    scale: unreal.Vector,
    material=None,
    collision_profile="NoCollision",
):
    existing, _ = find_mesh_component(blueprint, name)
    if existing:
        existing.set_editor_property("StaticMesh", mesh_asset)
        existing.set_editor_property("RelativeLocation", location)
        existing.set_editor_property("RelativeRotation", rotation)
        existing.set_editor_property("RelativeScale3D", scale)
        existing.set_editor_property("Mobility", unreal.ComponentMobility.MOVABLE)
        existing.set_editor_property("CastShadow", False)
        if hasattr(existing, "set_collision_profile_name"):
            existing.set_collision_profile_name(collision_profile)
        if hasattr(existing, "set_generate_overlap_events"):
            existing.set_generate_overlap_events(False)
        if material:
            existing.set_material(0, material)
        return

    subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsys.k2_gather_subobject_data_for_blueprint(blueprint)
    if not handles:
        raise RuntimeError(f"No subobject handles found for {blueprint.get_name()}")

    params = unreal.AddNewSubobjectParams()
    params.set_editor_property("parent_handle", handles[0])
    params.set_editor_property("new_class", unreal.StaticMeshComponent)
    params.set_editor_property("blueprint_context", blueprint)
    params.set_editor_property("conform_transform_to_parent", True)

    new_handle, fail_reason = subsys.add_new_subobject(params)
    if not unreal.SubobjectDataBlueprintFunctionLibrary.is_handle_valid(new_handle):
        raise RuntimeError(f"Failed to add feature mesh {name} to {blueprint.get_name()}: {fail_reason}")

    subsys.rename_subobject(new_handle, name)
    data = subsys.k2_find_subobject_data_from_handle(new_handle)
    comp = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
    if not comp:
        raise RuntimeError(f"Failed to resolve static mesh component for {name}")

    comp.set_editor_property("StaticMesh", mesh_asset)
    comp.set_editor_property("RelativeLocation", location)
    comp.set_editor_property("RelativeRotation", rotation)
    comp.set_editor_property("RelativeScale3D", scale)
    comp.set_editor_property("Mobility", unreal.ComponentMobility.MOVABLE)
    comp.set_editor_property("CastShadow", False)
    if hasattr(comp, "set_collision_profile_name"):
        comp.set_collision_profile_name(collision_profile)
    if hasattr(comp, "set_generate_overlap_events"):
        comp.set_generate_overlap_events(False)
    if material:
        comp.set_material(0, material)


def remove_unwanted_feature_meshes(blueprint, allowed_names):
    subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsys.k2_gather_subobject_data_for_blueprint(blueprint)
    if not handles:
        return

    root_handle = handles[0]
    to_delete = []
    for handle in handles:
        data = subsys.k2_find_subobject_data_from_handle(handle)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        if not obj or not isinstance(obj, unreal.StaticMeshComponent):
            continue
        obj_name = obj.get_name().replace("_GEN_VARIABLE", "")
        if obj_name.startswith("Geo_") and obj_name not in allowed_names:
            to_delete.append(handle)

    if to_delete:
        subsys.delete_subobjects(root_handle, to_delete, blueprint)


def set_room_defaults(blueprint, room_id: str, room_type: str, weight: float, min_conn: int, max_conn: int, required: bool, color):
    cdo = get_cdo(blueprint)
    cdo.set_editor_property("RoomID", room_id)
    cdo.set_editor_property("RoomType", room_type)
    cdo.set_editor_property("Weight", weight)
    cdo.set_editor_property("MinConnections", min_conn)
    cdo.set_editor_property("MaxConnections", max_conn)
    cdo.set_editor_property("bRequired", required)
    cdo.set_editor_property("DebugColor", color)
    cdo.set_editor_property("bExpandGeneration", True)


def set_room_bounds(blueprint, full_size):
    cdo = get_cdo(blueprint)
    room_bounds = cdo.get_editor_property("RoomBoundsBox")
    if not room_bounds:
        raise RuntimeError(f"{blueprint.get_name()} has no RoomBoundsBox")
    half_extents = unreal.Vector(full_size.x * 0.5, full_size.y * 0.5, full_size.z * 0.5)
    room_bounds.set_editor_property("BoxExtent", half_extents)
    room_bounds.set_editor_property("RelativeLocation", unreal.Vector(0.0, 0.0, half_extents.z))

def disable_parametric_base(blueprint):
    cdo = get_cdo(blueprint)
    settings = cdo.get_editor_property("ParametricSettings")
    settings.set_editor_property("bEnabled", False)
    settings.set_editor_property("bDebugDrawSlicePass", False)
    cdo.set_editor_property("ParametricSettings", settings)


def enable_stock_graybox(blueprint, floor_thickness=FLOOR_THICKNESS, wall_thickness=WALL_THICKNESS, ceiling_thickness=CEILING_THICKNESS, door_width=DOOR_WIDTH, door_height=DOOR_HEIGHT):
    cdo = get_cdo(blueprint)
    settings = cdo.get_editor_property("StockAssemblySettings")
    settings.set_editor_property("bEnabled", True)
    settings.set_editor_property("FloorThickness", floor_thickness)
    settings.set_editor_property("WallThickness", wall_thickness)
    settings.set_editor_property("CeilingThickness", ceiling_thickness)
    settings.set_editor_property("DoorWidth", door_width)
    settings.set_editor_property("DoorHeight", door_height)
    cdo.set_editor_property("StockAssemblySettings", settings)


def set_allowed_neighbors(blueprint, room_types):
    cdo = get_cdo(blueprint)
    cdo.set_editor_property("AllowedNeighborRoomTypes", list(room_types))


def set_placement_rules(
    blueprint,
    role,
    allow_main_path=True,
    allow_branch=False,
    can_terminate=True,
    min_depth=0,
    max_depth=-1,
    max_instances=-1,
):
    cdo = get_cdo(blueprint)
    rules = cdo.get_editor_property("PlacementRules")
    rules.set_editor_property("PlacementRole", role)
    rules.set_editor_property("bAllowOnMainPath", allow_main_path)
    rules.set_editor_property("bAllowOnBranch", allow_branch)
    rules.set_editor_property("bCanTerminatePath", can_terminate)
    rules.set_editor_property("MinDepthFromStart", min_depth)
    rules.set_editor_property("MaxDepthFromStart", max_depth)
    rules.set_editor_property("MaxInstances", max_instances)
    cdo.set_editor_property("PlacementRules", rules)


def set_stock_footprint(blueprint, footprint_type):
    cdo = get_cdo(blueprint)
    settings = cdo.get_editor_property("StockAssemblySettings")
    settings.set_editor_property("FootprintType", footprint_type)
    cdo.set_editor_property("StockAssemblySettings", settings)


def set_stock_materials(blueprint, floor_material, wall_material, ceiling_material, legacy_material=None):
    cdo = get_cdo(blueprint)
    cdo.set_editor_property("FloorMaterialOverride", floor_material)
    cdo.set_editor_property("WallMaterialOverride", wall_material)
    cdo.set_editor_property("CeilingMaterialOverride", ceiling_material)
    cdo.set_editor_property("LegacyRoomMaterialOverride", legacy_material)


def enable_entry_player_start(blueprint):
    cdo = get_cdo(blueprint)
    cdo.set_editor_property("bSpawnPlayerStart", True)
    cdo.set_editor_property("PlayerStartLocalOffset", unreal.Vector(0.0, 0.0, 140.0))


def add_connector(blueprint, name: str, location: unreal.Vector, rotation: unreal.Rotator, direction):
    existing, existing_handle = find_connector(blueprint, name)
    if existing:
        existing.set_editor_property("SocketID", name)
        existing.set_editor_property("Direction", direction)
        existing.set_editor_property("RelativeLocation", location)
        existing.set_editor_property("RelativeRotation", rotation)
        return

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
        raise RuntimeError(f"Failed to resolve component object for {name}")

    comp.set_editor_property("RelativeLocation", location)
    comp.set_editor_property("RelativeRotation", rotation)
    comp.set_editor_property("SocketID", name)
    comp.set_editor_property("Direction", direction)


def add_marker(blueprint, name: str, location: unreal.Vector, rotation: unreal.Rotator, scale: unreal.Vector, marker_type, radius=80.0, allow_steam=False, tags=None):
    existing, existing_handle = find_marker(blueprint, name)
    if existing:
        existing.set_editor_property("RelativeLocation", location)
        existing.set_editor_property("RelativeRotation", rotation)
        existing.set_editor_property("RelativeScale3D", scale)
        existing.set_editor_property("MarkerID", name)
        existing.set_editor_property("MarkerType", marker_type)
        existing.set_editor_property("Radius", radius)
        existing.set_editor_property("bAllowSteamFx", allow_steam)
        existing.set_editor_property("SemanticTags", tags or [])
        return

    subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsys.k2_gather_subobject_data_for_blueprint(blueprint)
    if not handles:
        raise RuntimeError(f"No subobject handles found for {blueprint.get_name()}")

    params = unreal.AddNewSubobjectParams()
    params.set_editor_property("parent_handle", handles[0])
    params.set_editor_property("new_class", unreal.ButchDecorationMarkerComponent)
    params.set_editor_property("blueprint_context", blueprint)
    params.set_editor_property("conform_transform_to_parent", True)

    new_handle, fail_reason = subsys.add_new_subobject(params)
    if not unreal.SubobjectDataBlueprintFunctionLibrary.is_handle_valid(new_handle):
        raise RuntimeError(f"Failed to add marker {name} to {blueprint.get_name()}: {fail_reason}")

    subsys.rename_subobject(new_handle, name)
    data = subsys.k2_find_subobject_data_from_handle(new_handle)
    comp = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
    if not comp:
        raise RuntimeError(f"Failed to resolve marker object for {name}")

    comp.set_editor_property("RelativeLocation", location)
    comp.set_editor_property("RelativeRotation", rotation)
    comp.set_editor_property("RelativeScale3D", scale)
    comp.set_editor_property("MarkerID", name)
    comp.set_editor_property("MarkerType", marker_type)
    comp.set_editor_property("Radius", radius)
    comp.set_editor_property("bAllowSteamFx", allow_steam)
    comp.set_editor_property("SemanticTags", tags or [])


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
        comp_name = obj.get_name()
        base_name = comp_name.replace("_GEN_VARIABLE", "")
        if base_name.startswith("Conn_") and base_name not in allowed_names:
            to_delete.append(handle)

    if to_delete:
        subsys.delete_subobjects(root_handle, to_delete, blueprint)


def remove_unwanted_markers(blueprint, allowed_names):
    subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsys.k2_gather_subobject_data_for_blueprint(blueprint)
    if not handles:
        return

    root_handle = handles[0]
    to_delete = []
    for handle in handles:
        data = subsys.k2_find_subobject_data_from_handle(handle)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        if not obj or not isinstance(obj, unreal.ButchDecorationMarkerComponent):
            continue
        obj_name = obj.get_name().replace("_GEN_VARIABLE", "")
        if obj_name.startswith("Mark_") and obj_name not in allowed_names:
            to_delete.append(handle)

    if to_delete:
        subsys.delete_subobjects(root_handle, to_delete, blueprint)


def add_cardinal_connectors(blueprint, full_size, include_east=True, include_west=True, include_north=True, include_south=True):
    half_x = full_size.x * 0.5
    half_y = full_size.y * 0.5
    door_z = 150.0
    allowed = set()

    if include_north:
        add_connector(blueprint, "Conn_N", unreal.Vector(0.0, half_y, door_z), unreal.Rotator(pitch=0.0, yaw=90.0, roll=0.0), unreal.RoomConnectorDirection.NORTH)
        allowed.add("Conn_N")
    if include_south:
        add_connector(blueprint, "Conn_S", unreal.Vector(0.0, -half_y, door_z), unreal.Rotator(pitch=0.0, yaw=-90.0, roll=0.0), unreal.RoomConnectorDirection.SOUTH)
        allowed.add("Conn_S")
    if include_east:
        add_connector(blueprint, "Conn_E", unreal.Vector(half_x, 0.0, door_z), unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0), unreal.RoomConnectorDirection.EAST)
        allowed.add("Conn_E")
    if include_west:
        add_connector(blueprint, "Conn_W", unreal.Vector(-half_x, 0.0, door_z), unreal.Rotator(pitch=0.0, yaw=180.0, roll=0.0), unreal.RoomConnectorDirection.WEST)
        allowed.add("Conn_W")

    remove_unwanted_connectors(blueprint, allowed)


def set_entry_markers(blueprint):
    allowed = set()
    add_marker(blueprint, "Mark_PropDesk", unreal.Vector(0.0, 0.0, 40.0), unreal.Rotator(), unreal.Vector(1.2, 0.5, 0.8), unreal.ButchDecorationMarkerType.GENERIC_PROP, radius=90.0)
    allowed.add("Mark_PropDesk")
    add_marker(blueprint, "Mark_AudioAmbience", unreal.Vector(0.0, 0.0, 300.0), unreal.Rotator(), unreal.Vector(0.25, 0.25, 0.25), unreal.ButchDecorationMarkerType.AUDIO_POINT, radius=150.0)
    allowed.add("Mark_AudioAmbience")
    remove_unwanted_markers(blueprint, allowed)


def set_locker_markers(blueprint):
    allowed = set()
    add_marker(blueprint, "Mark_LockersWest", unreal.Vector(-420.0, 0.0, 60.0), unreal.Rotator(), unreal.Vector(0.45, 2.8, 1.5), unreal.ButchDecorationMarkerType.GENERIC_PROP, radius=100.0)
    allowed.add("Mark_LockersWest")
    add_marker(blueprint, "Mark_PipeCeiling", unreal.Vector(0.0, 0.0, 310.0), unreal.Rotator(pitch=0.0, yaw=90.0, roll=0.0), unreal.Vector(8.0, 0.12, 0.12), unreal.ButchDecorationMarkerType.PIPE_LANE, radius=120.0, tags=["CeilingPipe"])
    allowed.add("Mark_PipeCeiling")
    remove_unwanted_markers(blueprint, allowed)


def set_straight_hall_markers(blueprint):
    allowed = set()
    add_marker(blueprint, "Mark_PipeRun", unreal.Vector(0.0, 0.0, 290.0), unreal.Rotator(pitch=0.0, yaw=90.0, roll=0.0), unreal.Vector(5.5, 0.12, 0.12), unreal.ButchDecorationMarkerType.PIPE_LANE, radius=100.0, tags=["CeilingPipe"])
    allowed.add("Mark_PipeRun")
    add_marker(blueprint, "Mark_LeakEast", unreal.Vector(150.0, 0.0, 220.0), unreal.Rotator(), unreal.Vector(0.12, 0.12, 0.4), unreal.ButchDecorationMarkerType.LEAK_CANDIDATE, radius=80.0, allow_steam=True, tags=["PipeLeak"])
    allowed.add("Mark_LeakEast")
    remove_unwanted_markers(blueprint, allowed)


def set_corner_markers(blueprint):
    allowed = set()
    add_marker(blueprint, "Mark_PipeSouth", unreal.Vector(0.0, -80.0, 290.0), unreal.Rotator(pitch=0.0, yaw=90.0, roll=0.0), unreal.Vector(2.0, 0.12, 0.12), unreal.ButchDecorationMarkerType.PIPE_LANE, radius=90.0, tags=["CeilingPipe"])
    allowed.add("Mark_PipeSouth")
    add_marker(blueprint, "Mark_PipeEast", unreal.Vector(80.0, 0.0, 290.0), unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0), unreal.Vector(2.0, 0.12, 0.12), unreal.ButchDecorationMarkerType.PIPE_LANE, radius=90.0, tags=["CeilingPipe"])
    allowed.add("Mark_PipeEast")
    add_marker(blueprint, "Mark_LeakCorner", unreal.Vector(110.0, -110.0, 220.0), unreal.Rotator(), unreal.Vector(0.12, 0.12, 0.4), unreal.ButchDecorationMarkerType.STEAM_VENT_CANDIDATE, radius=80.0, allow_steam=True, tags=["PipeLeak"])
    allowed.add("Mark_LeakCorner")
    remove_unwanted_markers(blueprint, allowed)


def set_stair_markers(blueprint):
    allowed = set()
    add_marker(blueprint, "Mark_PipeLower", unreal.Vector(130.0, -180.0, 300.0), unreal.Rotator(pitch=0.0, yaw=90.0, roll=0.0), unreal.Vector(2.2, 0.12, 0.12), unreal.ButchDecorationMarkerType.PIPE_LANE, radius=100.0, tags=["CeilingPipe"])
    allowed.add("Mark_PipeLower")
    add_marker(blueprint, "Mark_PipeUpper", unreal.Vector(-130.0, 180.0, 620.0), unreal.Rotator(pitch=0.0, yaw=90.0, roll=0.0), unreal.Vector(2.2, 0.12, 0.12), unreal.ButchDecorationMarkerType.PIPE_LANE, radius=100.0, tags=["CeilingPipe"])
    allowed.add("Mark_PipeUpper")
    add_marker(blueprint, "Mark_LeakUpper", unreal.Vector(150.0, 220.0, 560.0), unreal.Rotator(), unreal.Vector(0.12, 0.12, 0.4), unreal.ButchDecorationMarkerType.LEAK_CANDIDATE, radius=90.0, allow_steam=True, tags=["PipeLeak"])
    allowed.add("Mark_LeakUpper")
    remove_unwanted_markers(blueprint, allowed)


def set_cold_plunge_markers(blueprint):
    allowed = set()
    add_marker(blueprint, "Mark_ColdPlungeAudio", unreal.Vector(0.0, 0.0, 150.0), unreal.Rotator(), unreal.Vector(0.2, 0.2, 0.2), unreal.ButchDecorationMarkerType.AUDIO_POINT, radius=120.0)
    allowed.add("Mark_ColdPlungeAudio")
    remove_unwanted_markers(blueprint, allowed)


def set_storage_markers(blueprint):
    allowed = set()
    add_marker(blueprint, "Mark_StorageProp", unreal.Vector(0.0, 0.0, 60.0), unreal.Rotator(), unreal.Vector(1.4, 1.2, 1.2), unreal.ButchDecorationMarkerType.GENERIC_PROP, radius=100.0)
    allowed.add("Mark_StorageProp")
    remove_unwanted_markers(blueprint, allowed)


def set_pool_features(blueprint, cube_mesh, cylinder_mesh, water_material, porcelain_material):
    allowed = set()
    add_feature_mesh(
        blueprint,
        "Geo_PoolBasin",
        cylinder_mesh,
        unreal.Vector(0.0, 0.0, 18.0),
        unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0),
        unreal.Vector(3.2, 3.2, 0.35),
        porcelain_material,
    )
    allowed.add("Geo_PoolBasin")
    add_feature_mesh(
        blueprint,
        "Geo_PoolWater",
        cylinder_mesh,
        unreal.Vector(0.0, 0.0, 32.0),
        unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0),
        unreal.Vector(2.85, 2.85, 0.12),
        water_material,
    )
    allowed.add("Geo_PoolWater")
    add_feature_mesh(
        blueprint,
        "Geo_PoolBenchWest",
        cube_mesh,
        unreal.Vector(-520.0, 0.0, 38.0),
        unreal.Rotator(),
        unreal.Vector(0.35, 1.9, 0.28),
        porcelain_material,
    )
    allowed.add("Geo_PoolBenchWest")
    add_feature_mesh(
        blueprint,
        "Geo_PoolBenchEast",
        cube_mesh,
        unreal.Vector(520.0, 0.0, 38.0),
        unreal.Rotator(),
        unreal.Vector(0.35, 1.9, 0.28),
        porcelain_material,
    )
    allowed.add("Geo_PoolBenchEast")
    remove_unwanted_feature_meshes(blueprint, allowed)


def set_cold_plunge_features(blueprint, cylinder_mesh, water_material, porcelain_material):
    allowed = set()
    add_feature_mesh(
        blueprint,
        "Geo_ColdPlungeBasin",
        cylinder_mesh,
        unreal.Vector(0.0, 0.0, 18.0),
        unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0),
        unreal.Vector(1.8, 1.8, 0.28),
        porcelain_material,
    )
    allowed.add("Geo_ColdPlungeBasin")
    add_feature_mesh(
        blueprint,
        "Geo_ColdPlungeWater",
        cylinder_mesh,
        unreal.Vector(0.0, 0.0, 30.0),
        unreal.Rotator(pitch=90.0, yaw=0.0, roll=0.0),
        unreal.Vector(1.55, 1.55, 0.10),
        water_material,
    )
    allowed.add("Geo_ColdPlungeWater")
    remove_unwanted_feature_meshes(blueprint, allowed)


def set_toilet_features(blueprint, cube_mesh, porcelain_material, wall_material):
    allowed = set()
    add_feature_mesh(
        blueprint,
        "Geo_StallDividerLeft",
        cube_mesh,
        unreal.Vector(-120.0, 120.0, 120.0),
        unreal.Rotator(),
        unreal.Vector(0.08, 1.4, 2.4),
        wall_material,
    )
    allowed.add("Geo_StallDividerLeft")
    add_feature_mesh(
        blueprint,
        "Geo_StallDividerRight",
        cube_mesh,
        unreal.Vector(120.0, 120.0, 120.0),
        unreal.Rotator(),
        unreal.Vector(0.08, 1.4, 2.4),
        wall_material,
    )
    allowed.add("Geo_StallDividerRight")
    add_feature_mesh(
        blueprint,
        "Geo_ToiletOne",
        cube_mesh,
        unreal.Vector(-120.0, 220.0, 26.0),
        unreal.Rotator(),
        unreal.Vector(0.24, 0.34, 0.26),
        porcelain_material,
    )
    allowed.add("Geo_ToiletOne")
    add_feature_mesh(
        blueprint,
        "Geo_ToiletTwo",
        cube_mesh,
        unreal.Vector(120.0, 220.0, 26.0),
        unreal.Rotator(),
        unreal.Vector(0.24, 0.34, 0.26),
        porcelain_material,
    )
    allowed.add("Geo_ToiletTwo")
    add_feature_mesh(
        blueprint,
        "Geo_SinkBlock",
        cube_mesh,
        unreal.Vector(0.0, -180.0, 40.0),
        unreal.Rotator(),
        unreal.Vector(0.9, 0.22, 0.35),
        porcelain_material,
    )
    allowed.add("Geo_SinkBlock")
    remove_unwanted_feature_meshes(blueprint, allowed)


def set_storage_features(blueprint, cube_mesh, service_material):
    allowed = set()
    add_feature_mesh(
        blueprint,
        "Geo_ShelfWest",
        cube_mesh,
        unreal.Vector(-240.0, 0.0, 90.0),
        unreal.Rotator(),
        unreal.Vector(0.18, 1.4, 1.7),
        service_material,
    )
    allowed.add("Geo_ShelfWest")
    add_feature_mesh(
        blueprint,
        "Geo_ShelfEast",
        cube_mesh,
        unreal.Vector(240.0, 0.0, 90.0),
        unreal.Rotator(),
        unreal.Vector(0.18, 1.4, 1.7),
        service_material,
    )
    allowed.add("Geo_ShelfEast")
    add_feature_mesh(
        blueprint,
        "Geo_CrateStack",
        cube_mesh,
        unreal.Vector(0.0, 180.0, 35.0),
        unreal.Rotator(),
        unreal.Vector(0.55, 0.55, 0.55),
        service_material,
    )
    allowed.add("Geo_CrateStack")
    remove_unwanted_feature_meshes(blueprint, allowed)


def set_steam_room_features(blueprint, cube_mesh, cylinder_mesh, wood_material, service_material):
    allowed = set()
    add_feature_mesh(
        blueprint,
        "Geo_BenchNorth",
        cube_mesh,
        unreal.Vector(0.0, 240.0, 36.0),
        unreal.Rotator(),
        unreal.Vector(1.5, 0.32, 0.28),
        wood_material,
    )
    allowed.add("Geo_BenchNorth")
    add_feature_mesh(
        blueprint,
        "Geo_BenchSouth",
        cube_mesh,
        unreal.Vector(0.0, -240.0, 36.0),
        unreal.Rotator(),
        unreal.Vector(1.5, 0.32, 0.28),
        wood_material,
    )
    allowed.add("Geo_BenchSouth")
    add_feature_mesh(
        blueprint,
        "Geo_SteamVent",
        cylinder_mesh,
        unreal.Vector(0.0, 0.0, 18.0),
        unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0),
        unreal.Vector(0.35, 0.35, 0.16),
        service_material,
    )
    allowed.add("Geo_SteamVent")
    remove_unwanted_feature_meshes(blueprint, allowed)


def compile_and_save(blueprint):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    asset_path = unreal.EditorAssetLibrary.get_path_name_for_loaded_asset(blueprint)
    unreal.EditorAssetLibrary.save_asset(asset_path)


def main():
    ensure_folder(ROOMS_PATH)
    floor_material = load_required_asset(FLOOR_MATERIAL_PATH)
    wall_material = load_required_asset(WALL_MATERIAL_PATH)
    ceiling_material = load_required_asset(CEILING_MATERIAL_PATH)
    entry_floor_material = load_required_asset(ENTRY_FLOOR_MATERIAL_PATH)
    entry_wall_material = load_required_asset(ENTRY_WALL_MATERIAL_PATH)
    locker_floor_material = load_required_asset(LOCKER_FLOOR_MATERIAL_PATH)
    locker_wall_material = load_required_asset(LOCKER_WALL_MATERIAL_PATH)
    hall_floor_material = load_required_asset(HALL_FLOOR_MATERIAL_PATH)
    hall_wall_material = load_required_asset(HALL_WALL_MATERIAL_PATH)
    stair_floor_material = load_required_asset(STAIR_FLOOR_MATERIAL_PATH)
    stair_wall_material = load_required_asset(STAIR_WALL_MATERIAL_PATH)
    stair_ceiling_material = load_required_asset(STAIR_CEILING_MATERIAL_PATH)
    water_material = load_required_asset(WATER_MATERIAL_PATH)
    porcelain_material = load_required_asset(PORCELAIN_MATERIAL_PATH)
    service_metal_material = load_required_asset(SERVICE_METAL_MATERIAL_PATH)
    wood_bench_material = load_required_asset(WOOD_BENCH_MATERIAL_PATH)
    cube_mesh = load_required_asset(ENGINE_CUBE_PATH)
    cylinder_mesh = load_required_asset(ENGINE_CYLINDER_PATH)

    # Entry / Reception
    entry = create_blueprint("BP_Room_EntryReception")
    set_room_defaults(entry, "EntryReception", "EntryReception", 1.0, 1, 2, True, unreal.LinearColor(0.2, 0.9, 0.6, 1.0))
    entry_size = unreal.Vector(1200.0, 1000.0, 380.0)
    set_room_bounds(entry, entry_size)
    disable_parametric_base(entry)
    enable_stock_graybox(entry)
    set_stock_materials(entry, entry_floor_material, entry_wall_material, ceiling_material)
    set_placement_rules(entry, unreal.RoomPlacementRole.START, allow_main_path=False, allow_branch=False, can_terminate=True, min_depth=0, max_depth=0, max_instances=1)
    set_allowed_neighbors(entry, ["PublicHallStraight"])
    enable_entry_player_start(entry)
    add_cardinal_connectors(entry, entry_size, include_east=False, include_west=False, include_north=True, include_south=False)
    set_entry_markers(entry)
    compile_and_save(entry)

    # Public Hallway Straight
    hall = create_blueprint("BP_Room_PublicHall_Straight")
    set_room_defaults(hall, "PublicHallStraight", "PublicHallStraight", 2.0, 1, 2, True, unreal.LinearColor(0.6, 0.6, 0.95, 1.0))
    hall_size = unreal.Vector(400.0, 600.0, 340.0)
    set_room_bounds(hall, hall_size)
    disable_parametric_base(hall)
    enable_stock_graybox(hall)
    set_stock_materials(hall, hall_floor_material, hall_wall_material, ceiling_material)
    set_placement_rules(hall, unreal.RoomPlacementRole.MAIN_PATH, allow_main_path=True, allow_branch=True, can_terminate=True, min_depth=0, max_depth=-1, max_instances=-1)
    set_allowed_neighbors(hall, ["EntryReception", "PublicHallStraight", "PublicHallCorner", "LockerHall", "WashShower", "PoolHall", "Sauna", "BoilerService", "ColdPlunge", "SteamRoom", "Toilet", "Storage"])
    add_cardinal_connectors(hall, hall_size, include_east=False, include_west=False, include_north=True, include_south=True)
    set_straight_hall_markers(hall)
    compile_and_save(hall)

    # Public Hallway Corner Cell (South + East, rotates to other quadrants)
    corner = create_blueprint("BP_Room_PublicHall_Corner")
    set_room_defaults(corner, "PublicHallCorner", "PublicHallCorner", 1.2, 1, 2, False, unreal.LinearColor(0.7, 0.7, 1.0, 1.0))
    corner_size = unreal.Vector(400.0, 400.0, 340.0)
    set_room_bounds(corner, corner_size)
    disable_parametric_base(corner)
    enable_stock_graybox(corner)
    set_stock_footprint(corner, unreal.RoomStockFootprintType.CORNER_SOUTH_EAST)
    set_stock_materials(corner, hall_floor_material, hall_wall_material, ceiling_material)
    set_placement_rules(corner, unreal.RoomPlacementRole.MAIN_PATH, allow_main_path=True, allow_branch=True, can_terminate=True, min_depth=1, max_depth=-1, max_instances=-1)
    set_allowed_neighbors(corner, ["PublicHallStraight", "PublicHallCorner", "LockerHall", "WashShower", "PoolHall", "Sauna", "BoilerService", "ColdPlunge", "SteamRoom", "Toilet", "Storage"])
    add_cardinal_connectors(corner, corner_size, include_east=True, include_west=False, include_north=False, include_south=True)
    set_corner_markers(corner)
    compile_and_save(corner)

    # Locker Hall
    locker = create_blueprint("BP_Room_LockerHall")
    set_room_defaults(locker, "LockerHall", "LockerHall", 1.0, 2, 4, True, unreal.LinearColor(0.4, 0.8, 0.9, 1.0))
    locker_size = unreal.Vector(1200.0, 1600.0, 380.0)
    set_room_bounds(locker, locker_size)
    disable_parametric_base(locker)
    enable_stock_graybox(locker)
    set_stock_materials(locker, locker_floor_material, locker_wall_material, ceiling_material)
    set_placement_rules(locker, unreal.RoomPlacementRole.MAIN_PATH, allow_main_path=True, allow_branch=False, can_terminate=False, min_depth=2, max_depth=4, max_instances=1)
    set_allowed_neighbors(locker, ["PublicHallStraight", "PublicHallCorner", "WashShower"])
    add_cardinal_connectors(locker, locker_size, include_east=True, include_west=True, include_north=True, include_south=True)
    set_locker_markers(locker)
    compile_and_save(locker)

    # Wash / Shower transition
    wash = create_blueprint("BP_Room_WashShower")
    set_room_defaults(wash, "WashShower", "WashShower", 0.9, 2, 4, True, unreal.LinearColor(0.55, 0.78, 0.98, 1.0))
    wash_size = unreal.Vector(1000.0, 1200.0, 380.0)
    set_room_bounds(wash, wash_size)
    disable_parametric_base(wash)
    enable_stock_graybox(wash)
    set_stock_materials(wash, hall_floor_material, hall_wall_material, ceiling_material)
    set_placement_rules(wash, unreal.RoomPlacementRole.MAIN_PATH, allow_main_path=True, allow_branch=False, can_terminate=False, min_depth=3, max_depth=6, max_instances=1)
    set_allowed_neighbors(wash, ["LockerHall", "PublicHallStraight", "PublicHallCorner", "PoolHall", "Sauna", "SteamRoom", "ColdPlunge", "Toilet"])
    add_cardinal_connectors(wash, wash_size, include_east=True, include_west=True, include_north=True, include_south=True)
    compile_and_save(wash)

    # Pool Hall anchor
    pool = create_blueprint("BP_Room_PoolHall")
    set_room_defaults(pool, "PoolHall", "PoolHall", 0.8, 1, 4, True, unreal.LinearColor(0.2, 0.62, 0.95, 1.0))
    pool_size = unreal.Vector(1800.0, 1800.0, 420.0)
    set_room_bounds(pool, pool_size)
    disable_parametric_base(pool)
    enable_stock_graybox(pool)
    set_stock_materials(pool, hall_floor_material, hall_wall_material, ceiling_material)
    set_placement_rules(pool, unreal.RoomPlacementRole.MAIN_PATH, allow_main_path=True, allow_branch=False, can_terminate=True, min_depth=4, max_depth=8, max_instances=1)
    set_allowed_neighbors(pool, ["WashShower", "PublicHallStraight", "PublicHallCorner", "Sauna", "BoilerService", "ColdPlunge", "SteamRoom", "Storage"])
    add_cardinal_connectors(pool, pool_size, include_east=True, include_west=True, include_north=True, include_south=True)
    set_pool_features(pool, cube_mesh, cylinder_mesh, water_material, porcelain_material)
    compile_and_save(pool)

    # Cold plunge branch room
    plunge = create_blueprint("BP_Room_ColdPlunge")
    set_room_defaults(plunge, "ColdPlunge", "ColdPlunge", 0.45, 1, 1, False, unreal.LinearColor(0.20, 0.72, 0.95, 1.0))
    plunge_size = unreal.Vector(900.0, 900.0, 360.0)
    set_room_bounds(plunge, plunge_size)
    disable_parametric_base(plunge)
    enable_stock_graybox(plunge)
    set_stock_materials(plunge, hall_floor_material, hall_wall_material, ceiling_material)
    set_placement_rules(plunge, unreal.RoomPlacementRole.BRANCH, allow_main_path=False, allow_branch=True, can_terminate=True, min_depth=4, max_depth=9, max_instances=1)
    set_allowed_neighbors(plunge, ["PoolHall", "WashShower", "PublicHallStraight", "PublicHallCorner", "SteamRoom"])
    add_cardinal_connectors(plunge, plunge_size, include_east=False, include_west=False, include_north=False, include_south=True)
    set_cold_plunge_features(plunge, cylinder_mesh, water_material, porcelain_material)
    set_cold_plunge_markers(plunge)
    compile_and_save(plunge)

    # Sauna branch room
    sauna = create_blueprint("BP_Room_Sauna")
    set_room_defaults(sauna, "Sauna", "Sauna", 0.6, 1, 1, True, unreal.LinearColor(0.95, 0.62, 0.28, 1.0))
    sauna_size = unreal.Vector(800.0, 800.0, 360.0)
    set_room_bounds(sauna, sauna_size)
    disable_parametric_base(sauna)
    enable_stock_graybox(sauna)
    set_stock_materials(sauna, entry_floor_material, entry_wall_material, ceiling_material)
    set_placement_rules(sauna, unreal.RoomPlacementRole.BRANCH, allow_main_path=False, allow_branch=True, can_terminate=True, min_depth=4, max_depth=9, max_instances=1)
    set_allowed_neighbors(sauna, ["WashShower", "PoolHall", "PublicHallStraight", "PublicHallCorner"])
    add_cardinal_connectors(sauna, sauna_size, include_east=False, include_west=False, include_north=False, include_south=True)
    compile_and_save(sauna)

    # Steam room branch room
    steam = create_blueprint("BP_Room_SteamRoom")
    set_room_defaults(steam, "SteamRoom", "SteamRoom", 0.42, 1, 1, False, unreal.LinearColor(0.86, 0.70, 0.34, 1.0))
    steam_size = unreal.Vector(900.0, 900.0, 360.0)
    set_room_bounds(steam, steam_size)
    disable_parametric_base(steam)
    enable_stock_graybox(steam)
    set_stock_materials(steam, hall_floor_material, hall_wall_material, ceiling_material)
    set_placement_rules(steam, unreal.RoomPlacementRole.BRANCH, allow_main_path=False, allow_branch=True, can_terminate=True, min_depth=4, max_depth=9, max_instances=1)
    set_allowed_neighbors(steam, ["WashShower", "PoolHall", "PublicHallStraight", "PublicHallCorner", "ColdPlunge"])
    add_cardinal_connectors(steam, steam_size, include_east=False, include_west=False, include_north=False, include_south=True)
    set_steam_room_features(steam, cube_mesh, cylinder_mesh, wood_bench_material, service_metal_material)
    compile_and_save(steam)

    # Toilet branch room
    toilet = create_blueprint("BP_Room_Toilet")
    set_room_defaults(toilet, "Toilet", "Toilet", 0.32, 1, 1, False, unreal.LinearColor(0.72, 0.82, 0.88, 1.0))
    toilet_size = unreal.Vector(800.0, 1000.0, 360.0)
    set_room_bounds(toilet, toilet_size)
    disable_parametric_base(toilet)
    enable_stock_graybox(toilet)
    set_stock_materials(toilet, locker_floor_material, locker_wall_material, ceiling_material)
    set_placement_rules(toilet, unreal.RoomPlacementRole.BRANCH, allow_main_path=False, allow_branch=True, can_terminate=True, min_depth=2, max_depth=6, max_instances=1)
    set_allowed_neighbors(toilet, ["LockerHall", "WashShower", "PublicHallStraight", "PublicHallCorner"])
    add_cardinal_connectors(toilet, toilet_size, include_east=False, include_west=False, include_north=False, include_south=True)
    set_toilet_features(toilet, cube_mesh, porcelain_material, locker_wall_material)
    compile_and_save(toilet)

    # Boiler / service branch room
    boiler = create_blueprint("BP_Room_BoilerService")
    set_room_defaults(boiler, "BoilerService", "BoilerService", 0.35, 1, 1, True, unreal.LinearColor(0.85, 0.52, 0.22, 1.0))
    boiler_size = unreal.Vector(1000.0, 1000.0, 380.0)
    set_room_bounds(boiler, boiler_size)
    disable_parametric_base(boiler)
    enable_stock_graybox(boiler)
    set_stock_materials(boiler, stair_floor_material, stair_wall_material, stair_ceiling_material)
    set_placement_rules(boiler, unreal.RoomPlacementRole.BRANCH, allow_main_path=False, allow_branch=True, can_terminate=True, min_depth=4, max_depth=9, max_instances=1)
    set_allowed_neighbors(boiler, ["PoolHall", "PublicHallStraight", "PublicHallCorner"])
    add_cardinal_connectors(boiler, boiler_size, include_east=False, include_west=False, include_north=False, include_south=True)
    compile_and_save(boiler)

    # Storage branch room
    storage = create_blueprint("BP_Room_Storage")
    set_room_defaults(storage, "Storage", "Storage", 0.30, 1, 1, False, unreal.LinearColor(0.55, 0.50, 0.42, 1.0))
    storage_size = unreal.Vector(900.0, 900.0, 360.0)
    set_room_bounds(storage, storage_size)
    disable_parametric_base(storage)
    enable_stock_graybox(storage)
    set_stock_materials(storage, stair_floor_material, stair_wall_material, stair_ceiling_material)
    set_placement_rules(storage, unreal.RoomPlacementRole.BRANCH, allow_main_path=False, allow_branch=True, can_terminate=True, min_depth=3, max_depth=8, max_instances=1)
    set_allowed_neighbors(storage, ["PublicHallStraight", "PublicHallCorner", "BoilerService", "PoolHall"])
    add_cardinal_connectors(storage, storage_size, include_east=False, include_west=False, include_north=False, include_south=True)
    set_storage_features(storage, cube_mesh, service_metal_material)
    set_storage_markers(storage)
    compile_and_save(storage)

    # Public Hallway Stair Up (enters south at ground, exits north one level up)
    stair = create_blueprint("BP_Room_PublicHall_Stair_Up")
    set_room_defaults(stair, "PublicHallStairUp", "PublicHallStair", 0.35, 2, 2, False, unreal.LinearColor(0.85, 0.72, 0.35, 1.0))
    stair_size = unreal.Vector(400.0, 800.0, 760.0)
    set_room_bounds(stair, stair_size)
    disable_parametric_base(stair)
    enable_stock_graybox(stair)
    set_stock_footprint(stair, unreal.RoomStockFootprintType.STAIR_SOUTH_TO_NORTH_UP)
    set_stock_materials(stair, stair_floor_material, stair_wall_material, stair_ceiling_material)
    set_placement_rules(stair, unreal.RoomPlacementRole.VERTICAL, allow_main_path=False, allow_branch=False, can_terminate=False, min_depth=0, max_depth=-1, max_instances=2)
    set_allowed_neighbors(stair, ["PublicHallStraight", "PublicHallCorner", "PoolHall", "BoilerService", "Storage"])
    add_connector(stair, "Conn_S", unreal.Vector(0.0, -stair_size.y * 0.5, 150.0), unreal.Rotator(pitch=0.0, yaw=-90.0, roll=0.0), unreal.RoomConnectorDirection.SOUTH)
    add_connector(stair, "Conn_N", unreal.Vector(0.0, stair_size.y * 0.5, 550.0), unreal.Rotator(pitch=0.0, yaw=90.0, roll=0.0), unreal.RoomConnectorDirection.NORTH)
    remove_unwanted_connectors(stair, {"Conn_S", "Conn_N"})
    set_stair_markers(stair)
    compile_and_save(stair)

    log("Room setup complete.")


if __name__ == "__main__":
    main()
