import unreal


ROOM_BLUEPRINT_PATHS = {
    "EntryReception": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_EntryReception",
    "PublicHallStraight": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Straight",
    "LockerHall": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_LockerHall",
    "PoolHall": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PoolHall",
    "Sauna": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Sauna",
    "BoilerService": "/Game/WerewolfBH/Blueprints/Rooms/BP_Room_BoilerService",
}


MARKER_LAYOUTS = {
    "EntryReception": [
        ("NPC_Wait_A", unreal.Vector(-180.0, -220.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0), ["NPC.Activity.Wait", "Room.Function.Entry"]),
        ("NPC_Gossip_A", unreal.Vector(140.0, -40.0, 0.0), unreal.Rotator(0.0, 45.0, 0.0), ["NPC.Activity.Gossip", "Room.Function.Social"]),
        ("NPC_Gossip_B", unreal.Vector(240.0, 60.0, 0.0), unreal.Rotator(0.0, -135.0, 0.0), ["NPC.Activity.Gossip", "Room.Function.Social"]),
        ("Clue_ReceptionDesk_A", unreal.Vector(0.0, 40.0, 92.0), unreal.Rotator(0.0, 180.0, 0.0), ["Clue.Social", "Room.Function.Entry"]),
        ("MissionSocket_A", unreal.Vector(0.0, 240.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), ["Room.Function.Entry", "Room.Function.Social"]),
    ],
    "PublicHallStraight": [
        ("NPC_Observe_A", unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 90.0, 0.0), ["NPC.Activity.Observe", "Room.Function.Transition"]),
        ("Clue_Baseboard_A", unreal.Vector(-130.0, 40.0, 16.0), unreal.Rotator(0.0, 0.0, 0.0), ["Clue.Physical", "Room.Function.Transition"]),
        ("MissionSocket_A", unreal.Vector(0.0, 180.0, 0.0), unreal.Rotator(0.0, -90.0, 0.0), ["Room.Function.Transition"]),
    ],
    "LockerHall": [
        ("NPC_Gossip_A", unreal.Vector(-140.0, -120.0, 0.0), unreal.Rotator(0.0, 20.0, 0.0), ["NPC.Activity.Gossip", "Room.Function.Changing"]),
        ("NPC_Gossip_B", unreal.Vector(-20.0, -40.0, 0.0), unreal.Rotator(0.0, 200.0, 0.0), ["NPC.Activity.Gossip", "Room.Function.Changing"]),
        ("NPC_Observe_Door", unreal.Vector(0.0, 340.0, 0.0), unreal.Rotator(0.0, -90.0, 0.0), ["NPC.Activity.Observe", "Room.Function.Changing"]),
        ("Task_TowelRestock_A", unreal.Vector(420.0, -120.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), ["NPC.Activity.Clean", "Room.Function.Task"]),
        ("Clue_Locker_A", unreal.Vector(-430.0, 120.0, 72.0), unreal.Rotator(0.0, 0.0, 0.0), ["Clue.Physical", "Room.Function.Changing"]),
        ("MissionSocket_A", unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0), ["Room.Function.Social", "Room.Function.Changing"]),
    ],
    "PoolHall": [
        ("NPC_Relax_A", unreal.Vector(-620.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0), ["NPC.Activity.Relax", "Room.Function.Relaxation"]),
        ("NPC_Relax_B", unreal.Vector(620.0, 0.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), ["NPC.Activity.Relax", "Room.Function.Relaxation"]),
        ("Clue_Drain_A", unreal.Vector(0.0, -520.0, 12.0), unreal.Rotator(0.0, 0.0, 0.0), ["Clue.Physical", "Room.Function.Relaxation"]),
        ("MissionSocket_A", unreal.Vector(0.0, 600.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), ["Room.Function.Relaxation", "Room.Function.Social"]),
        ("FX_Steam_A", unreal.Vector(0.0, 0.0, 220.0), unreal.Rotator(0.0, 0.0, 0.0), ["Room.Env.Wet"]),
    ],
    "Sauna": [
        ("NPC_Relax_A", unreal.Vector(0.0, -120.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0), ["NPC.Activity.Relax", "Room.Env.Hot"]),
        ("Clue_Bench_A", unreal.Vector(180.0, 220.0, 54.0), unreal.Rotator(0.0, 180.0, 0.0), ["Clue.Physical", "Room.Env.Hot"]),
        ("MissionSocket_A", unreal.Vector(0.0, 180.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), ["Room.Function.Relaxation"]),
        ("FX_Steam_A", unreal.Vector(0.0, 0.0, 180.0), unreal.Rotator(0.0, 0.0, 0.0), ["Room.Env.SteamLow", "Room.Env.Hot"]),
    ],
    "BoilerService": [
        ("NPC_Observe_Panel", unreal.Vector(220.0, -180.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), ["NPC.Activity.Observe", "Room.Function.Maintenance"]),
        ("Task_BoilerPanel_A", unreal.Vector(220.0, 120.0, 0.0), unreal.Rotator(0.0, 180.0, 0.0), ["NPC.Activity.Clean", "Room.Function.Task", "Room.Function.Maintenance"]),
        ("Clue_Valve_A", unreal.Vector(-240.0, 180.0, 80.0), unreal.Rotator(0.0, 0.0, 0.0), ["Clue.Physical", "Room.Env.Mechanical"]),
        ("MissionSocket_A", unreal.Vector(0.0, -60.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0), ["Room.Function.Maintenance"]),
        ("FX_Steam_A", unreal.Vector(0.0, 0.0, 220.0), unreal.Rotator(0.0, 0.0, 0.0), ["Room.Env.Mechanical", "Room.Env.SteamLow"]),
    ],
}

