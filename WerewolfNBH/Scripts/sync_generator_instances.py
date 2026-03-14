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
]

for actor in actors:
    for prop in props:
        try:
            actor.set_editor_property(prop, generator_cdo.get_editor_property(prop))
        except Exception as exc:
            unreal.log_warning(f'Skipped {actor.get_name()}.{prop}: {exc}')
    actor.modify()
    unreal.log_warning(f'Synced generator actor: {actor.get_name()}')

unreal.EditorLevelLibrary.save_current_level()
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log_warning('Generator instance sync complete.')
