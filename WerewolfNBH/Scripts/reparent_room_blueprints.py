import unreal


def log(msg: str) -> None:
    unreal.log(f"[reparent_room_blueprints] {msg}")


ROOM_PARENT_MAP = {
    "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_EntryReception": "/Game/WerewolfBH/Blueprints/RoomTypes/BP_RoomType_EntryReception.BP_RoomType_EntryReception_C",
    "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_LockerHall": "/Game/WerewolfBH/Blueprints/RoomTypes/BP_RoomType_LockerHall.BP_RoomType_LockerHall_C",
    "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Straight": "/Game/WerewolfBH/Blueprints/RoomTypes/BP_RoomType_PublicHallway.BP_RoomType_PublicHallway_C",
}


def load_parent_class(parent_class_path: str):
    parent_class = unreal.load_class(None, parent_class_path)
    if parent_class:
        return parent_class

    parent_asset_path = parent_class_path.split(".")[0]
    parent_bp = unreal.load_asset(parent_asset_path)
    if parent_bp:
        generated = parent_bp.generated_class()
        if generated:
            return generated

    return None


def reparent_blueprint(bp_path: str, parent_class_path: str) -> None:
    bp = unreal.load_asset(bp_path)
    if not bp:
        raise RuntimeError(f"Could not load blueprint: {bp_path}")

    parent_class = load_parent_class(parent_class_path)
    if not parent_class:
        raise RuntimeError(f"Could not load parent class: {parent_class_path}")

    unreal.BlueprintEditorLibrary.reparent_blueprint(bp, parent_class)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(bp_path)
    log(f"Reparented {bp_path} -> {parent_class.get_name()}")


def main() -> None:
    for bp_path, parent_class_path in ROOM_PARENT_MAP.items():
        reparent_blueprint(bp_path, parent_class_path)

    log("Reparenting complete.")


if __name__ == "__main__":
    main()

