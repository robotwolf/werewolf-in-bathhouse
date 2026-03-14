import unreal


ROOMS_PATH = "/Game/WerewolfBH/Blueprints/Rooms"
MATERIALS_PATH = "/Game/LevelPrototyping/Materials"
FLOOR_MATERIAL_PATH = f"{MATERIALS_PATH}/MI_PrototypeGrid_TopDark"
WALL_MATERIAL_PATH = f"{MATERIALS_PATH}/MI_PrototypeGrid_Gray"
CEILING_MATERIAL_PATH = f"{MATERIALS_PATH}/MI_PrototypeGrid_Gray_02"


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


def set_room_defaults(blueprint, room_id: str, room_type: str, weight: float, min_conn: int, max_conn: int, required: bool, color):
    cdo = get_cdo(blueprint)
    cdo.set_editor_property("RoomID", room_id)
    cdo.set_editor_property("RoomType", room_type)
    cdo.set_editor_property("Weight", weight)
    cdo.set_editor_property("MinConnections", min_conn)
    cdo.set_editor_property("MaxConnections", max_conn)
    cdo.set_editor_property("bRequired", required)
    cdo.set_editor_property("DebugColor", color)


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


def enable_stock_graybox(blueprint, floor_thickness=20.0, wall_thickness=30.0, ceiling_thickness=20.0, door_width=200.0, door_height=260.0):
    cdo = get_cdo(blueprint)
    settings = cdo.get_editor_property("StockAssemblySettings")
    settings.set_editor_property("bEnabled", True)
    settings.set_editor_property("FloorThickness", floor_thickness)
    settings.set_editor_property("WallThickness", wall_thickness)
    settings.set_editor_property("CeilingThickness", ceiling_thickness)
    settings.set_editor_property("DoorWidth", door_width)
    settings.set_editor_property("DoorHeight", door_height)
    cdo.set_editor_property("StockAssemblySettings", settings)


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


def compile_and_save(blueprint):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    asset_path = unreal.EditorAssetLibrary.get_path_name_for_loaded_asset(blueprint)
    unreal.EditorAssetLibrary.save_asset(asset_path)


def main():
    ensure_folder(ROOMS_PATH)
    floor_material = load_required_asset(FLOOR_MATERIAL_PATH)
    wall_material = load_required_asset(WALL_MATERIAL_PATH)
    ceiling_material = load_required_asset(CEILING_MATERIAL_PATH)

    # Entry / Reception
    entry = create_blueprint("BP_Room_EntryReception")
    set_room_defaults(entry, "EntryReception", "EntryReception", 1.0, 1, 3, True, unreal.LinearColor(0.2, 0.9, 0.6, 1.0))
    entry_size = unreal.Vector(1200.0, 1000.0, 380.0)
    set_room_bounds(entry, entry_size)
    disable_parametric_base(entry)
    enable_stock_graybox(entry)
    set_stock_materials(entry, floor_material, wall_material, ceiling_material)
    enable_entry_player_start(entry)
    # Two primary exits for prototype: north + south.
    add_cardinal_connectors(entry, entry_size, include_east=False, include_west=False, include_north=True, include_south=True)
    compile_and_save(entry)

    # Locker Hall
    locker = create_blueprint("BP_Room_LockerHall")
    set_room_defaults(locker, "LockerHall", "LockerHall", 1.2, 2, 4, True, unreal.LinearColor(0.4, 0.8, 0.9, 1.0))
    locker_size = unreal.Vector(1200.0, 1600.0, 380.0)
    set_room_bounds(locker, locker_size)
    disable_parametric_base(locker)
    enable_stock_graybox(locker)
    set_stock_materials(locker, floor_material, wall_material, ceiling_material)
    add_cardinal_connectors(locker, locker_size, include_east=True, include_west=True, include_north=True, include_south=True)
    compile_and_save(locker)

    # Public Hallway Straight
    hall = create_blueprint("BP_Room_PublicHall_Straight")
    set_room_defaults(hall, "PublicHallStraight", "PublicHallStraight", 2.0, 2, 2, True, unreal.LinearColor(0.6, 0.6, 0.95, 1.0))
    hall_size = unreal.Vector(400.0, 600.0, 340.0)
    set_room_bounds(hall, hall_size)
    disable_parametric_base(hall)
    enable_stock_graybox(hall)
    set_stock_materials(hall, floor_material, wall_material, ceiling_material)
    add_cardinal_connectors(hall, hall_size, include_east=False, include_west=False, include_north=True, include_south=True)
    compile_and_save(hall)

    # Public Hallway Corner Cell (South + East, rotates to other quadrants)
    corner = create_blueprint("BP_Room_PublicHall_Corner")
    set_room_defaults(corner, "PublicHallCorner", "PublicHallCorner", 1.2, 2, 2, True, unreal.LinearColor(0.7, 0.7, 1.0, 1.0))
    corner_size = unreal.Vector(400.0, 400.0, 340.0)
    set_room_bounds(corner, corner_size)
    disable_parametric_base(corner)
    enable_stock_graybox(corner)
    set_stock_footprint(corner, unreal.RoomStockFootprintType.CORNER_SOUTH_EAST)
    set_stock_materials(corner, floor_material, wall_material, ceiling_material)
    add_cardinal_connectors(corner, corner_size, include_east=True, include_west=False, include_north=False, include_south=True)
    compile_and_save(corner)

    log("Room setup complete.")


if __name__ == "__main__":
    main()
