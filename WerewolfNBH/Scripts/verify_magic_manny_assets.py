import json
import unreal


OUT_PATH = r"E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Saved/magic_manny_verification.json"


def asset_exists(path):
    return unreal.EditorAssetLibrary.does_asset_exist(path)


def object_path(value):
    if not value:
        return None
    if hasattr(value, "get_path_name"):
        return value.get_path_name()
    return str(value)


def inspect_bp(path, props):
    result = {
        "path": path,
        "exists": asset_exists(path),
    }

    asset = unreal.load_asset(path)
    if not asset:
        return result

    result["class"] = asset.get_class().get_name()

    generated_class = None
    if hasattr(asset, "generated_class"):
        generated_class = asset.generated_class()
    result["generated_class"] = object_path(generated_class)

    if not generated_class:
        return result

    cdo = unreal.get_default_object(generated_class)
    values = {}
    for prop_name in props:
        try:
            value = cdo.get_editor_property(prop_name)
            if isinstance(value, unreal.Text):
                values[prop_name] = str(value)
            elif isinstance(value, unreal.LinearColor):
                values[prop_name] = {
                    "r": value.r,
                    "g": value.g,
                    "b": value.b,
                    "a": value.a,
                }
            else:
                values[prop_name] = object_path(value)
        except Exception as ex:
            values[prop_name] = f"<error: {ex}>"

    result["properties"] = values
    return result


def inspect_spell_weapon(path):
    result = inspect_bp(
        path,
        [
            "spell_display_name",
            "cast_animation",
            "cooldown_seconds",
            "cast_lead_time",
            "projectile_class",
            "projectile_config",
        ],
    )

    asset = unreal.load_asset(path)
    if not asset or not hasattr(asset, "generated_class"):
        return result

    cdo = unreal.get_default_object(asset.generated_class())
    try:
        config = cdo.get_editor_property("projectile_config")
        result["projectile_config"] = {
            "spell_color": {
                "r": config.spell_color.r,
                "g": config.spell_color.g,
                "b": config.spell_color.b,
                "a": config.spell_color.a,
            },
            "projectile_speed": config.projectile_speed,
            "projectile_life_seconds": config.projectile_life_seconds,
            "projectile_scale": config.projectile_scale,
            "direct_damage": config.direct_damage,
            "impact_force": config.impact_force,
            "impact_radius": config.impact_radius,
            "light_intensity": config.light_intensity,
            "light_radius": config.light_radius,
            "lingering_duration": config.lingering_duration,
            "lingering_damage_per_tick": config.lingering_damage_per_tick,
            "lingering_force": config.lingering_force,
            "lingering_radius": config.lingering_radius,
            "lingering_tick_interval": config.lingering_tick_interval,
            "lingering_field_class": object_path(config.lingering_field_class),
        }
    except Exception as ex:
        result["projectile_config_error"] = str(ex)
    return result


def inspect_data_table(path):
    result = {
        "path": path,
        "exists": asset_exists(path),
        "rows": {},
    }

    table = unreal.load_asset(path)
    if not table:
        return result

    rows = json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table))
    for row in rows:
        name = row.get("Name")
        if name in ("MagicBolt", "MagicForce", "MagicAoE"):
            result["rows"][name] = row

    return result


def main():
    report = {
        "character": inspect_bp(
            "/Game/Magic/Blueprints/BP_MagicCasterCharacter",
            [
                "idle_levitation_animation",
                "default_input_mapping",
                "mouse_look_input_mapping",
                "weapon_input_mapping",
                "move_action",
                "look_action",
                "jump_action_asset",
                "shoot_action",
            ],
        ),
        "controller": inspect_bp(
            "/Game/Magic/Blueprints/BP_MagicPlayerController",
            ["spell_status_widget_class"],
        ),
        "game_mode": inspect_bp(
            "/Game/Magic/Blueprints/BP_MagicGameMode",
            ["default_pawn_class", "player_controller_class"],
        ),
        "pickups": {
            "bolt": inspect_bp("/Game/Magic/Blueprints/Pickups/BP_SpellPickup_Bolt", ["spell_weapon_class"]),
            "force": inspect_bp("/Game/Magic/Blueprints/Pickups/BP_SpellPickup_Force", ["spell_weapon_class"]),
            "aoe": inspect_bp("/Game/Magic/Blueprints/Pickups/BP_SpellPickup_AoE", ["spell_weapon_class"]),
        },
        "weapons": {
            "bolt": inspect_spell_weapon("/Game/Magic/Blueprints/Weapons/BP_SpellWeapon_Bolt"),
            "force": inspect_spell_weapon("/Game/Magic/Blueprints/Weapons/BP_SpellWeapon_Force"),
            "aoe": inspect_spell_weapon("/Game/Magic/Blueprints/Weapons/BP_SpellWeapon_AoE"),
        },
        "projectiles": {
            "base": asset_exists("/Game/Magic/Blueprints/Projectiles/BP_MagicProjectileBase"),
            "bolt": asset_exists("/Game/Magic/Blueprints/Projectiles/BP_MagicProjectile_Bolt"),
            "force": asset_exists("/Game/Magic/Blueprints/Projectiles/BP_MagicProjectile_Force"),
            "aoe": asset_exists("/Game/Magic/Blueprints/Projectiles/BP_MagicProjectile_AoE"),
        },
        "ui": {
            "spell_status_widget": asset_exists("/Game/Magic/UI/WBP_MagicSpellStatus"),
        },
        "map": {
            "magic_arena": asset_exists("/Game/Magic/Maps/Lvl_MagicArena"),
        },
        "weapon_table": inspect_data_table("/Game/Variant_Shooter/Blueprints/Pickups/DT_WeaponList"),
    }

    with open(OUT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    unreal.log(f"Wrote {OUT_PATH}")


if __name__ == "__main__":
    main()
