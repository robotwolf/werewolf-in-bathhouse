import unreal

MATERIALS_PATH = "/Game/WerewolfBH/Materials/Assembler"


def load_or_create_asset(asset_path, asset_class, factory):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset:
            return asset, False

    package_path, asset_name = asset_path.rsplit('/', 1)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=asset_name,
        package_path=package_path,
        asset_class=asset_class,
        factory=factory,
    )
    return asset, True


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def clear_material_expressions(material):
    unreal.MaterialEditingLibrary.delete_all_material_expressions(material)


def add_expression(material, expr_class, x, y):
    return unreal.MaterialEditingLibrary.create_material_expression(material, expr_class, x, y)


def compile_layout_save(material):
    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)


def build_flat_material(asset_name, base_color, roughness=0.9, specular=0.25, metallic=0.0):
    asset_path = f"{MATERIALS_PATH}/{asset_name}"
    material, _ = load_or_create_asset(asset_path, unreal.Material, unreal.MaterialFactoryNew())

    material.set_editor_property('blend_mode', unreal.BlendMode.BLEND_OPAQUE)
    material.set_editor_property('two_sided', False)
    clear_material_expressions(material)

    color = add_expression(material, unreal.MaterialExpressionVectorParameter, -600, -120)
    color.set_editor_property('parameter_name', 'BaseColor')
    color.set_editor_property('default_value', base_color)

    rough = add_expression(material, unreal.MaterialExpressionScalarParameter, -600, 80)
    rough.set_editor_property('parameter_name', 'Roughness')
    rough.set_editor_property('default_value', roughness)

    spec = add_expression(material, unreal.MaterialExpressionScalarParameter, -600, 240)
    spec.set_editor_property('parameter_name', 'Specular')
    spec.set_editor_property('default_value', specular)

    metal = add_expression(material, unreal.MaterialExpressionScalarParameter, -600, 400)
    metal.set_editor_property('parameter_name', 'Metallic')
    metal.set_editor_property('default_value', metallic)

    unreal.MaterialEditingLibrary.connect_material_property(color, '', unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(rough, '', unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.connect_material_property(spec, '', unreal.MaterialProperty.MP_SPECULAR)
    unreal.MaterialEditingLibrary.connect_material_property(metal, '', unreal.MaterialProperty.MP_METALLIC)

    compile_layout_save(material)
    unreal.log(f'[create_assembler_test_materials] Wrote {asset_path}')
    return material


def main():
    ensure_folder(MATERIALS_PATH)
    build_flat_material('M_Assembler_Test_Floor', unreal.LinearColor(0.09, 0.17, 0.12, 1.0), roughness=0.95, specular=0.12)
    build_flat_material('M_Assembler_Test_Wall', unreal.LinearColor(0.56, 0.54, 0.48, 1.0), roughness=0.88, specular=0.20)
    build_flat_material('M_Assembler_Test_Ceiling', unreal.LinearColor(0.66, 0.70, 0.74, 1.0), roughness=0.82, specular=0.18)
    build_flat_material('M_Assembler_Test_Accent', unreal.LinearColor(0.42, 0.16, 0.10, 1.0), roughness=0.75, specular=0.22)
    unreal.log('[create_assembler_test_materials] Material setup complete.')


if __name__ == '__main__':
    main()
