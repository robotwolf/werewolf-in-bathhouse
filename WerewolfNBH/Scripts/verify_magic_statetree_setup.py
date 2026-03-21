import json
import unreal


def gather_subobjects(bp):
    subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsystem.k2_gather_subobject_data_for_blueprint(bp)
    results = []
    for handle in handles:
        data = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(handle)
        if hasattr(unreal.SubobjectDataBlueprintFunctionLibrary, "get_associated_object"):
            obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        else:
            obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
        if obj:
            results.append(f"{obj.get_class().get_name()}:{obj.get_name()}")
    return sorted(results)


def main():
    state_tree = unreal.load_asset("/Game/Magic/Blueprints/AI/ST_MagicNPC")
    magic_npc = unreal.load_asset("/Game/Magic/Blueprints/BP_MagicNPC")
    magic_ai = unreal.load_asset("/Game/Magic/Blueprints/AI/BP_MagicAIController")

    if not state_tree or not magic_npc or not magic_ai:
        raise RuntimeError("Missing one or more magic StateTree assets.")

    npc_components = gather_subobjects(magic_npc)
    ai_components = gather_subobjects(magic_ai)

    payload = {
        "state_tree": state_tree.get_path_name(),
        "npc_has_text_render": any("TextRenderComponent" in item for item in npc_components),
        "npc_components": npc_components,
        "ai_components": ai_components,
    }

    unreal.log("[verify_magic_statetree_setup] " + json.dumps(payload, indent=2))


if __name__ == "__main__":
    main()
