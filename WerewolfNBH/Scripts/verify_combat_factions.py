import json
import unreal


OUT_PATH = r"E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Saved/combat_factions_verification.json"


def serialize(value):
    if value is None:
        return None
    if hasattr(value, "get_path_name"):
        return value.get_path_name()
    if isinstance(value, (bool, int, float, str)):
        return value
    if isinstance(value, list):
        return [serialize(item) for item in value]
    return str(value)


def inspect_bp(path, props):
    result = {
        "path": path,
        "exists": unreal.EditorAssetLibrary.does_asset_exist(path),
    }
    asset = unreal.load_asset(path)
    if not asset:
        return result

    result["class"] = asset.get_class().get_name()
    try:
        result["parent_class"] = serialize(asset.get_editor_property("parent_class"))
    except Exception:
        pass

    if hasattr(asset, "generated_class"):
        gc = asset.generated_class()
        result["generated_class"] = serialize(gc)
        if gc:
            cdo = unreal.get_default_object(gc)
            values = {}
            for prop in props:
                try:
                    values[prop] = serialize(cdo.get_editor_property(prop))
                except Exception as ex:
                    values[prop] = f"<error: {ex}>"
            result["properties"] = values
    return result


report = {
    "shooter_controller": inspect_bp(
        "/Game/Variant_Shooter/Blueprints/AI/BP_ShooterAIController",
        ["combat_faction", "attack_players", "attack_same_faction", "attack_other_combat_factions"],
    ),
    "shooter_npc": inspect_bp(
        "/Game/Variant_Shooter/Blueprints/AI/BP_ShooterNPC",
        ["ai_controller_class", "tags"],
    ),
    "magic_ai_controller": inspect_bp(
        "/Game/Magic/Blueprints/AI/BP_MagicAIController",
        ["combat_faction", "attack_players", "attack_same_faction", "attack_other_combat_factions"],
    ),
    "magic_npc": inspect_bp(
        "/Game/Magic/Blueprints/BP_MagicNPC",
        ["ai_controller_class", "starting_spell_class", "tags"],
    ),
}

with open(OUT_PATH, "w", encoding="utf-8") as handle:
    json.dump(report, handle, indent=2)

unreal.log(f"Wrote {OUT_PATH}")
