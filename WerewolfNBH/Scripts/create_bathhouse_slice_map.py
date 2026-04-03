import unreal


MAP_DIR = "/Game/WerewolfBH/Maps"
MAP_PATH = f"{MAP_DIR}/L_BathhouseSlice"
MANNY_ANIM_SOURCE = "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"
MANNY_ANIM_TARGET = "/Game/WerewolfBH/Blueprints/NPC/ABP_Manny_StagingNPC"
STAGING_NPC_BP_PATH = "/Game/WerewolfBH/Blueprints/NPC/BP_StagingNPC"
ROOM_GENERATOR_BP_PATH = "/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator"
LAYOUT_PROFILE_PATH = "/Game/WerewolfBH/Data/Ginny/Layouts/DA_GinnyLayout_Bathhouse_Default"
NPC_PROFILE_PATH = "/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_FirstTimer"
FIRST_PERSON_GAMEMODE_PATH = "/Game/FirstPerson/Blueprints/BP_FirstPersonGameMode"
PLANE_MESH_PATH = "/Engine/BasicShapes/Plane"
DIRECTIONAL_LIGHT_INTENSITY = 75000.0
SKY_LIGHT_INTENSITY = 1.5


def log(message: str) -> None:
    unreal.log(f"[create_bathhouse_slice_map] {message}")


def ensure_directory(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def load_asset(path: str):
    asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def ensure_duplicate_asset(source_path: str, target_path: str):
    if unreal.EditorAssetLibrary.does_asset_exist(target_path):
        return unreal.load_asset(target_path)

    target_dir, _ = target_path.rsplit("/", 1)
    ensure_directory(target_dir)
    if not unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path):
        raise RuntimeError(f"Failed to duplicate {source_path} -> {target_path}")
    return unreal.load_asset(target_path)


def compile_and_save_blueprint(blueprint) -> None:
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)


def configure_staging_npc(anim_blueprint) -> None:
    staging_npc_bp = load_asset(STAGING_NPC_BP_PATH)
    generated_class = staging_npc_bp.generated_class()
    if not generated_class:
        raise RuntimeError(f"{STAGING_NPC_BP_PATH} has no generated class")

    cdo = unreal.get_default_object(generated_class)
    cdo.set_editor_property("DefaultAnimationBlueprint", anim_blueprint.generated_class())
    compile_and_save_blueprint(staging_npc_bp)
    log("Configured BP_StagingNPC to use ABP_Manny_StagingNPC.")


def ensure_map():
    ensure_directory(MAP_DIR)
    created_new_map = False
    if not unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        if not level_subsystem.new_level(MAP_PATH):
            raise RuntimeError(f"Failed to create map: {MAP_PATH}")
        created_new_map = True

    world = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    if not world:
        raise RuntimeError(f"Failed to load map: {MAP_PATH}")

    return world, created_new_map


def get_all_level_actors():
    return unreal.EditorActorSubsystem().get_all_level_actors()


def find_actor_by_label(label: str):
    for actor in get_all_level_actors():
        if actor.get_actor_label() == label:
            return actor
    return None


def ensure_actor(label: str, actor_class, location: unreal.Vector, rotation: unreal.Rotator):
    actor = find_actor_by_label(label)
    if actor:
        return actor

    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class, location, rotation)
    if not actor:
        raise RuntimeError(f"Failed to spawn actor {label}")
    actor.set_actor_label(label)
    return actor


def set_component_mobility(component, mobility) -> None:
    if hasattr(component, "set_mobility"):
        component.set_mobility(mobility)
        return
    try:
        component.set_editor_property("mobility", mobility)
    except Exception:
        pass


