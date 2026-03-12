import unreal


DEST_PATH = "/Game/WerewolfBH/Blueprints/Rooms"
ASSET_NAME = "BP_Room_EntryReception"


def log(message: str) -> None:
    unreal.log(f"[create_first_bathhouse_room] {message}")


def ensure_folder(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        log(f"Created folder: {path}")


def main() -> None:
    ensure_folder(DEST_PATH)

    asset_path = f"{DEST_PATH}/{ASSET_NAME}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        log(f"Asset already exists: {asset_path}")
        return

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", unreal.RoomModuleBase)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    blueprint = asset_tools.create_asset(ASSET_NAME, DEST_PATH, None, factory)

    if not blueprint:
        raise RuntimeError(f"Failed to create blueprint: {asset_path}")

    generated_class = blueprint.generated_class()
    cdo = unreal.get_default_object(generated_class)

    cdo.set_editor_property("RoomID", "EntryReception")
    cdo.set_editor_property("RoomType", "EntryReception")
    cdo.set_editor_property("Weight", 1.0)
    cdo.set_editor_property("MinConnections", 1)
    cdo.set_editor_property("MaxConnections", 3)
    cdo.set_editor_property("bRequired", True)
    cdo.set_editor_property("DebugColor", unreal.LinearColor(0.2, 0.9, 0.6, 1.0))

    # Set graybox bounds on the RoomBoundsBox component.
    room_bounds = cdo.get_editor_property("RoomBoundsBox")
    if room_bounds:
        room_bounds.set_editor_property("BoxExtent", unreal.Vector(600.0, 500.0, 190.0))
        room_bounds.set_editor_property("RelativeLocation", unreal.Vector(0.0, 0.0, 190.0))

    unreal.EditorAssetLibrary.save_asset(asset_path)
    log(f"Created bathhouse room blueprint: {asset_path}")


if __name__ == "__main__":
    main()