MARKER_PREFIXES = ("NPC_", "Task_", "Clue_", "MissionSocket_", "FX_")


def log(message: str) -> None:
    unreal.log(f"[sync_room_gameplay_markers] {message}")


def load_blueprint(path: str):
    blueprint = unreal.load_asset(path)
    if not blueprint:
        raise RuntimeError(f"Missing blueprint: {path}")
    return blueprint


def gather_handles(blueprint):
    subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    return subsys, subsys.k2_gather_subobject_data_for_blueprint(blueprint)


def find_scene_component(blueprint, name: str):
    subsys, handles = gather_handles(blueprint)
    for handle in handles:
        data = subsys.k2_find_subobject_data_from_handle(handle)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        if not obj or not isinstance(obj, unreal.SceneComponent):
            continue
        obj_name = obj.get_name().replace("_GEN_VARIABLE", "")
        if obj_name == name:
            return obj, handle
    return None, None


def add_or_update_marker(blueprint, name: str, location: unreal.Vector, rotation: unreal.Rotator, tags):
    existing, _ = find_scene_component(blueprint, name)
    if existing:
        existing.set_editor_property("RelativeLocation", location)
        existing.set_editor_property("RelativeRotation", rotation)
        existing.set_editor_property("RelativeScale3D", unreal.Vector(1.0, 1.0, 1.0))
        existing.set_editor_property("ComponentTags", list(tags))
        existing.set_editor_property("Mobility", unreal.ComponentMobility.MOVABLE)
        return

    subsys, handles = gather_handles(blueprint)
    if not handles:
        raise RuntimeError(f"No subobject handles found for {blueprint.get_name()}")

    params = unreal.AddNewSubobjectParams()
    params.set_editor_property("parent_handle", handles[0])
    params.set_editor_property("new_class", unreal.SceneComponent)
    params.set_editor_property("blueprint_context", blueprint)
    params.set_editor_property("conform_transform_to_parent", True)

    new_handle, fail_reason = subsys.add_new_subobject(params)
    if not unreal.SubobjectDataBlueprintFunctionLibrary.is_handle_valid(new_handle):
        raise RuntimeError(f"Failed to add gameplay marker {name} to {blueprint.get_name()}: {fail_reason}")

    subsys.rename_subobject(new_handle, name)
    data = subsys.k2_find_subobject_data_from_handle(new_handle)
    component = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
    if not component:
        raise RuntimeError(f"Failed to resolve gameplay marker component for {name}")

    component.set_editor_property("RelativeLocation", location)
    component.set_editor_property("RelativeRotation", rotation)
    component.set_editor_property("RelativeScale3D", unreal.Vector(1.0, 1.0, 1.0))
    component.set_editor_property("ComponentTags", list(tags))
    component.set_editor_property("Mobility", unreal.ComponentMobility.MOVABLE)


def remove_unwanted_markers(blueprint, allowed_names):
    subsys, handles = gather_handles(blueprint)
    if not handles:
        return

    root_handle = handles[0]
    to_delete = []
    for handle in handles:
        data = subsys.k2_find_subobject_data_from_handle(handle)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        if not obj or not isinstance(obj, unreal.SceneComponent):
            continue

        base_name = obj.get_name().replace("_GEN_VARIABLE", "")
        if any(base_name.startswith(prefix) for prefix in MARKER_PREFIXES) and base_name not in allowed_names:
            to_delete.append(handle)

    if to_delete:
        subsys.delete_subobjects(root_handle, to_delete, blueprint)


def compile_and_save_blueprint(blueprint):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)


def sync_room_markers(room_key: str, blueprint_path: str):
    blueprint = load_blueprint(blueprint_path)
    marker_defs = MARKER_LAYOUTS.get(room_key, [])
    allowed_names = set()
    for name, location, rotation, tags in marker_defs:
        add_or_update_marker(blueprint, name, location, rotation, tags)
        allowed_names.add(name)

    remove_unwanted_markers(blueprint, allowed_names)
    compile_and_save_blueprint(blueprint)
    log(f"Synchronized {len(marker_defs)} gameplay markers for {room_key}.")


def main():
    for room_key, blueprint_path in ROOM_BLUEPRINT_PATHS.items():
        sync_room_markers(room_key, blueprint_path)

    log("Gameplay marker sync complete.")


if __name__ == "__main__":
    main()
