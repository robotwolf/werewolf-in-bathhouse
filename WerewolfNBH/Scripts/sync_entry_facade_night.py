import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(__file__)
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import setup_bathhouse_rooms as base


ROOM_ASSET_NAME = "BP_Room_EntryFacadeNight"
TRON_GLOW_PATH = "/Game/WerewolfBH/Materials/Tron_Glow"


def log(message: str) -> None:
    unreal.log(f"[sync_entry_facade_night] {message}")


def main():
    base.ensure_folder(base.ROOMS_PATH)

    entry_floor_material = base.load_required_asset(base.ENTRY_FLOOR_MATERIAL_PATH)
    entry_wall_material = base.load_required_asset(base.ENTRY_WALL_MATERIAL_PATH)
    ceiling_material = base.load_required_asset(base.CEILING_MATERIAL_PATH)
    hall_floor_material = base.load_required_asset(base.HALL_FLOOR_MATERIAL_PATH)
    service_material = base.load_required_asset(base.SERVICE_METAL_MATERIAL_PATH)
    sky_material = base.load_required_asset(TRON_GLOW_PATH)
    cube_mesh = base.load_required_asset(base.ENGINE_CUBE_PATH)
    cylinder_mesh = base.load_required_asset(base.ENGINE_CYLINDER_PATH)

    room = base.create_blueprint(ROOM_ASSET_NAME)
    base.set_room_defaults(
        room,
        "EntryFacadeNight",
        "EntryFacadeNight",
        1.0,
        1,
        1,
        True,
        unreal.LinearColor(0.16, 0.78, 0.96, 1.0),
    )

    room_size = unreal.Vector(2600.0, 1800.0, 920.0)
    base.set_room_bounds(room, room_size)
    base.disable_parametric_base(room)
    base.enable_stock_graybox(room, floor_thickness=24.0, wall_thickness=32.0, ceiling_thickness=28.0, door_width=240.0, door_height=320.0)
    base.set_stock_materials(room, entry_floor_material, entry_wall_material, ceiling_material)
    base.set_placement_rules(
        room,
        unreal.RoomPlacementRole.START,
        allow_main_path=False,
        allow_branch=False,
        can_terminate=True,
        min_depth=0,
        max_depth=0,
        max_instances=1,
    )
    base.set_allowed_neighbors(room, ["PublicHallStraight"])
    base.enable_entry_player_start(room)

    cdo = base.get_cdo(room)
    cdo.set_editor_property("PlayerStartLocalOffset", unreal.Vector(0.0, -640.0, 140.0))
    base.add_cardinal_connectors(room, room_size, include_east=False, include_west=False, include_north=True, include_south=False)

    allowed_geo = set()

    base.add_feature_mesh(
        room,
        "Geo_Apron",
        cube_mesh,
        unreal.Vector(0.0, -120.0, 8.0),
        unreal.Rotator(),
        unreal.Vector(11.6, 8.0, 0.12),
        hall_floor_material,
    )
    allowed_geo.add("Geo_Apron")

    base.add_feature_mesh(
        room,
        "Geo_Curb",
        cube_mesh,
        unreal.Vector(0.0, -700.0, 18.0),
        unreal.Rotator(),
        unreal.Vector(11.8, 0.18, 0.20),
        service_material,
    )
    allowed_geo.add("Geo_Curb")

    base.add_feature_mesh(
        room,
        "Geo_SkyPanel",
        cube_mesh,
        unreal.Vector(0.0, -40.0, 790.0),
        unreal.Rotator(),
        unreal.Vector(13.0, 9.0, 0.08),
        sky_material,
    )
    allowed_geo.add("Geo_SkyPanel")

    base.add_feature_mesh(
        room,
        "Geo_FacadeLeft",
        cube_mesh,
        unreal.Vector(-720.0, 640.0, 270.0),
        unreal.Rotator(),
        unreal.Vector(4.3, 0.22, 4.7),
        entry_wall_material,
    )
    allowed_geo.add("Geo_FacadeLeft")

    base.add_feature_mesh(
        room,
        "Geo_FacadeRight",
        cube_mesh,
        unreal.Vector(720.0, 640.0, 270.0),
        unreal.Rotator(),
        unreal.Vector(4.3, 0.22, 4.7),
        entry_wall_material,
    )
    allowed_geo.add("Geo_FacadeRight")

    base.add_feature_mesh(
        room,
        "Geo_FacadeLintel",
        cube_mesh,
        unreal.Vector(0.0, 640.0, 455.0),
        unreal.Rotator(),
        unreal.Vector(2.8, 0.22, 1.0),
        entry_wall_material,
    )
    allowed_geo.add("Geo_FacadeLintel")

    base.add_feature_mesh(
        room,
        "Geo_DoorAwning",
        cube_mesh,
        unreal.Vector(0.0, 500.0, 400.0),
        unreal.Rotator(),
        unreal.Vector(3.1, 1.2, 0.16),
        service_material,
    )
    allowed_geo.add("Geo_DoorAwning")

    base.add_feature_mesh(
        room,
        "Geo_NeonSign",
        cube_mesh,
        unreal.Vector(0.0, 520.0, 520.0),
        unreal.Rotator(),
        unreal.Vector(2.6, 0.10, 0.72),
        sky_material,
    )
    allowed_geo.add("Geo_NeonSign")

    base.add_feature_mesh(
        room,
        "Geo_BoothCounter",
        cube_mesh,
        unreal.Vector(0.0, 420.0, 58.0),
        unreal.Rotator(),
        unreal.Vector(1.6, 0.55, 0.55),
        entry_wall_material,
    )
    allowed_geo.add("Geo_BoothCounter")

    base.add_feature_mesh(
        room,
        "Geo_BenchWest",
        cube_mesh,
        unreal.Vector(-760.0, -40.0, 30.0),
        unreal.Rotator(),
        unreal.Vector(2.0, 0.48, 0.24),
        entry_wall_material,
    )
    allowed_geo.add("Geo_BenchWest")

    base.add_feature_mesh(
        room,
        "Geo_AshCanWest",
        cylinder_mesh,
        unreal.Vector(-520.0, -140.0, 44.0),
        unreal.Rotator(),
        unreal.Vector(0.34, 0.34, 0.86),
        service_material,
    )
    allowed_geo.add("Geo_AshCanWest")

    base.add_feature_mesh(
        room,
        "Geo_VendingBody",
        cube_mesh,
        unreal.Vector(860.0, 40.0, 150.0),
        unreal.Rotator(),
        unreal.Vector(1.0, 0.62, 3.0),
        service_material,
    )
    allowed_geo.add("Geo_VendingBody")

    base.add_feature_mesh(
        room,
        "Geo_VendingFront",
        cube_mesh,
        unreal.Vector(920.0, 40.0, 160.0),
        unreal.Rotator(),
        unreal.Vector(0.08, 0.56, 2.8),
        sky_material,
    )
    allowed_geo.add("Geo_VendingFront")

    base.add_feature_mesh(
        room,
        "Geo_PosterCase",
        cube_mesh,
        unreal.Vector(-980.0, 320.0, 220.0),
        unreal.Rotator(),
        unreal.Vector(0.08, 1.0, 1.8),
        sky_material,
    )
    allowed_geo.add("Geo_PosterCase")

    base.add_feature_mesh(
        room,
        "Geo_ParkingStripe",
        cube_mesh,
        unreal.Vector(-480.0, -520.0, 12.0),
        unreal.Rotator(),
        unreal.Vector(2.4, 0.08, 0.03),
        sky_material,
    )
    allowed_geo.add("Geo_ParkingStripe")

    base.remove_unwanted_feature_meshes(room, allowed_geo)
    base.compile_and_save(room)
    log(f"Synchronized contained-exterior start room: {ROOM_ASSET_NAME}")


if __name__ == "__main__":
    main()
