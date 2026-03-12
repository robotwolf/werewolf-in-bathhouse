import unreal


DEST_PATH = "/Game/WerewolfBH/Blueprints/Assembler"


def log(message: str) -> None:
    unreal.log(f"[assembler_blueprints] {message}")


def ensure_folder(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        log(f"Created folder: {path}")


def create_blueprint(asset_name: str, parent_class) -> None:
    asset_path = f"{DEST_PATH}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        log(f"Asset already exists: {asset_path}")
        return

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", parent_class)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    blueprint = asset_tools.create_asset(asset_name, DEST_PATH, None, factory)

    if not blueprint:
        raise RuntimeError(f"Failed to create blueprint: {asset_path}")

    unreal.EditorAssetLibrary.save_asset(asset_path)
    log(f"Created blueprint: {asset_path}")


def delete_asset_if_exists(asset_path: str) -> None:
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)
        log(f"Deleted asset: {asset_path}")


def main() -> None:
    ensure_folder(DEST_PATH)

    delete_asset_if_exists(f"{DEST_PATH}/BP_RoomGenerator_Preview")
    delete_asset_if_exists(f"{DEST_PATH}/BP_Room_Hub_Preview")
    delete_asset_if_exists(f"{DEST_PATH}/BP_Room_Corridor_Preview")
    delete_asset_if_exists(f"{DEST_PATH}/BP_Room_DeadEnd_Preview")

    create_blueprint("BP_RoomGenerator", unreal.RoomGenerator)
    create_blueprint("BP_Room_Hub", unreal.PrototypeRoomHub)
    create_blueprint("BP_Room_Corridor", unreal.PrototypeRoomCorridor)
    create_blueprint("BP_Room_DeadEnd", unreal.PrototypeRoomDeadEnd)

    unreal.EditorAssetLibrary.save_directory(DEST_PATH)
    log("Done.")


if __name__ == "__main__":
    main()
