import unreal

ROOM_PATH = "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Stair_Up"


def log(message: str) -> None:
    unreal.log(f"[update_public_stair_profile] {message}")


def load_bp(path: str):
    asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing blueprint: {path}")
    return asset


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
            obj_name = obj.get_name()
            if obj_name == name or obj_name == f"{name}_GEN_VARIABLE":
                return obj
    return None


def save_blueprint(blueprint):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    asset_path = unreal.EditorAssetLibrary.get_path_name_for_loaded_asset(blueprint)
    unreal.EditorAssetLibrary.save_asset(asset_path)


bp = load_bp(ROOM_PATH)
cdo = get_cdo(bp)

room_size = unreal.Vector(1200.0, 1800.0, 760.0)
box = cdo.get_editor_property("RoomBoundsBox")
box.set_editor_property("BoxExtent", room_size * 0.5)
box.set_editor_property("RelativeLocation", unreal.Vector(0.0, 0.0, room_size.z * 0.5))

settings = cdo.get_editor_property("StockAssemblySettings")
settings.set_editor_property("FootprintType", unreal.RoomStockFootprintType.STAIR_SOUTH_TO_NORTH_UP)
settings.set_editor_property("StairLayoutType", unreal.RoomStockStairLayoutType.STRAIGHT)
settings.set_editor_property("DoorWidthMode", unreal.RoomStockDoorWidthMode.DOUBLE_WIDE)
settings.set_editor_property("StairWalkWidth", 1200.0)
settings.set_editor_property("StairLowerLandingDepth", 360.0)
settings.set_editor_property("StairUpperLandingDepth", 360.0)
settings.set_editor_property("StairStepCount", 14)
settings.set_editor_property("StairRiseHeight", 420.0)
settings.set_editor_property("StairSideInset", 0.0)
settings.set_editor_property("bCreateStairLandingSideOpenings", True)
settings.set_editor_property("StairLandingSideOpeningWidth", 320.0)
settings.set_editor_property("StairLandingSideOpeningHeight", 280.0)
cdo.set_editor_property("StockAssemblySettings", settings)

cdo.set_editor_property("MinConnections", 1)
cdo.set_editor_property("MaxConnections", 1)
cdo.set_editor_property("bExpandGeneration", False)
cdo.set_editor_property("bShowRoomNameLabel", True)
cdo.set_editor_property("bBillboardRoomNameLabel", True)
cdo.set_editor_property("bShowConnectorDebugArrows", True)
cdo.set_editor_property("TransitionType", unreal.RoomTransitionType.CONFIG_HANDOFF)
cdo.set_editor_property("TransitionTargetConfigId", "SecondFloor_PrivateCubicles")

rules = cdo.get_editor_property("PlacementRules")
rules.set_editor_property("PlacementRole", unreal.RoomPlacementRole.BRANCH)
rules.set_editor_property("bAllowOnMainPath", False)
rules.set_editor_property("bAllowOnBranch", True)
rules.set_editor_property("bCanTerminatePath", True)
rules.set_editor_property("MinDepthFromStart", 4)
rules.set_editor_property("MaxDepthFromStart", 9)
rules.set_editor_property("MaxInstances", 1)
cdo.set_editor_property("PlacementRules", rules)

south = find_connector(bp, "Conn_S")
if south:
    south.set_editor_property("RelativeLocation", unreal.Vector(0.0, -room_size.y * 0.5, 150.0))
    south.set_editor_property("RelativeRotation", unreal.Rotator(pitch=0.0, yaw=-90.0, roll=0.0))

north = find_connector(bp, "Conn_N")
if north:
    north.set_editor_property("RelativeLocation", unreal.Vector(0.0, room_size.y * 0.5, 570.0))
    north.set_editor_property("RelativeRotation", unreal.Rotator(pitch=0.0, yaw=90.0, roll=0.0))

save_blueprint(bp)
log("Updated BP_Room_PublicHall_Stair_Up to the wide public stair profile.")
