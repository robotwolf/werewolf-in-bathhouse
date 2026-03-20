import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(__file__)
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

import setup_bathhouse_rooms as base


ROOM_ASSET_NAME = "BP_Room_SmokingPatioPocket"
TRON_GLOW_PATH = "/Game/WerewolfBH/Materials/Tron_Glow"


def log(message: str) -> None:
    unreal.log(f"[sync_contained_exterior_room] {message}")


def main():
    base.ensure_folder(base.ROOMS_PATH)

    hall_floor_material = base.load_required_asset(base.HALL_FLOOR_MATERIAL_PATH)
    hall_wall_material = base.load_required_asset(base.HALL_WALL_MATERIAL_PATH)
    ceiling_material = base.load_required_asset(base.CEILING_MATERIAL_PATH)
    service_material = base.load_required_asset(base.SERVICE_METAL_MATERIAL_PATH)
    entry_wall_material = base.load_required_asset(base.ENTRY_WALL_MATERIAL_PATH)
    sky_material = base.load_required_asset(TRON_GLOW_PATH)
    cube_mesh = base.load_required_asset(base.ENGINE_CUBE_PATH)
    cylinder_mesh = base.load_required_asset(base.ENGINE_CYLINDER_PATH)

    room = base.create_blueprint(ROOM_ASSET_NAME)
    base.set_room_defaults(
        room,
        "SmokingPatioPocket",
        "SmokingPatioPocket",
        0.14,
        1,
        1,
        False,
        unreal.LinearColor(0.40, 0.86, 0.64, 1.0),
    )

    room_size = unreal.Vector(2200.0, 1800.0, 900.0)
    base.set_room_bounds(room, room_size)
    base.disable_parametric_base(room)
    base.enable_stock_graybox(room, floor_thickness=24.0, wall_thickness=32.0, ceiling_thickness=28.0, door_width=220.0, door_height=300.0)
    base.set_stock_materials(room, hall_floor_material, hall_wall_material, ceiling_material)
    base.set_placement_rules(
        room,
        unreal.RoomPlacementRole.BRANCH,
        allow_main_path=False,
        allow_branch=True,
        can_terminate=True,
        min_depth=4,
        max_depth=12,
        max_instances=1,
    )
    base.set_allowed_neighbors(room, ["PublicHallStraight", "PublicHallCorner", "PublicHallLTurn", "PoolHall"])
    base.add_cardinal_connectors(room, room_size, include_east=False, include_west=False, include_north=False, include_south=True)

    allowed_geo = set()

    base.add_feature_mesh(
        room,
        "Geo_PatioFloor",
        cube_mesh,
        unreal.Vector(0.0, 120.0, 6.0),
        unreal.Rotator(),
        unreal.Vector(11.0, 8.0, 0.12),
        hall_floor_material,
    )
    allowed_geo.add("Geo_PatioFloor")

    base.add_feature_mesh(
        room,
        "Geo_SkyPanel",
        cube_mesh,
        unreal.Vector(0.0, 140.0, 760.0),
        unreal.Rotator(),
        unreal.Vector(12.0, 9.0, 0.08),
        sky_material,
    )
    allowed_geo.add("Geo_SkyPanel")

    base.add_feature_mesh(
        room,
        "Geo_BackFacade",
        cube_mesh,
        unreal.Vector(0.0, 620.0, 260.0),
        unreal.Rotator(),
        unreal.Vector(10.5, 0.24, 4.8),
        entry_wall_material,
    )
    allowed_geo.add("Geo_BackFacade")

    base.add_feature_mesh(
        room,
        "Geo_WestFacade",
        cube_mesh,
        unreal.Vector(-860.0, 140.0, 240.0),
        unreal.Rotator(),
        unreal.Vector(0.22, 4.2, 4.2),
        entry_wall_material,
    )
    allowed_geo.add("Geo_WestFacade")

    base.add_feature_mesh(
        room,
        "Geo_EastFacade",
        cube_mesh,
        unreal.Vector(860.0, 180.0, 230.0),
        unreal.Rotator(),
        unreal.Vector(0.22, 4.8, 4.1),
        entry_wall_material,
    )
    allowed_geo.add("Geo_EastFacade")

    base.add_feature_mesh(
        room,
        "Geo_SmokeAwning",
        cube_mesh,
        unreal.Vector(-520.0, -40.0, 340.0),
        unreal.Rotator(),
        unreal.Vector(3.2, 1.9, 0.12),
        service_material,
    )
    allowed_geo.add("Geo_SmokeAwning")

    for index, x_offset in enumerate((-700.0, -340.0)):
        name = f"Geo_AwningPost_{index}"
        base.add_feature_mesh(
            room,
            name,
            cube_mesh,
            unreal.Vector(x_offset, -40.0, 170.0),
            unreal.Rotator(),
            unreal.Vector(0.10, 0.10, 3.3),
            service_material,
        )
        allowed_geo.add(name)

    base.add_feature_mesh(
        room,
        "Geo_Bench",
        cube_mesh,
        unreal.Vector(-560.0, 100.0, 28.0),
        unreal.Rotator(),
        unreal.Vector(2.2, 0.5, 0.26),
        entry_wall_material,
    )
    allowed_geo.add("Geo_Bench")

    base.add_feature_mesh(
        room,
        "Geo_AshCan",
        cylinder_mesh,
        unreal.Vector(-220.0, -60.0, 42.0),
        unreal.Rotator(),
        unreal.Vector(0.35, 0.35, 0.84),
        service_material,
    )
    allowed_geo.add("Geo_AshCan")

    base.add_feature_mesh(
        room,
        "Geo_PortaBody",
        cube_mesh,
        unreal.Vector(660.0, 360.0, 155.0),
        unreal.Rotator(),
        unreal.Vector(1.15, 1.05, 3.05),
        service_material,
    )
    allowed_geo.add("Geo_PortaBody")

    base.add_feature_mesh(
        room,
        "Geo_PortaDoor",
        cube_mesh,
        unreal.Vector(720.0, 360.0, 150.0),
        unreal.Rotator(),
        unreal.Vector(0.08, 0.90, 2.70),
        sky_material,
    )
    allowed_geo.add("Geo_PortaDoor")

    base.add_feature_mesh(
        room,
        "Geo_PortaVent",
        cube_mesh,
        unreal.Vector(660.0, 360.0, 312.0),
        unreal.Rotator(),
        unreal.Vector(0.90, 0.12, 0.20),
        sky_material,
    )
    allowed_geo.add("Geo_PortaVent")

    base.remove_unwanted_feature_meshes(room, allowed_geo)
    base.compile_and_save(room)
    log(f"Synchronized contained exterior room: {ROOM_ASSET_NAME}")


if __name__ == "__main__":
    main()
