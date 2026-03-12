import unreal

ASSET_ROOT = "/Game/UI"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def create_material(name, folder):
    asset_path = f"{folder}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Material exists: {asset_path}")
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    factory = unreal.MaterialFactoryNew()
    mat = asset_tools.create_asset(name, folder, unreal.Material, factory)
    return mat


def create_material_instance(name, folder, parent):
    asset_path = f"{folder}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Material instance exists: {asset_path}")
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    factory = unreal.MaterialInstanceConstantFactoryNew()
    mi = asset_tools.create_asset(name, folder, unreal.MaterialInstanceConstant, factory)
    mi.set_editor_property("parent", parent)
    return mi


def create_widget_blueprint(name, folder):
    asset_path = f"{folder}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Widget exists: {asset_path}")
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", unreal.UserWidget)
    wb = asset_tools.create_asset(name, folder, unreal.WidgetBlueprint, factory)
    return wb


def set_material_common_ui_settings(mat, blend_mode):
    mat.set_editor_property("material_domain", unreal.MaterialDomain.MD_UI)
    mat.set_editor_property("blend_mode", blend_mode)
    # Try to set unlit shading model if exposed on this version
    try:
        mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    except Exception:
        try:
            mat.set_editor_property("shading_models", [unreal.MaterialShadingModel.MSM_UNLIT])
        except Exception:
            pass


