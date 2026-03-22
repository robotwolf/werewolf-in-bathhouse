import unreal


MAP_PATH = "/Game/WerewolfBH/GeneratorTest"
SEEDS = (1337, 1338, 1351)
PROTOTYPE_ROOM_PREFIX = "/Script/WerewolfNBH.Prototype"
STAIR_CLASS_FRAGMENT = "BP_Room_PublicHall_Stair_Up"
SMOKING_PATIO_CLASS_FRAGMENT = "BP_Room_SmokingPatioPocket"
BUTCH_CLASS_PATH = "/Game/WerewolfBH/Blueprints/Assembler/BP_ButchDecorator.BP_ButchDecorator_C"
STAIR_TRANSITION_TARGET = "SecondFloor_PrivateCubicles"
SMOKING_PATIO_PROFILE_PATH = "/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_SmokingPatioPocket"
NPC_PROFILE_PATHS = (
    "/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_Ronin",
    "/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_FirstTimer",
    "/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_FitnessObsessive",
    "/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_FloorManager",
    "/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_OccultScholar",
)


def get_marker_count_for_family(room, family):
    if family == unreal.RoomGameplayMarkerFamily.NPC:
        return len(room.get_npc_markers())
    if family == unreal.RoomGameplayMarkerFamily.TASK:
        return len(room.get_task_markers())
    if family == unreal.RoomGameplayMarkerFamily.CLUE:
        return len(room.get_clue_markers())
    if family == unreal.RoomGameplayMarkerFamily.MISSION_SOCKET:
        return len(room.get_mission_markers())
    if family == unreal.RoomGameplayMarkerFamily.FX:
        return len(room.get_fx_markers())
    return 0


def validate_marker_picker(room, family, seed):
    empty_tags = unreal.GameplayTagContainer()
    actual_count = get_marker_count_for_family(room, family)
    if actual_count <= 0:
        return

    marker = unreal.RoomGameplayMarkerLibrary.pick_gameplay_marker_from_room(
        room,
        family,
        seed,
        empty_tags,
        empty_tags,
        True,
    )
    marker_name = marker.get_editor_property("MarkerName")
    if not marker_name:
        fail(f"Marker picker returned empty marker for {room.get_name()} family {family.name}")


def validate_cross_room_marker_picker(rooms, family, seed):
    empty_tags = unreal.GameplayTagContainer()
    candidate_rooms = unreal.RoomGameplayMarkerLibrary.get_candidate_rooms_for_gameplay_markers(
        rooms,
        family,
        empty_tags,
        empty_tags,
        empty_tags,
        empty_tags,
        True,
        True,
    )
    if not candidate_rooms:
        return

    selection = unreal.RoomGameplayMarkerLibrary.pick_gameplay_marker_across_rooms(
        rooms,
        family,
        seed,
        empty_tags,
        empty_tags,
        empty_tags,
        empty_tags,
        empty_tags,
        empty_tags,
        True,
        True,
        True,
    )
    if isinstance(selection, (tuple, list)):
        selected_room, selected_marker = selection
    else:
        fail(f"Cross-room marker picker returned unexpected value type for family {family.name}: {type(selection)}")

    if not selected_room:
        fail(f"Cross-room marker picker returned no room for family {family.name}")

    if selected_room not in candidate_rooms:
        fail(f"Cross-room marker picker chose room outside candidate set for family {family.name}")

    marker_name = selected_marker.get_editor_property("MarkerName")
    if not marker_name:
        fail(f"Cross-room marker picker returned empty marker for family {family.name}")


def validate_scored_cross_room_marker_picker(rooms, family, seed):
    empty_tags = unreal.GameplayTagContainer()
    selection = unreal.RoomGameplayMarkerLibrary.pick_best_gameplay_marker_across_rooms(
        rooms,
        family,
        seed,
        empty_tags,
        empty_tags,
        empty_tags,
        empty_tags,
        empty_tags,
        empty_tags,
        empty_tags,
        empty_tags,
        empty_tags,
        3.0,
        2.0,
        1.0,
        True,
        True,
        True,
    )

    if not isinstance(selection, (tuple, list)) or len(selection) != 3:
        fail(f"Scored cross-room marker picker returned unexpected value type for family {family.name}: {type(selection)}")

    selected_room, selected_marker, selected_score = selection
    if not selected_room:
        fail(f"Scored cross-room marker picker returned no room for family {family.name}")

    marker_name = selected_marker.get_editor_property("MarkerName")
    if not marker_name:
        fail(f"Scored cross-room marker picker returned empty marker for family {family.name}")

    if selected_score < 0.0:
        fail(f"Scored cross-room marker picker returned negative score for family {family.name}")


