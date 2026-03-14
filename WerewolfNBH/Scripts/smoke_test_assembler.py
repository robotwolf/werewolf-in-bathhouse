import unreal


MAP_PATH = "/Game/WerewolfBH/GeneratorTest"
SEED_PRIMARY = 1337
SEED_SECONDARY = 1338


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

    signature = []
    room_set = set(spawned_rooms)
    adjacency = {}

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

        neighbors = set()
        for record in room.get_editor_property("ConnectedRooms"):
            other = record.get_editor_property("OtherRoom")
            if other and other in room_set:
                neighbors.add(other)
        adjacency[room] = neighbors

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

    signature.sort()
    log(f"Seed {seed} spawned {len(spawned_rooms)} reachable rooms.")
    return tuple(signature), len(spawned_rooms)


def main():
    world = load_test_world()
    generator = get_generator(world)

    sig_a, count_a = build_layout_signature(generator, SEED_PRIMARY)
    sig_b, count_b = build_layout_signature(generator, SEED_PRIMARY)

    if sig_a != sig_b or count_a != count_b:
        fail("Determinism failed: same seed produced different signatures")

    sig_c, _ = build_layout_signature(generator, SEED_SECONDARY)
    if sig_c == sig_a:
        log("Warning: alternate seed produced identical signature (possible but uncommon)")

    log("Smoke test passed: determinism and reachability checks are healthy.")


if __name__ == "__main__":
    main()