def build_glass_material(mat):
    mel = unreal.MaterialEditingLibrary
    mel.delete_all_material_expressions(mat)

    # Parameters
    glass_color = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -600, -200)
    glass_color.set_editor_property("parameter_name", "Glass_Color")
    glass_color.set_editor_property("default_value", unreal.LinearColor(0.839, 0.847, 0.839, 1.0))  # #D6D8D6

    glass_opacity = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -600, 0)
    glass_opacity.set_editor_property("parameter_name", "Glass_Opacity")
    glass_opacity.set_editor_property("default_value", 0.90)

    edge_dark = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -600, 200)
    edge_dark.set_editor_property("parameter_name", "Edge_Dark")
    edge_dark.set_editor_property("default_value", unreal.LinearColor(0.663, 0.682, 0.667, 1.0))  # #A9AEAA

    edge_strength = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -600, 380)
    edge_strength.set_editor_property("parameter_name", "Edge_Strength")
    edge_strength.set_editor_property("default_value", 0.25)

    noise_strength = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -600, 560)
    noise_strength.set_editor_property("parameter_name", "Noise_Strength")
    noise_strength.set_editor_property("default_value", 0.08)

    # UVs
    uv = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -400, -40)

    const_05 = mel.create_material_expression(mat, unreal.MaterialExpressionConstant2Vector, -400, 120)
    const_05.set_editor_property("r", 0.5)
    const_05.set_editor_property("g", 0.5)

    sub = mel.create_material_expression(mat, unreal.MaterialExpressionSubtract, -200, 20)
    mel.connect_material_expressions(uv, "", sub, "A")
    mel.connect_material_expressions(const_05, "", sub, "B")

    length = mel.create_material_expression(mat, unreal.MaterialExpressionLength, 0, 20)
    mel.connect_material_expressions(sub, "", length, "")

    one_minus = mel.create_material_expression(mat, unreal.MaterialExpressionOneMinus, 200, 20)
    mel.connect_material_expressions(length, "", one_minus, "")

    edge_mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, 400, 20)
    mel.connect_material_expressions(one_minus, "", edge_mul, "A")
    mel.connect_material_expressions(edge_strength, "", edge_mul, "B")

    # Noise
    const_0 = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -400, 320)
    const_0.set_editor_property("r", 0.0)

    append_uv = mel.create_material_expression(mat, unreal.MaterialExpressionAppendVector, -200, 320)
    mel.connect_material_expressions(uv, "", append_uv, "A")
    mel.connect_material_expressions(const_0, "", append_uv, "B")

    noise = mel.create_material_expression(mat, unreal.MaterialExpressionNoise, 0, 320)
    mel.connect_material_expressions(append_uv, "", noise, "Position")

    noise_mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, 200, 320)
    mel.connect_material_expressions(noise, "", noise_mul, "A")
    mel.connect_material_expressions(noise_strength, "", noise_mul, "B")

    edge_add = mel.create_material_expression(mat, unreal.MaterialExpressionAdd, 600, 80)
    mel.connect_material_expressions(edge_mul, "", edge_add, "A")
    mel.connect_material_expressions(noise_mul, "", edge_add, "B")

    # Lerp between glass color and edge dark
    lerp = mel.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, 800, 20)
    mel.connect_material_expressions(glass_color, "", lerp, "A")
    mel.connect_material_expressions(edge_dark, "", lerp, "B")
    mel.connect_material_expressions(edge_add, "", lerp, "Alpha")

    # Output
    mel.connect_material_property(lerp, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(glass_opacity, "", unreal.MaterialProperty.MP_OPACITY)

    mel.recompile_material(mat)


def build_brass_material(mat):
    mel = unreal.MaterialEditingLibrary
    mel.delete_all_material_expressions(mat)

    brass_base = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -400, -60)
    brass_base.set_editor_property("parameter_name", "Brass_Base")
    brass_base.set_editor_property("default_value", unreal.LinearColor(0.784, 0.635, 0.325, 1.0))  # #C8A253

    brass_glow = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -400, 120)
    brass_glow.set_editor_property("parameter_name", "Brass_Glow")
    brass_glow.set_editor_property("default_value", unreal.LinearColor(0.882, 0.769, 0.478, 1.0))  # #E1C47A

    glow_strength = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -400, 300)
    glow_strength.set_editor_property("parameter_name", "Glow_Strength")
    glow_strength.set_editor_property("default_value", 0.15)

    glow_mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -100, 200)
    mel.connect_material_expressions(brass_glow, "", glow_mul, "A")
    mel.connect_material_expressions(glow_strength, "", glow_mul, "B")

    add = mel.create_material_expression(mat, unreal.MaterialExpressionAdd, 150, 60)
    mel.connect_material_expressions(brass_base, "", add, "A")
    mel.connect_material_expressions(glow_mul, "", add, "B")

    mel.connect_material_property(add, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.recompile_material(mat)


def main():
    # Ensure directories
    ensure_dir(ASSET_ROOT)
    ensure_dir(f"{ASSET_ROOT}/Materials")
    ensure_dir(f"{ASSET_ROOT}/Materials/Instances")
    ensure_dir(f"{ASSET_ROOT}/Widgets")
    ensure_dir(f"{ASSET_ROOT}/Widgets/HUD")
    ensure_dir(f"{ASSET_ROOT}/Widgets/Investigation")
    ensure_dir(f"{ASSET_ROOT}/Widgets/Social")
    ensure_dir(f"{ASSET_ROOT}/Widgets/Settings")
    ensure_dir(f"{ASSET_ROOT}/Widgets/Shared")

    # Materials
    m_glass = create_material("M_UI_Glass", f"{ASSET_ROOT}/Materials")
    set_material_common_ui_settings(m_glass, unreal.BlendMode.BLEND_ALPHA_COMPOSITE)
    build_glass_material(m_glass)

    m_brass = create_material("M_UI_Brass", f"{ASSET_ROOT}/Materials")
    set_material_common_ui_settings(m_brass, unreal.BlendMode.BLEND_OPAQUE)
    build_brass_material(m_brass)

    # Material Instances
    mi_glass = create_material_instance("MI_UI_Glass", f"{ASSET_ROOT}/Materials/Instances", m_glass)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi_glass, "Glass_Opacity", 0.90)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi_glass, "Edge_Strength", 0.25)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi_glass, "Noise_Strength", 0.08)

    mi_brass = create_material_instance("MI_UI_Brass", f"{ASSET_ROOT}/Materials/Instances", m_brass)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi_brass, "Glow_Strength", 0.15)

    # Widget blueprints (empty shells)
    widgets = {
        "HUD": [
            "WBP_HUDRoot",
            "WBP_HUDTopLeftStack",
            "WBP_HUDTopRightStack",
            "WBP_HUDBottomLeftStack",
            "WBP_HUDBottomRightStack",
            "WBP_HUDInteractionPrompt",
            "WBP_DirectionalCue",
            "WBP_SteamMeter",
            "WBP_MoonMeter",
            "WBP_SuspicionMeter",
            "WBP_CoverMeter",
            "WBP_TowelCount",
            "WBP_ObjectiveLine",
            "WBP_FullMoonIndicator",
            "WBP_DoorStateBadge",
            "WBP_AccessTagBadge",
            "WBP_RoomAnomalyTracker",
        ],
        "Investigation": [
            "WBP_ClueFeed",
            "WBP_ClueLog",
            "WBP_EvidenceBoard",
            "WBP_SuspectList",
        ],
        "Social": [
            "WBP_NPCConesToggle",
            "WBP_HeatDeliriumPrompt",
        ],
        "Settings": [
            "WBP_SettingsRoot",
            "WBP_ColorblindLegend",
        ],
        "Shared": [
            "WBP_LabelPlacard",
            "WBP_IconBadge",
            "WBP_MeterTotemBase",
            "WBP_MeterFill",
            "WBP_SuspectConfidenceRing",
            "WBP_ClueRow",
            "WBP_SuspectRow",
            "WBP_SliderRow",
            "WBP_SubtitleLine",
            "WBP_OverlayInvestigation",
            "WBP_OverlaySocial",
        ],
    }

    for folder, names in widgets.items():
        target_folder = f"{ASSET_ROOT}/Widgets/{folder}"
        for name in names:
            create_widget_blueprint(name, target_folder)

    # Save all
    unreal.EditorAssetLibrary.save_directory(ASSET_ROOT)
    unreal.log("Spa-Gothic UI assets created/updated.")


if __name__ == "__main__":
    main()
