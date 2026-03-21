import json
import unreal


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()

ROOT = "/Game/Magic"
BLUEPRINTS = f"{ROOT}/Blueprints"
WEAPONS = f"{BLUEPRINTS}/Weapons"
PROJECTILES = f"{BLUEPRINTS}/Projectiles"
PICKUPS = f"{BLUEPRINTS}/Pickups"
UI = f"{ROOT}/UI"
MAPS = f"{ROOT}/Maps"


def log(message):
    unreal.log(f"[create_magic_manny_assets] {message}")


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def split_path(asset_path):
    folder, name = asset_path.rsplit("/", 1)
    return folder, name


def save_compile(bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_loaded_asset(bp)


def ensure_blueprint(asset_path, parent_class):
    asset = unreal.load_asset(asset_path)
    if asset:
        return asset

    folder, name = split_path(asset_path)
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", parent_class)
    bp = ASSET_TOOLS.create_asset(name, folder, unreal.Blueprint, factory)
    save_compile(bp)
    return bp


def ensure_widget_blueprint(asset_path, parent_class):
    asset = unreal.load_asset(asset_path)
    if asset:
        return asset

    folder, name = split_path(asset_path)
    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    bp = ASSET_TOOLS.create_asset(name, folder, unreal.WidgetBlueprint, factory)
    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    return bp


def gather_subobjects(bp):
    subsys = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsys.k2_gather_subobject_data_for_blueprint(bp)
    subobjects = {}
    for handle in handles:
        data = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(handle)
        if hasattr(unreal.SubobjectDataBlueprintFunctionLibrary, "get_associated_object"):
            obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        else:
            obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
        if obj:
            subobjects[obj.get_name()] = obj
    return subobjects


def find_named_subobject(bp, contains):
    contains = contains.lower()
    for name, obj in gather_subobjects(bp).items():
        if contains in name.lower():
            return obj
    return None


def set_projectile_config(cdo, values):
    config = cdo.get_editor_property("projectile_config")
    for key, value in values.items():
        config.set_editor_property(key, value)
    cdo.set_editor_property("projectile_config", config)


def try_create_magic_map(map_path, game_mode_bp, pickup_bps):
    source_map = "/Game/Variant_Shooter/Lvl_ArenaShooter"
    if not unreal.EditorAssetLibrary.does_asset_exist(map_path):
        if unreal.EditorAssetLibrary.does_asset_exist(source_map):
            unreal.EditorAssetLibrary.duplicate_asset(source_map, map_path)
            log(f"Duplicated map {source_map} -> {map_path}")

    try:
        unreal.EditorLoadingAndSavingUtils.load_map(map_path)
        world = unreal.EditorLevelLibrary.get_editor_world()
        if world:
            world_settings = world.get_world_settings()
            try:
                world_settings.set_editor_property("default_game_mode", game_mode_bp.generated_class())
            except Exception:
                pass

            existing_magic_pickups = [
                actor
                for actor in unreal.EditorLevelLibrary.get_all_level_actors()
                if actor.get_class().get_name().startswith("BP_SpellPickup_")
            ]
            if not existing_magic_pickups:
                locations = [
                    unreal.Vector(0.0, -400.0, 80.0),
                    unreal.Vector(0.0, 0.0, 80.0),
                    unreal.Vector(0.0, 400.0, 80.0),
                ]
                for pickup_bp, location in zip(pickup_bps, locations):
                    unreal.EditorLevelLibrary.spawn_actor_from_class(pickup_bp.generated_class(), location)
                log("Spawned three spell pickups into the magic arena map.")

            unreal.EditorLevelLibrary.save_current_level()
    except Exception as ex:
        log(f"Map setup skipped: {ex}")


def update_weapon_table(spell_weapon_bps):
    dt = unreal.load_asset("/Game/Variant_Shooter/Blueprints/Pickups/DT_WeaponList")
    if not dt:
        log("DT_WeaponList not found; skipping table update.")
        return

    rows = json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(dt))
    rows_by_name = {row["Name"]: row for row in rows}
    sphere_mesh_path = "/Engine/BasicShapes/Sphere.Sphere"

    desired_rows = {
        "MagicBolt": spell_weapon_bps["Bolt"],
        "MagicForce": spell_weapon_bps["Force"],
        "MagicAoE": spell_weapon_bps["AoE"],
    }

    for row_name, bp in desired_rows.items():
        rows_by_name[row_name] = {
            "Name": row_name,
            "Static Mesh": sphere_mesh_path,
            "Spawn Actor": f"{bp.get_path_name()}_C",
        }

    ordered_rows = [rows_by_name[name] for name in sorted(rows_by_name.keys())]
    unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(dt, json.dumps(ordered_rows, indent=2))
    unreal.EditorAssetLibrary.save_loaded_asset(dt)
    log("Updated DT_WeaponList with magic spell entries.")