def load_npc_profiles():
    profiles = []
    for path in NPC_PROFILE_PATHS:
        profile = unreal.load_asset(path)
        if not profile:
            fail(f"Missing authored NPC profile asset: {path}")
        profiles.append(profile)
    return profiles


def validate_npc_profile_marker_consumer(rooms, generator, seed):
    if not rooms:
        return

    probe_class = getattr(unreal, "StagehandNPCMarkerProbe", None)
    if not probe_class:
        fail("Could not resolve StagehandNPCMarkerProbe class")

    profiles = load_npc_profiles()
    for index, npc_profile in enumerate(profiles):
        activities = unreal.StagehandSimulationLibrary.get_applicable_activities_for_phase(
            npc_profile,
            unreal.StagehandRunPhase.OPENING_HOURS,
            False,
        )
        if not activities:
            fail(f"StagehandSimulationLibrary returned no applicable activities for authored NPC {npc_profile.get_name()}")

        selection = unreal.StagehandSimulationLibrary.pick_marker_for_npc_profile(
            npc_profile,
            rooms,
            unreal.StagehandRunPhase.OPENING_HOURS,
            False,
            seed + index,
        )

        if not selection.get_editor_property("bFoundSelection"):
            fail(f"StagehandSimulationLibrary failed to find an NPC marker for authored NPC {npc_profile.get_name()}")

        selected_room = selection.get_editor_property("Room")
        if not selected_room:
            fail(f"StagehandSimulationLibrary returned no room for authored NPC {npc_profile.get_name()}")

        marker = selection.get_editor_property("Marker")
        marker_name = marker.get_editor_property("MarkerName")
        if not marker_name:
            fail(f"StagehandSimulationLibrary returned an empty marker for authored NPC {npc_profile.get_name()}")

        probe = unreal.EditorLevelLibrary.spawn_actor_from_class(
            probe_class,
            unreal.Vector(0.0, 0.0, 0.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        if not probe:
            fail(f"Failed to spawn StagehandNPCMarkerProbe for authored NPC {npc_profile.get_name()}")

        probe.set_editor_property("TargetGenerator", generator)
        probe.set_editor_property("NPCProfile", npc_profile)
        probe.set_editor_property("Phase", unreal.StagehandRunPhase.OPENING_HOURS)
        probe.set_editor_property("SelectionSeed", seed + index)
        probe.set_editor_property("bDrawDebugMarker", False)

        if not probe.refresh_probe():
            fail(f"StagehandNPCMarkerProbe failed to refresh for authored NPC {npc_profile.get_name()}")

        probe_selection = probe.get_editor_property("Selection")
        if not probe_selection.get_editor_property("bFoundSelection"):
            fail(f"StagehandNPCMarkerProbe did not record a selection for authored NPC {npc_profile.get_name()}")

        probe.destroy_actor()


def log(message: str) -> None:
    unreal.log(f"[smoke_test_assembler] {message}")


def fail(message: str) -> None:
    raise RuntimeError(f"[smoke_test_assembler] {message}")


def load_test_world():
    world = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    if not world:
        fail(f"Unable to load map: {MAP_PATH}")
    return world


def resolve_generator_class():
    generator_class = getattr(unreal, "RoomGenerator", None)
    if generator_class:
        return generator_class

    generator_class = unreal.load_class(None, "/Script/WerewolfNBH.RoomGenerator")
    if not generator_class:
        fail("Could not resolve RoomGenerator class (/Script/WerewolfNBH.RoomGenerator)")
    return generator_class


def get_generator(world):
    generator_class = resolve_generator_class()
    generators = unreal.GameplayStatics.get_all_actors_of_class(world, generator_class)
    if generators:
        return generators[0]

    location = unreal.Vector(0.0, 0.0, 0.0)
    rotation = unreal.Rotator(0.0, 0.0, 0.0)
    spawned = unreal.EditorLevelLibrary.spawn_actor_from_class(generator_class, location, rotation)
    if not spawned:
        fail("No RoomGenerator actor found in GeneratorTest map, and temporary spawn failed")

    log("Spawned temporary RoomGenerator for smoke test because GeneratorTest map did not contain one.")
    return spawned


def build_layout_signature(generator, seed: int):
    generator.set_editor_property("bGenerateOnBeginPlay", False)
    generator.set_editor_property("bUseNewSeedOnGenerate", False)
    generator.set_editor_property("RunSeed", seed)

    generator.clear_generated_layout()
    generator.generate_layout()

    spawned_rooms = [room for room in generator.get_editor_property("SpawnedRooms") if room]
    if not spawned_rooms:
        fail(f"No rooms spawned for seed {seed}")

    max_rooms = generator.get_editor_property("MaxRooms")
    if len(spawned_rooms) > max_rooms:
        fail(f"Spawned {len(spawned_rooms)} rooms, but MaxRooms is {max_rooms}")

    if not generator.run_layout_validation(True):
        issues = generator.get_editor_property("LastValidationIssues")
        fail(f"Layout validation failed for seed {seed}: {issues}")

    if not generator.get_editor_property("LayoutProfile"):
        fail("Generator is missing LayoutProfile in the profile-backed baseline")

    signature = []
    room_set = set(spawned_rooms)
    adjacency = {}
    ordered_main_path = [room for room in generator.get_editor_property("GeneratedMainPathRooms") if room]

    for room in spawned_rooms:
        location = room.get_actor_location()
        rotation = room.get_actor_rotation()
        room_class = room.get_class().get_path_name()
        sig_entry = (
            room_class,
            round(location.x, 2),
            round(location.y, 2),
            round(location.z, 2),
            round(rotation.yaw, 2),
        )
        signature.append(sig_entry)

        room_class_name = room.get_class().get_path_name()
        if PROTOTYPE_ROOM_PREFIX in room_class_name:
            fail(f"Prototype room leaked into default config for seed {seed}: {room_class_name}")
        if not room.get_editor_property("RoomProfile"):
            fail(f"Spawned room is missing RoomProfile for seed {seed}: {room.get_name()}")

        neighbors = set()
        for record in room.get_editor_property("ConnectedRooms"):
            other = record.get_editor_property("OtherRoom")
            if other and other in room_set:
                neighbors.add(other)
        adjacency[room] = neighbors

        min_connections = room.get_editor_property("MinConnections")
        max_connections = room.get_editor_property("MaxConnections")
        connection_count = len(neighbors)
        if connection_count < min_connections or connection_count > max_connections:
            fail(f"Room {room.get_name()} violates connection budget: {connection_count} not in [{min_connections}, {max_connections}]")

        room_profile = room.get_editor_property("RoomProfile")
        marker_requirements = list(room_profile.get_editor_property("GameplayMarkerRequirements")) if room_profile else []
        for requirement in marker_requirements:
            family = requirement.get_editor_property("MarkerFamily")
            min_count = requirement.get_editor_property("MinCount")
            max_count = requirement.get_editor_property("MaxCount")
            actual_count = get_marker_count_for_family(room, family)
            if actual_count < min_count:
                fail(
                    f"Room {room.get_name()} is missing required gameplay markers for {family.name}: "
                    f"{actual_count} < {min_count}"
                )
            if max_count >= 0 and actual_count > max_count:
                fail(
                    f"Room {room.get_name()} exceeds gameplay marker budget for {family.name}: "
                    f"{actual_count} > {max_count}"
                )
            validate_marker_picker(room, family, seed)

    for family in (
        unreal.RoomGameplayMarkerFamily.NPC,
        unreal.RoomGameplayMarkerFamily.TASK,
        unreal.RoomGameplayMarkerFamily.CLUE,
        unreal.RoomGameplayMarkerFamily.MISSION_SOCKET,
        unreal.RoomGameplayMarkerFamily.FX,
    ):
        validate_cross_room_marker_picker(spawned_rooms, family, seed)
        validate_scored_cross_room_marker_picker(spawned_rooms, family, seed)

    validate_npc_profile_marker_consumer(spawned_rooms, generator, seed)

    visited = set()
    stack = [spawned_rooms[0]]
    while stack:
        node = stack.pop()
        if node in visited:
            continue
        visited.add(node)
        stack.extend(adjacency.get(node, []))

    if len(visited) != len(spawned_rooms):
        fail(f"Reachability failed for seed {seed}: visited {len(visited)} / {len(spawned_rooms)} rooms")

    if len(ordered_main_path) < 3:
        fail(f"Main spine too short for seed {seed}: {len(ordered_main_path)}")

    entry_room = ordered_main_path[0]
    if entry_room.get_editor_property("RoomType") != "EntryReception":
        fail(f"First room is not EntryReception for seed {seed}")

    if len(ordered_main_path) < 2 or ordered_main_path[1].get_editor_property("RoomType") != "PublicHallStraight":
        fail(f"First room after EntryReception is not PublicHallStraight for seed {seed}")

    for room, neighbors in adjacency.items():
        if room.get_editor_property("RoomType") != "LockerHall":
            continue
        for neighbor in neighbors:
            if neighbor.get_editor_property("RoomType") == "EntryReception":
                fail(f"LockerHall directly adjacent to EntryReception for seed {seed}")

    fallback_classes = generator.get_editor_property("ConnectorFallbackRooms")
    fallback_paths = {cls.get_path_name() for cls in fallback_classes if cls}
    if not fallback_paths or any(("PublicHall_Straight" not in path and "PublicHall_Corner" not in path and "PublicHall_LTurn" not in path) for path in fallback_paths):
        fail(f"ConnectorFallbackRooms contains non-hallway classes: {sorted(fallback_paths)}")

    if not generator.get_editor_property("bUseIntentionalHallApproaches"):
        fail("Intentional hall approaches are not enabled on the generator baseline")

    preset_name = str(generator.get_editor_property("HallwayApproachPreset"))
    if "LIMINAL_BRANCH" not in preset_name and "CUSTOM" not in preset_name:
        fail(f"Unexpected hallway approach preset on generator baseline: {preset_name}")

    min_approach_segments = generator.get_editor_property("MinHallwayApproachSegments")
    max_approach_segments = generator.get_editor_property("MaxHallwayApproachSegments")
    if min_approach_segments < 1 or max_approach_segments < min_approach_segments:
        fail(
            "Invalid intentional hallway approach settings: "
            f"min={min_approach_segments} max={max_approach_segments}"
        )

    available_paths = {cls.get_path_name() for cls in generator.get_editor_property("AvailableRooms") if cls}
    if not any(STAIR_CLASS_FRAGMENT in path for path in available_paths):
        fail("Stair room is not present in AvailableRooms despite being part of the optional branch policy")
    if not any(SMOKING_PATIO_CLASS_FRAGMENT in path for path in available_paths):
        fail("Contained exterior room is not present in AvailableRooms despite being part of the optional branch policy")

    pool_paths = {
        room_class.get_path_name()
        for entry in generator.get_editor_property("RoomClassPool")
        for room_class in [entry.get_editor_property("RoomClass")]
        if room_class
    }
    if not any(SMOKING_PATIO_CLASS_FRAGMENT in path for path in pool_paths):
        fail("Contained exterior room is not present in RoomClassPool despite being part of the optional branch policy")

    smoking_patio_profile = unreal.load_asset(SMOKING_PATIO_PROFILE_PATH)
    if not smoking_patio_profile:
        fail(f"Missing contained exterior room profile asset: {SMOKING_PATIO_PROFILE_PATH}")
    if not smoking_patio_profile.get_editor_property("bOverrideHallwayApproachPolicy"):
        fail("Contained exterior room profile is missing hallway-override policy")
    if smoking_patio_profile.get_editor_property("MinRequiredApproachSegments") < 2:
        fail("Contained exterior room profile does not require a long enough approach")
    if smoking_patio_profile.get_editor_property("RequiredMinimumCornerLikeSegments") < 1:
        fail("Contained exterior room profile does not require a corner-like approach segment")
    if not smoking_patio_profile.get_editor_property("bRequireApproachBeforePlacement"):
        fail("Contained exterior room profile should require approach-before-placement")
    if not smoking_patio_profile.get_editor_property("bRequireOverrideSatisfaction"):
        fail("Contained exterior room profile should require override satisfaction")

    butch_class = unreal.load_class(None, BUTCH_CLASS_PATH)
    if butch_class:
        butch_actors = unreal.GameplayStatics.get_all_actors_of_class(generator.get_world(), butch_class)
        if butch_actors:
            fail(f"Butch should be frozen in healthy default config, but found {len(butch_actors)} actor(s)")

    stair_rooms = [room for room in spawned_rooms if STAIR_CLASS_FRAGMENT in room.get_class().get_path_name()]
    if len(stair_rooms) > 1:
        fail(f"Expected at most one stair room, found {len(stair_rooms)} for seed {seed}")
    smoking_patio_rooms = [room for room in spawned_rooms if SMOKING_PATIO_CLASS_FRAGMENT in room.get_class().get_path_name()]
    if len(smoking_patio_rooms) > 1:
        fail(f"Expected at most one contained exterior room, found {len(smoking_patio_rooms)} for seed {seed}")

    main_path_set = set(ordered_main_path)
    for stair_room in stair_rooms:
        if stair_room in main_path_set:
            fail(f"Stair room should not appear on the main path for seed {seed}")
        if stair_room.get_editor_property("TransitionType") == unreal.RoomTransitionType.NONE:
            fail(f"Stair room is missing transition semantics for seed {seed}")
        if stair_room.get_editor_property("TransitionTargetConfigId") != STAIR_TRANSITION_TARGET:
            fail(f"Stair room transition target mismatch for seed {seed}")
        stair_profile = stair_room.get_editor_property("RoomProfile")
        if not stair_profile:
            fail(f"Stair room is missing its RoomProfile for seed {seed}")

    for smoking_patio_room in smoking_patio_rooms:
        if smoking_patio_room in main_path_set:
            fail(f"Contained exterior room should not appear on the main path for seed {seed}")
        smoking_profile = smoking_patio_room.get_editor_property("RoomProfile")
        if not smoking_profile:
            fail(f"Contained exterior room is missing its RoomProfile for seed {seed}")

    required_branch_types = {"Sauna", "BoilerService"}
    for required_type in required_branch_types:
        if not any(room.get_editor_property("RoomType") == required_type for room in spawned_rooms):
            fail(f"Missing required branch room {required_type} for seed {seed}")

    summary_lines = list(generator.get_editor_property("LastGenerationSummaryLines"))
    if not summary_lines:
        fail(f"Generation summary was empty for seed {seed}")
    if not any(f"Seed={seed}" in line for line in summary_lines):
        fail(f"Generation summary did not report seed {seed}")
    if not any("RequiredMain " in line for line in summary_lines):
        fail("Generation summary missing RequiredMain line")
    if not any("RequiredBranch " in line for line in summary_lines):
        fail("Generation summary missing RequiredBranch line")
    if not any("HallwayApproach Preset=" in line for line in summary_lines):
        fail("Generation summary missing hallway approach preset line")
    if not any("SpecialRoom " in line for line in summary_lines):
        fail("Generation summary missing special room reporting")
    if not any("Transitions=" in line for line in summary_lines):
        fail("Generation summary missing Transitions line")
    if stair_rooms and not any(STAIR_TRANSITION_TARGET in line for line in summary_lines):
        fail(f"Generation summary did not mention stair transition target {STAIR_TRANSITION_TARGET}")

    signature.sort()
    log(f"Seed {seed} spawned {len(spawned_rooms)} reachable rooms.")
    return tuple(signature), len(spawned_rooms)


def run_negative_validation_probe(generator):
    generator.clear_generated_layout()
    generator.set_editor_property("RunSeed", SEEDS[0])
    generator.generate_layout()
    rooms = [room for room in generator.get_editor_property("SpawnedRooms") if room]
    if not rooms:
        fail("Negative validation probe could not generate a layout")

    target_room = rooms[-1]
    room_profile = target_room.get_editor_property("RoomProfile")
    if not room_profile:
        fail("Negative validation probe target room is missing RoomProfile")

    original_neighbors = list(room_profile.get_editor_property("AllowedNeighborRoomTypes"))
    room_profile.set_editor_property("AllowedNeighborRoomTypes", ["DefinitelyNotARoomType"])
    valid = generator.run_layout_validation(False)
    room_profile.set_editor_property("AllowedNeighborRoomTypes", original_neighbors)
    if valid:
        fail("Negative validation probe expected failure, but validation passed")
    log("Negative validation probe behaved correctly.")


def main():
    world = load_test_world()
    generator = get_generator(world)

    sig_a, count_a = build_layout_signature(generator, SEEDS[0])
    sig_b, count_b = build_layout_signature(generator, SEEDS[0])

    if sig_a != sig_b or count_a != count_b:
        fail("Determinism failed: same seed produced different signatures")

    for seed in SEEDS[1:]:
        build_layout_signature(generator, seed)

    run_negative_validation_probe(generator)
    log("Smoke test passed: determinism, semantic sanity, and validation checks are healthy.")


if __name__ == "__main__":
    main()
