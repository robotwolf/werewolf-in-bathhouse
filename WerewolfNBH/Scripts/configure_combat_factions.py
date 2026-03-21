import unreal


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
ROOT = "/Game/Magic/Blueprints/AI"


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


def set_tags(cdo, tag_names):
    cdo.set_editor_property("tags", [unreal.Name(name) for name in tag_names])


def main():
    ensure_dir(ROOT)

    shooter_controller_bp = unreal.load_asset("/Game/Variant_Shooter/Blueprints/AI/BP_ShooterAIController")
    shooter_npc_bp = unreal.load_asset("/Game/Variant_Shooter/Blueprints/AI/BP_ShooterNPC")
    spell_bolt_bp = unreal.load_asset("/Game/Magic/Blueprints/Weapons/BP_SpellWeapon_Bolt")

    if shooter_controller_bp:
        unreal.BlueprintEditorLibrary.reparent_blueprint(shooter_controller_bp, unreal.CombatFactionAIController)
        shooter_controller_cdo = unreal.get_default_object(shooter_controller_bp.generated_class())
        shooter_controller_cdo.set_editor_property("combat_faction", unreal.CombatFaction.SHOOTER)
        shooter_controller_cdo.set_editor_property("attack_players", False)
        shooter_controller_cdo.set_editor_property("attack_same_faction", False)
        shooter_controller_cdo.set_editor_property("attack_other_combat_factions", True)
        save_compile(shooter_controller_bp)

    if shooter_npc_bp:
        shooter_npc_cdo = unreal.get_default_object(shooter_npc_bp.generated_class())
        set_tags(shooter_npc_cdo, ["CombatFaction.Shooter"])
        save_compile(shooter_npc_bp)

    magic_ai_controller_bp = ensure_blueprint(f"{ROOT}/BP_MagicAIController", unreal.MagicCombatAIController)
    magic_ai_controller_cdo = unreal.get_default_object(magic_ai_controller_bp.generated_class())
    magic_ai_controller_cdo.set_editor_property("combat_faction", unreal.CombatFaction.MAGIC)
    magic_ai_controller_cdo.set_editor_property("attack_players", False)
    magic_ai_controller_cdo.set_editor_property("attack_same_faction", False)
    magic_ai_controller_cdo.set_editor_property("attack_other_combat_factions", True)
    save_compile(magic_ai_controller_bp)

    magic_npc_bp = ensure_blueprint("/Game/Magic/Blueprints/BP_MagicNPC", unreal.MagicNPCCharacter)
    magic_npc_cdo = unreal.get_default_object(magic_npc_bp.generated_class())
    magic_npc_cdo.set_editor_property("ai_controller_class", magic_ai_controller_bp.generated_class())
    if spell_bolt_bp:
        magic_npc_cdo.set_editor_property("starting_spell_class", spell_bolt_bp.generated_class())
    set_tags(magic_npc_cdo, ["CombatFaction.Magic"])
    save_compile(magic_npc_bp)

    unreal.EditorAssetLibrary.save_directory("/Game/Variant_Shooter")
    unreal.EditorAssetLibrary.save_directory("/Game/Magic")
    unreal.log("[configure_combat_factions] Combat factions configured.")


if __name__ == "__main__":
    main()
