import unreal

def ensure_directory(path):
    """Ensures that the given directory exists in the Unreal Editor."""
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

def create_master_material(mat_path, default_roughness=0.5, noise_scale=0.1):
    """Creates a stylized master material with base color, roughness, and noise."""
    if unreal.EditorAssetLibrary.does_asset_exist(mat_path):
        return unreal.EditorAssetLibrary.load_asset(mat_path)

    mat_factory = unreal.MaterialFactoryNew()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    pkg_path = '/'.join(mat_path.split('/')[:-1])
    asset_name = mat_path.split('/')[-1]

    mat = asset_tools.create_asset(
        asset_name=asset_name,
        package_path=pkg_path,
        asset_class=unreal.Material,
        factory=mat_factory
    )

    if not mat:
        return None

    # Base Color Parameter
    base_color_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionVectorParameter, -400, 0
    )
    base_color_node.set_editor_property("parameter_name", "BaseColor")
    base_color_node.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))

    # Roughness Parameter
    roughness_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionScalarParameter, -400, 200
    )
    roughness_node.set_editor_property("parameter_name", "Roughness")
    roughness_node.set_editor_property("default_value", default_roughness)

    # Noise for stylized feel
    noise_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionNoise, -400, -200
    )
    noise_node.set_editor_property("scale", noise_scale)

    multiply_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionMultiply, -200, 0
    )

    unreal.MaterialEditingLibrary.connect_material_expressions(
        base_color_node, "", multiply_node, "A"
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(
        noise_node, "", multiply_node, "B"
    )

    unreal.MaterialEditingLibrary.connect_material_property(
        multiply_node, "", unreal.MaterialProperty.MP_BASE_COLOR
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness_node, "", unreal.MaterialProperty.MP_ROUGHNESS
    )

    unreal.MaterialEditingLibrary.recompile_material(mat)
    return mat

def create_material_instance(mat_inst_path, parent_mat, color, roughness):
    """Creates a material instance from a parent material."""
    if unreal.EditorAssetLibrary.does_asset_exist(mat_inst_path):
        mat_inst = unreal.EditorAssetLibrary.load_asset(mat_inst_path)
    else:
        mat_inst_factory = unreal.MaterialInstanceConstantFactoryNew()
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

        pkg_path = '/'.join(mat_inst_path.split('/')[:-1])
        asset_name = mat_inst_path.split('/')[-1]

        mat_inst = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=pkg_path,
            asset_class=unreal.MaterialInstanceConstant,
            factory=mat_inst_factory
        )

        if mat_inst and parent_mat:
            unreal.MaterialEditingLibrary.set_material_instance_parent(mat_inst, parent_mat)

    if mat_inst:
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            mat_inst, "BaseColor", unreal.LinearColor(color[0], color[1], color[2], color[3])
        )
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            mat_inst, "Roughness", roughness
        )
        unreal.MaterialEditingLibrary.update_material_instance(mat_inst)

    return mat_inst

def assign_material_to_mesh(static_mesh, material_instance):
    """Assigns a material instance to the first material slot of a static mesh."""
    if not static_mesh or not material_instance:
        return

    materials = static_mesh.get_editor_property('static_materials')

    if not materials or len(materials) == 0:
        new_mat = unreal.StaticMaterial()
        new_mat.material_interface = material_instance
        materials = [new_mat]
    else:
        materials[0].material_interface = material_instance

    static_mesh.set_editor_property('static_materials', materials)

def create_dynamic_mesh():
    """Creates a new dynamic mesh with multi-level fallback."""
    try:
        return unreal.GeometryScriptLibrary_CreateNewDynamicMesh.create_new_dynamic_mesh()
    except AttributeError:
        try:
            return unreal.GeometryScript_AssetUtils.create_new_dynamic_mesh()
        except AttributeError:
            return unreal.DynamicMesh()

def get_geometry_script_primitive_functions():
    """Returns the GeometryScript primitive functions library."""
    if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
        return unreal.GeometryScriptLibrary_MeshPrimitiveFunctions
    else:
        return unreal.GeometryScript_MeshPrimitiveFunctions

def create_static_mesh_from_dynamic_mesh(dyn_mesh, mesh_path):
    """Creates a static mesh asset from a dynamic mesh."""
    create_options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    if hasattr(unreal, "GeometryScriptLibrary_CreateNewStaticMeshAssetFromDynamicMesh"):
        return unreal.GeometryScriptLibrary_CreateNewStaticMeshAssetFromDynamicMesh.create_new_static_mesh_asset_from_dynamic_mesh(
            dynamic_mesh=dyn_mesh,
            asset_path_and_name=mesh_path,
            options=create_options
        )
    else:
        return unreal.GeometryScript_AssetUtils.create_new_static_mesh_asset_from_dynamic_mesh(
            dynamic_mesh=dyn_mesh,
            asset_path_and_name=mesh_path,
            options=create_options
        )