def configure_floor_actor():
    plane_mesh = load_asset(PLANE_MESH_PATH)
    floor_actor = ensure_actor(
        "BathhouseSlice_Floor",
        unreal.StaticMeshActor,
        unreal.Vector(0.0, 0.0, 0.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )

    floor_component = floor_actor.get_component_by_class(unreal.StaticMeshComponent)
    floor_component.set_static_mesh(plane_mesh)
    floor_component.set_editor_property("RelativeScale3D", unreal.Vector(140.0, 140.0, 1.0))
    floor_component.set_editor_property("CastShadow", False)
    if hasattr(floor_component, "set_collision_profile_name"):
        floor_component.set_collision_profile_name("BlockAll")


def configure_environment():
    directional_light = ensure_actor(
        "BathhouseSlice_DirectionalLight",
        unreal.DirectionalLight,
        unreal.Vector(0.0, 0.0, 500.0),
        unreal.Rotator(-42.0, 35.0, 0.0),
    )
    sky_light = ensure_actor(
        "BathhouseSlice_SkyLight",
        unreal.SkyLight,
        unreal.Vector(0.0, 0.0, 0.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    ensure_actor(
        "BathhouseSlice_SkyAtmosphere",
        unreal.SkyAtmosphere,
        unreal.Vector(0.0, 0.0, 0.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    ensure_actor(
        "BathhouseSlice_Fog",
        unreal.ExponentialHeightFog,
        unreal.Vector(0.0, 0.0, 0.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )

    directional_component = directional_light.get_component_by_class(unreal.DirectionalLightComponent)
    if directional_component:
        set_component_mobility(directional_component, unreal.ComponentMobility.MOVABLE)
        directional_component.set_intensity(DIRECTIONAL_LIGHT_INTENSITY)
        directional_component.set_indirect_lighting_intensity(1.5)
        directional_component.set_cast_shadows(True)
        directional_component.set_editor_property("atmosphere_sun_light", True)

    sky_component = sky_light.get_component_by_class(unreal.SkyLightComponent)
    if sky_component:
        set_component_mobility(sky_component, unreal.ComponentMobility.MOVABLE)
        sky_component.set_intensity(SKY_LIGHT_INTENSITY)
        sky_component.set_real_time_capture(True)
        sky_component.set_cast_shadows(True)
        sky_component.recapture_sky()

    configure_floor_actor()


def configure_navmesh():
    nav = ensure_actor(
        "BathhouseSlice_NavMesh",
        unreal.NavMeshBoundsVolume,
        unreal.Vector(0.0, 0.0, 220.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    brush_component = nav.get_editor_property("brush_component")
    if brush_component:
        brush_component.set_editor_property("RelativeScale3D", unreal.Vector(26.0, 26.0, 6.0))


def configure_player_start():
    ensure_actor(
        "BathhouseSlice_PlayerStart",
        unreal.PlayerStart,
        unreal.Vector(-1400.0, -800.0, 120.0),
        unreal.Rotator(0.0, 30.0, 0.0),
    )


def configure_world_settings(world):
    game_mode_bp = load_asset(FIRST_PERSON_GAMEMODE_PATH)
    world_settings = world.get_world_settings()
    try:
        world_settings.set_editor_property("default_game_mode", game_mode_bp.generated_class())
        log("Applied BP_FirstPersonGameMode as the map override.")
    except Exception as exc:
        log(f"Skipped map game mode override: {exc}")


def configure_room_generator():
    generator_bp = load_asset(ROOM_GENERATOR_BP_PATH)
    layout_profile = load_asset(LAYOUT_PROFILE_PATH)
    staging_npc_bp = load_asset(STAGING_NPC_BP_PATH)
    npc_profile = load_asset(NPC_PROFILE_PATH)

    generator = ensure_actor(
        "BathhouseSlice_RoomGenerator",
        generator_bp.generated_class(),
        unreal.Vector(0.0, 0.0, 10.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )

    generator.set_editor_property("LayoutProfile", layout_profile)
    generator.set_editor_property("RunSeed", 1337)
    generator.set_editor_property("bGenerateOnBeginPlay", True)
    generator.set_editor_property("bUseNewSeedOnGenerate", False)
    generator.set_editor_property("bPrintDebugMessages", True)
    generator.set_editor_property("bDebugDrawBounds", False)
    generator.set_editor_property("bDebugDrawDoors", False)
    generator.set_editor_property("bRunButchAfterGeneration", False)
    generator.set_editor_property("bSpawnButchIfMissing", False)
    generator.set_editor_property("bAutoSpawnStagingDemoCoordinator", True)
    generator.set_editor_property("StagingDemoNPCClass", staging_npc_bp.generated_class())
    generator.set_editor_property("StagingDemoProfile", npc_profile)
    generator.set_editor_property("StagingDemoPhase", unreal.StagingRunPhase.OPENING_HOURS)
    generator.set_editor_property("bTreatStagingDemoAsWerewolf", False)
    generator.set_editor_property("StagingDemoSeedOffset", 101)

    generator.set_editor_property("bLimitStagingDemoToBathhouseSliceMap", True)
    for property_name in ("bAutoSpawnGideonDirector",):
        try:
            generator.set_editor_property(property_name, False)
        except Exception:
            pass
    generator.set_editor_property("bLimitGideonToBathhouseSliceMap", True)

    log("Configured BathhouseSlice room generator actor.")


def main():
    anim_blueprint = ensure_duplicate_asset(MANNY_ANIM_SOURCE, MANNY_ANIM_TARGET)
    configure_staging_npc(anim_blueprint)

    world, created_new_map = ensure_map()
    configure_world_settings(world)
    configure_environment()
    configure_navmesh()
    configure_player_start()
    configure_room_generator()

    unreal.EditorLevelLibrary.save_current_level()
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

    if created_new_map:
        log(f"Created canonical slice map at {MAP_PATH}.")
    else:
        log(f"Updated canonical slice map at {MAP_PATH}.")


if __name__ == "__main__":
    main()
