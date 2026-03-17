import unreal

MAP_PATH = "/Game/WerewolfBH/Proofs/RV/Maps/Ginny_RV_Proof"
LANE_FRAGMENT = "BP_Room_RV_Lane"
RV_FRAGMENT = "BP_Room_RV_SingleWide"


def fail(message: str) -> None:
    raise RuntimeError(f"[smoke_test_rv_proof] {message}")


def log(message: str) -> None:
    unreal.log(f"[smoke_test_rv_proof] {message}")


def load_world():
    world = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    if not world:
        fail(f"Unable to load map: {MAP_PATH}")
    return world


def get_generator(world):
    generator_class = unreal.load_class(None, "/Script/WerewolfNBH.RoomGenerator")
    generators = unreal.GameplayStatics.get_all_actors_of_class(world, generator_class)
    if not generators:
        fail("No RoomGenerator actor found in RV proof map")
    return generators[0]


def main():
    world = load_world()
    generator = get_generator(world)
    generator.set_editor_property("bGenerateOnBeginPlay", False)
    generator.set_editor_property("bUseNewSeedOnGenerate", False)
    generator.set_editor_property("RunSeed", 2112)
    generator.clear_generated_layout()
    generator.generate_layout()

    rooms = [room for room in generator.get_editor_property("SpawnedRooms") if room]
    if len(rooms) < 3:
        fail(f"Expected at least 3 RV proof rooms, found {len(rooms)}")

    lane_rooms = [room for room in rooms if LANE_FRAGMENT in room.get_class().get_path_name()]
    rv_rooms = [room for room in rooms if RV_FRAGMENT in room.get_class().get_path_name()]
    if not lane_rooms:
        fail("No RV lane rooms spawned")
    if not rv_rooms:
        fail("No RV single-wide rooms spawned")

    for room in rv_rooms:
        profile = room.get_editor_property("RoomProfile")
        if not profile:
            fail(f"RV room {room.get_name()} is missing RoomProfile")
        stock = profile.get_editor_property("StockAssemblySettings")
        if not stock.get_editor_property("bOverrideConstructionTechnique"):
            fail("RV room profile is not overriding Mason construction technique")
        if stock.get_editor_property("ConstructionTechnique") != unreal.MasonConstructionTechnique.OBJECT_SHELL:
            fail("RV room profile is not using Mason ObjectShell")

    for room in lane_rooms:
        profile = room.get_editor_property("RoomProfile")
        if not profile:
            fail(f"Lane room {room.get_name()} is missing RoomProfile")
        stock = profile.get_editor_property("StockAssemblySettings")
        if stock.get_editor_property("ConstructionTechnique") != unreal.MasonConstructionTechnique.OPEN_LOT:
            fail("Lane room profile is not using Mason OpenLot")

    if not generator.get_editor_property("LayoutProfile"):
        fail("RV proof generator is missing LayoutProfile")

    summary = list(generator.get_editor_property("LastGenerationSummaryLines"))
    if not summary:
        fail("RV proof generation summary is empty")

    log(f"RV proof smoke passed with {len(lane_rooms)} lane room(s) and {len(rv_rooms)} RV room(s).")


if __name__ == "__main__":
    main()
