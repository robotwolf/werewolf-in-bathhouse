import unreal

MAP_PATH = '/Game/WerewolfBH/GeneratorTest'
GENERATOR_BP_PATH = '/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator'
BUTCH_BP_OBJECT_PATH = '/Game/WerewolfBH/Blueprints/Assembler/BP_ButchDecorator'
BUTCH_BP_PATH = f'{BUTCH_BP_OBJECT_PATH}.BP_ButchDecorator'

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
if not world:
    raise RuntimeError(f'Failed to load {MAP_PATH}')

generator_bp = unreal.load_asset(GENERATOR_BP_PATH)
if not generator_bp:
    raise RuntimeError(f'Failed to load {GENERATOR_BP_PATH}')

generator_class = generator_bp.generated_class()
generator_cdo = unreal.get_default_object(generator_class)
butch_bp = unreal.load_asset(BUTCH_BP_OBJECT_PATH)
if not butch_bp:
    if not unreal.EditorAssetLibrary.does_directory_exist('/Game/WerewolfBH/Blueprints/Assembler'):
        unreal.EditorAssetLibrary.make_directory('/Game/WerewolfBH/Blueprints/Assembler')
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('ParentClass', unreal.ButchDecorator)
    butch_bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset('BP_ButchDecorator', '/Game/WerewolfBH/Blueprints/Assembler', unreal.Blueprint, factory)
    if butch_bp:
        unreal.BlueprintEditorLibrary.compile_blueprint(butch_bp)
        unreal.EditorAssetLibrary.save_loaded_asset(butch_bp)
if not butch_bp:
    raise RuntimeError(f'Failed to load or create {BUTCH_BP_PATH}')
butch_class = butch_bp.generated_class()
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
    'RequiredMainPathRooms',
    'RequiredBranchRooms',
    'LayoutProfile',
    'RunSeed',
    'MaxRooms',
    'AttemptsPerDoor',
    'MaxLayoutAttempts',
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
for actor in butch_actors:
    actor.destroy_actor()
    unreal.log_warning(f'Removed Butch actor from healthy baseline: {actor.get_name()}')

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log_warning('Generator instance sync complete.')
