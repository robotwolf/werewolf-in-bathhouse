import unreal


PROJECT_MATERIALS = "/Game/UI/Materials"
PROJECT_INSTANCES = "/Game/UI/Materials/Instances"


def load_or_create_asset(asset_path, asset_class, factory):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset:
        return asset, False

    package_path, asset_name = asset_path.rsplit("/", 1)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=asset_name,
        package_path=package_path,
        asset_class=asset_class,
        factory=factory,
    )
    return asset, True


def clear_material_expressions(material):
    unreal.MaterialEditingLibrary.delete_all_material_expressions(material)


def add_expression(material, expr_class, x, y):
    expr = unreal.MaterialEditingLibrary.create_material_expression(material, expr_class, x, y)
    return expr


def set_param_name(expr, name):
    expr.set_editor_property("parameter_name", name)


def compile_layout_save(material):
    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)


def build_iris_material():
    asset_path = f"{PROJECT_MATERIALS}/M_UI_WipeIris"
    material, _ = load_or_create_asset(asset_path, unreal.Material, unreal.MaterialFactoryNew())

    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_UI)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_ALPHA_COMPOSITE)
    clear_material_expressions(material)

    texcoord = add_expression(material, unreal.MaterialExpressionTextureCoordinate, -1400, -200)
    center_x = add_expression(material, unreal.MaterialExpressionScalarParameter, -1400, -20)
    center_y = add_expression(material, unreal.MaterialExpressionScalarParameter, -1400, 140)
    append_center = add_expression(material, unreal.MaterialExpressionAppendVector, -1180, 60)
    subtract_center = add_expression(material, unreal.MaterialExpressionSubtract, -980, -70)

    mask_r = add_expression(material, unreal.MaterialExpressionComponentMask, -760, -170)
    mask_r.set_editor_property("r", True)
    mask_g = add_expression(material, unreal.MaterialExpressionComponentMask, -760, 10)
    mask_g.set_editor_property("g", True)

    square_r = add_expression(material, unreal.MaterialExpressionMultiply, -540, -170)
    square_g = add_expression(material, unreal.MaterialExpressionMultiply, -540, 10)
    add_len = add_expression(material, unreal.MaterialExpressionAdd, -330, -70)
    sqrt_len = add_expression(material, unreal.MaterialExpressionSquareRoot, -130, -70)

    radius = add_expression(material, unreal.MaterialExpressionScalarParameter, -540, 220)
    softness = add_expression(material, unreal.MaterialExpressionScalarParameter, -540, 360)
    opacity = add_expression(material, unreal.MaterialExpressionScalarParameter, -540, 500)

    subtract_radius = add_expression(material, unreal.MaterialExpressionSubtract, 90, -70)
    divide_softness = add_expression(material, unreal.MaterialExpressionDivide, 320, -70)
    saturate_alpha = add_expression(material, unreal.MaterialExpressionSaturate, 530, -70)
    final_alpha = add_expression(material, unreal.MaterialExpressionMultiply, 760, -70)

    tint = add_expression(material, unreal.MaterialExpressionVectorParameter, 760, 180)

    set_param_name(center_x, "CenterX")
    center_x.set_editor_property("default_value", 0.5)
    set_param_name(center_y, "CenterY")
    center_y.set_editor_property("default_value", 0.5)
    set_param_name(radius, "Radius")
    radius.set_editor_property("default_value", 0.9)
    set_param_name(softness, "Softness")
    softness.set_editor_property("default_value", 0.08)
    set_param_name(opacity, "Opacity")
    opacity.set_editor_property("default_value", 1.0)
    set_param_name(tint, "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(0.0, 0.0, 0.0, 1.0))

    unreal.MaterialEditingLibrary.connect_material_expressions(center_x, "", append_center, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(center_y, "", append_center, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(texcoord, "", subtract_center, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(append_center, "", subtract_center, "B")

    unreal.MaterialEditingLibrary.connect_material_expressions(subtract_center, "", mask_r, "")
    unreal.MaterialEditingLibrary.connect_material_expressions(subtract_center, "", mask_g, "")

    unreal.MaterialEditingLibrary.connect_material_expressions(mask_r, "", square_r, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(mask_r, "", square_r, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(mask_g, "", square_g, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(mask_g, "", square_g, "B")

    unreal.MaterialEditingLibrary.connect_material_expressions(square_r, "", add_len, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(square_g, "", add_len, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(add_len, "", sqrt_len, "")

    unreal.MaterialEditingLibrary.connect_material_expressions(sqrt_len, "", subtract_radius, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(radius, "", subtract_radius, "B")

    unreal.MaterialEditingLibrary.connect_material_expressions(subtract_radius, "", divide_softness, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(softness, "", divide_softness, "B")

    unreal.MaterialEditingLibrary.connect_material_expressions(divide_softness, "", saturate_alpha, "")
    unreal.MaterialEditingLibrary.connect_material_expressions(saturate_alpha, "", final_alpha, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(opacity, "", final_alpha, "B")

    unreal.MaterialEditingLibrary.connect_material_property(
        tint,
        "",
        unreal.MaterialProperty.MP_EMISSIVE_COLOR,
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        final_alpha,
        "",
        unreal.MaterialProperty.MP_OPACITY,
    )

    compile_layout_save(material)
    return material


def build_steam_material():
    asset_path = f"{PROJECT_MATERIALS}/M_UI_WipeSteam"
    material, _ = load_or_create_asset(asset_path, unreal.Material, unreal.MaterialFactoryNew())

    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_UI)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_ALPHA_COMPOSITE)
    clear_material_expressions(material)

    texcoord = add_expression(material, unreal.MaterialExpressionTextureCoordinate, -1600, -200)
    mask_u = add_expression(material, unreal.MaterialExpressionComponentMask, -1380, -280)
    mask_u.set_editor_property("r", True)
    mask_v = add_expression(material, unreal.MaterialExpressionComponentMask, -1380, -80)
    mask_v.set_editor_property("g", True)

    frequency = add_expression(material, unreal.MaterialExpressionScalarParameter, -1380, 120)
    speed = add_expression(material, unreal.MaterialExpressionScalarParameter, -1380, 280)
    time_node = add_expression(material, unreal.MaterialExpressionTime, -1380, 430)
    distortion = add_expression(material, unreal.MaterialExpressionScalarParameter, -1380, 560)

    mul_v_freq = add_expression(material, unreal.MaterialExpressionMultiply, -1130, 10)
    mul_time_speed = add_expression(material, unreal.MaterialExpressionMultiply, -1130, 220)
    add_wave_phase = add_expression(material, unreal.MaterialExpressionAdd, -900, 90)
    sine_wave = add_expression(material, unreal.MaterialExpressionSine, -680, 90)
    mul_distortion = add_expression(material, unreal.MaterialExpressionMultiply, -480, 90)
    add_distorted_u = add_expression(material, unreal.MaterialExpressionAdd, -260, -90)

    edge_position = add_expression(material, unreal.MaterialExpressionScalarParameter, -480, 320)
    edge_softness = add_expression(material, unreal.MaterialExpressionScalarParameter, -480, 470)
    opacity = add_expression(material, unreal.MaterialExpressionScalarParameter, -480, 620)
    tint = add_expression(material, unreal.MaterialExpressionVectorParameter, -260, 620)

    subtract_edge = add_expression(material, unreal.MaterialExpressionSubtract, -30, -90)
    divide_softness = add_expression(material, unreal.MaterialExpressionDivide, 210, -90)
    saturate_alpha = add_expression(material, unreal.MaterialExpressionSaturate, 450, -90)
    final_alpha = add_expression(material, unreal.MaterialExpressionMultiply, 700, -90)

    set_param_name(frequency, "WaveFrequency")
    frequency.set_editor_property("default_value", 22.0)
    set_param_name(speed, "WaveSpeed")
    speed.set_editor_property("default_value", 0.35)
    set_param_name(distortion, "WaveDistortion")
    distortion.set_editor_property("default_value", 0.035)
    set_param_name(edge_position, "EdgePosition")
    edge_position.set_editor_property("default_value", 0.25)
    set_param_name(edge_softness, "EdgeSoftness")
    edge_softness.set_editor_property("default_value", 0.18)
    set_param_name(opacity, "Opacity")
    opacity.set_editor_property("default_value", 0.85)
    set_param_name(tint, "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(0.84, 0.86, 0.85, 1.0))

    unreal.MaterialEditingLibrary.connect_material_expressions(texcoord, "", mask_u, "")
    unreal.MaterialEditingLibrary.connect_material_expressions(texcoord, "", mask_v, "")

    unreal.MaterialEditingLibrary.connect_material_expressions(mask_v, "", mul_v_freq, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(frequency, "", mul_v_freq, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(time_node, "", mul_time_speed, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(speed, "", mul_time_speed, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(mul_v_freq, "", add_wave_phase, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(mul_time_speed, "", add_wave_phase, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(add_wave_phase, "", sine_wave, "")
    unreal.MaterialEditingLibrary.connect_material_expressions(sine_wave, "", mul_distortion, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(distortion, "", mul_distortion, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(mask_u, "", add_distorted_u, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(mul_distortion, "", add_distorted_u, "B")

    unreal.MaterialEditingLibrary.connect_material_expressions(add_distorted_u, "", subtract_edge, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(edge_position, "", subtract_edge, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(subtract_edge, "", divide_softness, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(edge_softness, "", divide_softness, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(divide_softness, "", saturate_alpha, "")
    unreal.MaterialEditingLibrary.connect_material_expressions(saturate_alpha, "", final_alpha, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(opacity, "", final_alpha, "B")

    unreal.MaterialEditingLibrary.connect_material_property(
        tint,
        "",
        unreal.MaterialProperty.MP_EMISSIVE_COLOR,
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        final_alpha,
        "",
        unreal.MaterialProperty.MP_OPACITY,
    )

    compile_layout_save(material)
    return material


def load_or_create_instance(asset_path, parent_material):
    instance, _ = load_or_create_asset(asset_path, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    instance.set_editor_property("parent", parent_material)
    unreal.EditorAssetLibrary.save_loaded_asset(instance)
    return instance


def set_scalar(instance, name, value):
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(instance, name, value)


def set_vector(instance, name, value):
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(instance, name, value)


def build_instances(iris_material, steam_material):
    iris_instance = load_or_create_instance(f"{PROJECT_INSTANCES}/MI_UI_WipeIris_Default", iris_material)
    set_scalar(iris_instance, "CenterX", 0.5)
    set_scalar(iris_instance, "CenterY", 0.5)
    set_scalar(iris_instance, "Radius", 0.9)
    set_scalar(iris_instance, "Softness", 0.08)
    set_scalar(iris_instance, "Opacity", 1.0)
    set_vector(iris_instance, "Tint", unreal.LinearColor(0.0, 0.0, 0.0, 1.0))
    unreal.EditorAssetLibrary.save_loaded_asset(iris_instance)

    steam_front = load_or_create_instance(f"{PROJECT_INSTANCES}/MI_UI_WipeSteam_Front", steam_material)
    set_scalar(steam_front, "WaveFrequency", 24.0)
    set_scalar(steam_front, "WaveSpeed", 0.40)
    set_scalar(steam_front, "WaveDistortion", 0.04)
    set_scalar(steam_front, "EdgePosition", 0.18)
    set_scalar(steam_front, "EdgeSoftness", 0.22)
    set_scalar(steam_front, "Opacity", 0.72)
    set_vector(steam_front, "Tint", unreal.LinearColor(0.88, 0.90, 0.89, 1.0))
    unreal.EditorAssetLibrary.save_loaded_asset(steam_front)

    steam_back = load_or_create_instance(f"{PROJECT_INSTANCES}/MI_UI_WipeSteam_Back", steam_material)
    set_scalar(steam_back, "WaveFrequency", 18.0)
    set_scalar(steam_back, "WaveSpeed", -0.22)
    set_scalar(steam_back, "WaveDistortion", 0.025)
    set_scalar(steam_back, "EdgePosition", 0.10)
    set_scalar(steam_back, "EdgeSoftness", 0.28)
    set_scalar(steam_back, "Opacity", 0.45)
    set_vector(steam_back, "Tint", unreal.LinearColor(0.70, 0.73, 0.72, 1.0))
    unreal.EditorAssetLibrary.save_loaded_asset(steam_back)


def main():
    iris_material = build_iris_material()
    steam_material = build_steam_material()
    build_instances(iris_material, steam_material)
    unreal.log("Created or updated UI wipe materials and instances.")


if __name__ == "__main__":
    main()