def main():
    for path in [ROOT, BLUEPRINTS, WEAPONS, PROJECTILES, PICKUPS, UI, MAPS]:
        ensure_dir(path)

    wbp_magic_status = ensure_widget_blueprint(f"{UI}/WBP_MagicSpellStatus", unreal.MagicSpellStatusWidget)

    bp_magic_character = ensure_blueprint(f"{BLUEPRINTS}/BP_MagicCasterCharacter", unreal.MagicCasterCharacter)
    bp_magic_controller = ensure_blueprint(f"{BLUEPRINTS}/BP_MagicPlayerController", unreal.MagicPlayerController)
    bp_magic_game_mode = ensure_blueprint(f"{BLUEPRINTS}/BP_MagicGameMode", unreal.MagicGameMode)
    bp_spell_weapon_base = ensure_blueprint(f"{WEAPONS}/BP_SpellWeaponBase", unreal.MagicSpellWeaponBase)
    bp_projectile_base = ensure_blueprint(f"{PROJECTILES}/BP_MagicProjectileBase", unreal.MagicProjectileBase)
    bp_pickups_base = ensure_blueprint(f"{PICKUPS}/BP_SpellPickupBase", unreal.MagicSpellPickup)

    weapon_defs = {
        "Bolt": {
            "display_name": "Arc Bolt",
            "cast_animation": unreal.load_asset("/Game/CombatMagicAnims/Animations/AS_ManaCastShot"),
            "cooldown_seconds": 0.35,
            "cast_lead_time": 0.08,
            "projectile_class": bp_projectile_base.generated_class(),
            "projectile_config": {
                "spell_color": unreal.LinearColor(0.22, 0.78, 1.0, 1.0),
                "projectile_speed": 3400.0,
                "projectile_life_seconds": 3.0,
                "projectile_scale": 0.16,
                "direct_damage": 28.0,
                "impact_force": 125.0,
                "impact_radius": 0.0,
                "light_intensity": 2200.0,
                "light_radius": 300.0,
            },
        },
        "Force": {
            "display_name": "Force Orb",
            "cast_animation": unreal.load_asset("/Game/CombatMagicAnims/Animations/AS_LevitatingMagicAttack"),
            "cooldown_seconds": 0.85,
            "cast_lead_time": 0.14,
            "projectile_class": bp_projectile_base.generated_class(),
            "projectile_config": {
                "spell_color": unreal.LinearColor(0.95, 0.38, 1.0, 1.0),
                "projectile_speed": 2200.0,
                "projectile_life_seconds": 4.0,
                "projectile_scale": 0.28,
                "direct_damage": 10.0,
                "impact_force": 1500.0,
                "impact_radius": 240.0,
                "light_intensity": 2600.0,
                "light_radius": 360.0,
            },
        },
        "AoE": {
            "display_name": "Solar Burst",
            "cast_animation": unreal.load_asset("/Game/CombatMagicAnims/Animations/AS_LevitatingFireballCast"),
            "cooldown_seconds": 1.2,
            "cast_lead_time": 0.18,
            "projectile_class": bp_projectile_base.generated_class(),
            "projectile_config": {
                "spell_color": unreal.LinearColor(1.0, 0.54, 0.16, 1.0),
                "projectile_speed": 1500.0,
                "projectile_life_seconds": 5.0,
                "projectile_scale": 0.42,
                "direct_damage": 22.0,
                "impact_force": 650.0,
                "impact_radius": 320.0,
                "light_intensity": 3200.0,
                "light_radius": 440.0,
                "lingering_duration": 4.0,
                "lingering_damage_per_tick": 7.0,
                "lingering_force": 240.0,
                "lingering_radius": 280.0,
                "lingering_tick_interval": 0.5,
            },
        },
    }

    projectile_bps = {}
    spell_weapon_bps = {}
    pickup_bps = []

    for key, definition in weapon_defs.items():
        projectile_bp = ensure_blueprint(f"{PROJECTILES}/BP_MagicProjectile_{key}", unreal.MagicProjectileBase)
        projectile_bps[key] = projectile_bp

        weapon_bp = ensure_blueprint(f"{WEAPONS}/BP_SpellWeapon_{key}", unreal.MagicSpellWeaponBase)
        weapon_cdo = unreal.get_default_object(weapon_bp.generated_class())
        weapon_cdo.set_editor_property("spell_display_name", definition["display_name"])
        weapon_cdo.set_editor_property("cast_animation", definition["cast_animation"])
        weapon_cdo.set_editor_property("cooldown_seconds", definition["cooldown_seconds"])
        weapon_cdo.set_editor_property("cast_lead_time", definition["cast_lead_time"])
        weapon_cdo.set_editor_property("projectile_class", projectile_bp.generated_class())
        set_projectile_config(weapon_cdo, definition["projectile_config"])
        save_compile(weapon_bp)
        spell_weapon_bps[key] = weapon_bp

        pickup_bp = ensure_blueprint(f"{PICKUPS}/BP_SpellPickup_{key}", unreal.MagicSpellPickup)
        pickup_cdo = unreal.get_default_object(pickup_bp.generated_class())
        pickup_cdo.set_editor_property("spell_weapon_class", weapon_bp.generated_class())

        display_mesh = find_named_subobject(pickup_bp, "DisplayMesh")
        if display_mesh:
            display_mesh.set_editor_property("relative_scale3d", unreal.Vector(definition["projectile_config"]["projectile_scale"] * 3.5, definition["projectile_config"]["projectile_scale"] * 3.5, definition["projectile_config"]["projectile_scale"] * 3.5))

        save_compile(pickup_bp)
        pickup_bps.append(pickup_bp)

    controller_cdo = unreal.get_default_object(bp_magic_controller.generated_class())
    controller_cdo.set_editor_property("spell_status_widget_class", wbp_magic_status.generated_class())
    save_compile(bp_magic_controller)

    game_mode_cdo = unreal.get_default_object(bp_magic_game_mode.generated_class())
    game_mode_cdo.set_editor_property("default_pawn_class", bp_magic_character.generated_class())
    game_mode_cdo.set_editor_property("player_controller_class", bp_magic_controller.generated_class())
    save_compile(bp_magic_game_mode)

    update_weapon_table(spell_weapon_bps)
    try_create_magic_map(f"{MAPS}/Lvl_MagicArena", bp_magic_game_mode, pickup_bps)

    unreal.EditorAssetLibrary.save_directory(ROOT)
    log("Magic Manny assets created/updated.")


if __name__ == "__main__":
    main()
