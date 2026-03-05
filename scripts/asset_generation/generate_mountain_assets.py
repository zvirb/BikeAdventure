import unreal
import sys

def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

def create_master_material(mat_path):
    if unreal.EditorAssetLibrary.does_asset_exist(mat_path):
        return unreal.EditorAssetLibrary.load_asset(mat_path)

    mat_factory = unreal.MaterialFactoryNew()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # Extract package path and asset name
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
    roughness_node.set_editor_property("default_value", 0.5)

    # Simple Noise Node to add stylized gradient/noise feel
    noise_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionNoise, -400, -200
    )
    noise_node.set_editor_property("scale", 0.1)

    # Multiply Noise with BaseColor
    multiply_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionMultiply, -200, 0
    )

    unreal.MaterialEditingLibrary.connect_material_property(
        base_color_node, "RGB", unreal.MaterialProperty.MP_BASE_COLOR
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness_node, "", unreal.MaterialProperty.MP_ROUGHNESS
    )

    unreal.MaterialEditingLibrary.recompile_material(mat)
    return mat

def create_material_instance(mat_inst_path, parent_mat, color, roughness):
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
    if not static_mesh or not material_instance:
        return

    # Standard approach to set material on a static mesh in UE5 Python
    materials = static_mesh.get_editor_property('static_materials')

    if not materials or len(materials) == 0:
        # Create a new element
        new_mat = unreal.StaticMaterial()
        new_mat.material_interface = material_instance
        materials = [new_mat]
    else:
        # Update first element
        materials[0].material_interface = material_instance

    static_mesh.set_editor_property('static_materials', materials)

def generate_alpine_plant(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    # In UE5 Geometry Script, Blueprint Function Libraries translate to snake_case module functions
    # For primitives, it is usually unreal.GeometryScript_MeshPrimitiveFunctions
    try:
        dyn_mesh = unreal.GeometryScript_AssetUtils.create_new_dynamic_mesh()
    except AttributeError:
        # Fallback to direct instantiation if the function is not available
        dyn_mesh = unreal.DynamicMesh()

    options = unreal.GeometryScriptPrimitiveOptions()
    transform = unreal.Transform()

    # Add trunk
    try:
        unreal.GeometryScript_MeshPrimitiveFunctions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            radius=10.0,
            height=50.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Add leaves layers
        leaf_transform = unreal.Transform()
        leaf_transform.translation = [0, 0, 40.0]
        unreal.GeometryScript_MeshPrimitiveFunctions.append_cone(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=leaf_transform,
            base_radius=40.0,
            top_radius=0.0,
            height=60.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        leaf_transform.translation = [0, 0, 80.0]
        unreal.GeometryScript_MeshPrimitiveFunctions.append_cone(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=leaf_transform,
            base_radius=30.0,
            top_radius=0.0,
            height=50.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )
    except Exception as e:
        print(f"GeometryScript generation failed (perhaps plugin not enabled): {e}")
        return

    # Save Dynamic Mesh to Static Mesh Asset
    create_options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    try:
        static_mesh = unreal.GeometryScript_AssetUtils.create_new_static_mesh_asset_from_dynamic_mesh(
            dynamic_mesh=dyn_mesh,
            asset_path_and_name=mesh_path,
            options=create_options
        )

        # Apply material
        assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript mesh bake failed: {e}")


def generate_mountain_rock(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    try:
        dyn_mesh = unreal.GeometryScript_AssetUtils.create_new_dynamic_mesh()
    except AttributeError:
        dyn_mesh = unreal.DynamicMesh()

    options = unreal.GeometryScriptPrimitiveOptions()
    transform = unreal.Transform()

    # Create base box to act as a rock
    try:
        unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            dimension_x=100.0,
            dimension_y=120.0,
            dimension_z=80.0,
            steps_x=3,
            steps_y=3,
            steps_z=3,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Apply some noise for organic look
        perlin_options = unreal.GeometryScript3DGridNeuralNetworkOptions() # Placeholder for options if needed, but not required
        # Note: Actual perlin noise function might be GeometryScript_MeshModifiers.apply_perlin_noise_to_mesh
        # We will keep it low-poly simple for now.
    except Exception as e:
        print(f"GeometryScript generation failed: {e}")
        return

    create_options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    try:
        static_mesh = unreal.GeometryScript_AssetUtils.create_new_static_mesh_asset_from_dynamic_mesh(
            dynamic_mesh=dyn_mesh,
            asset_path_and_name=mesh_path,
            options=create_options
        )

        # Apply material
        assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")


def main():
    base_dir = '/Game/Art/Models/Mountains'
    mat_dir = '/Game/Art/Materials'

    ensure_directory(base_dir)
    ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedMaster"
    master_mat = create_master_material(master_mat_path)

    # Procedural Material Instances
    plant_mat_path = f"{mat_dir}/MI_AlpinePlant"
    plant_mat = create_material_instance(plant_mat_path, master_mat, [0.1, 0.4, 0.1, 1.0], 0.8)

    rock_mat_path = f"{mat_dir}/MI_MountainRock"
    rock_mat = create_material_instance(rock_mat_path, master_mat, [0.3, 0.3, 0.3, 1.0], 0.9)

    # Programmatic 3D Modeling
    plant_mesh_path = f"{base_dir}/SM_AlpinePlant"
    generate_alpine_plant(plant_mesh_path, plant_mat)

    rock_mesh_path = f"{base_dir}/SM_MountainRock"
    generate_mountain_rock(rock_mesh_path, rock_mat)

    print("Asset generation for Mountains biome complete.")

if __name__ == '__main__':
    main()
