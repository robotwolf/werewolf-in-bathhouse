import unreal


ROOM_TYPES_PATH = "/Game/WerewolfBH/Blueprints/RoomTypes"


def log(message: str) -> None:
    unreal.log(f"[create_room_type_bases] {message}")


def ensure_folder(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        log(f"Created folder: {path}")


def get_cdo(blueprint):
    generated = blueprint.generated_class()
    if not generated:
        raise RuntimeError(f"Blueprint has no generated class: {blueprint.get_name()}")
    return unreal.get_default_object(generated)


def create_blueprint(asset_name: str):
    asset_path = f"{ROOM_TYPES_PATH}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return unreal.load_asset(asset_path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", unreal.RoomModuleBase)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    blueprint = asset_tools.create_asset(asset_name, ROOM_TYPES_PATH, None, factory)
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


def compile_and_save(blueprint):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    asset_path = unreal.EditorAssetLibrary.get_path_name_for_loaded_asset(blueprint)
    unreal.EditorAssetLibrary.save_asset(asset_path)


def main():
    ensure_folder(ROOM_TYPES_PATH)

    public_color = unreal.LinearColor(0.35, 0.7, 1.0, 1.0)
    staff_color = unreal.LinearColor(1.0, 0.6, 0.2, 1.0)
    outdoor_color = unreal.LinearColor(0.2, 0.9, 0.5, 1.0)
    supernatural_color = unreal.LinearColor(0.7, 0.3, 0.9, 1.0)

    room_specs = [
        ("EntryReception", "RM_EntryReception", 1, 3, True, public_color),
        ("FrontHall", "RM_FrontHall", 2, 3, True, public_color),
        ("LockerHall", "RM_LockerHall", 2, 4, True, public_color),
        ("CubicleRoom", "RM_CubicleRoom", 1, 3, True, public_color),
        ("BathroomVanity", "RM_BathroomVanity", 1, 3, True, public_color),
        ("PublicHallway", "RM_PublicHallway", 2, 4, True, public_color),
        ("SteamCave", "RM_SteamCave", 2, 3, True, public_color),
        ("DrySauna", "RM_DrySauna", 1, 2, True, public_color),
        ("ShowerRoom", "RM_ShowerRoom", 2, 4, True, public_color),
        ("PoolHall", "RM_PoolHall", 2, 4, True, public_color),
        ("ColdPlunge", "RM_ColdPlunge", 1, 2, False, supernatural_color),
        ("OutdoorSmokerYard", "RM_OutdoorSmokerYard", 1, 2, False, outdoor_color),
        ("Laundry", "RM_Laundry", 1, 3, True, staff_color),
        ("BoilerMaintenance", "RM_BoilerMaintenance", 1, 3, True, staff_color),
        ("StaffCorridor", "RM_StaffCorridor", 2, 4, True, staff_color),
        ("MaintenanceCloset", "RM_MaintenanceCloset", 1, 1, False, staff_color),
        ("LoungeJuiceBar", "RM_LoungeJuiceBar", 1, 3, False, public_color),
        ("HiddenAccessStub", "RM_HiddenAccessStub", 1, 2, False, supernatural_color),
    ]

    for room_type, room_id, min_conn, max_conn, required, color in room_specs:
        asset_name = f"BP_RoomType_{room_type}"
        blueprint = create_blueprint(asset_name)
        set_room_defaults(blueprint, room_id, room_type, 1.0, min_conn, max_conn, required, color)
        compile_and_save(blueprint)

    log("Room type bases created/updated.")


if __name__ == "__main__":
    main()

