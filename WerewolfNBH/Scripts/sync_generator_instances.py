import unreal

MAP_PATH = '/Game/WerewolfBH/GeneratorTest'
GENERATOR_BP_PATH = '/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator'

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
if not world:
    raise RuntimeError(f'Failed to load {MAP_PATH}')

generator_bp = unreal.load_asset(GENERATOR_BP_PATH)
if not generator_bp:
    raise RuntimeError(f'Failed to load {GENERATOR_BP_PATH}')

generator_class = generator_bp.generated_class()
generator_cdo = unreal.get_default_object(generator_class)
butch_class = getattr(unreal, 'ButchDecorator', None) or unreal.load_class(None, '/Script/WerewolfNBH.ButchDecorator')
if not butch_class:
    raise RuntimeError('Failed to resolve native ButchDecorator class')
butch_cdo = unreal.get_default_object(butch_class)
actor_class = getattr(unreal, 'RoomGenerator', None) or unreal.load_class(None, '/Script/WerewolfNBH.RoomGenerator')
actors = unreal.GameplayStatics.get_all_actors_of_class(world, actor_class)
unreal.log_warning(f'Found {len(actors)} generator actor(s) in {MAP_PATH}')

props = [
    'StartRoomClass',
    'DeadEndRoomClass',
    'AvailableRooms',
    'RoomClassPool',
    'ConnectorFallbackRooms',
    'RunSeed',
    'MaxRooms',
    'AttemptsPerDoor',
    'VerticalSnapSize',
    'bAllowVerticalTransitions',
    'MaxVerticalDisplacement',
    'bUseNewSeedOnGenerate',
    'bDebugDrawBounds',
    'bDebugDrawDoors',
    'bPrintDebugMessages',
    'bGenerateOnBeginPlay',
    'bOverrideRoomSliceDebug',
    'bGlobalSliceDebugEnabled',
    'GlobalSliceDebugDuration',
    'bEnableHallwayChains',
    'MaxHallwayChainSegments',
    'bRunButchAfterGeneration',
    'bSpawnButchIfMissing',
    'ButchDecoratorClass',
]

for actor in actors:
    for prop in props:
        try:
            actor.set_editor_property(prop, generator_cdo.get_editor_property(prop))
        except Exception as exc:
            unreal.log_warning(f'Skipped {actor.get_name()}.{prop}: {exc}')
    actor.modify()
    unreal.log_warning(f'Synced generator actor: {actor.get_name()}')

butch_actors = unreal.GameplayStatics.get_all_actors_of_class(world, butch_class)
if not butch_actors:
    spawned_butch = unreal.EditorLevelLibrary.spawn_actor_from_class(butch_class, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
    butch_actors = [spawned_butch] if spawned_butch else []
    unreal.log_warning(f'Spawned Butch actor(s): {len(butch_actors)}')
else:
    unreal.log_warning(f'Found {len(butch_actors)} Butch actor(s) in {MAP_PATH}')

butch_props = [
    'bDecorateOnBeginPlay',
    'bSpawnPlaceholderMarkers',
    'bDebugDrawMarkers',
    'DebugDrawDuration',
    'MarkerScaleMultiplier',
]

for actor in butch_actors:
    for prop in butch_props:
        try:
            actor.set_editor_property(prop, butch_cdo.get_editor_property(prop))
        except Exception as exc:
            unreal.log_warning(f'Skipped {actor.get_name()}.{prop}: {exc}')
    actor.modify()
    unreal.log_warning(f'Synced Butch actor: {actor.get_name()}')

unreal.EditorLevelLibrary.save_current_level()
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log_warning('Generator instance sync complete.')
