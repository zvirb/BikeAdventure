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

    # Connect BaseColor and Noise to Multiply
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

def generate_farm_house(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    try:
        dyn_mesh = unreal.GeometryScriptLibrary_CreateNewDynamicMesh.create_new_dynamic_mesh()
    except AttributeError:
        try:
            dyn_mesh = unreal.GeometryScript_AssetUtils.create_new_dynamic_mesh()
        except AttributeError:
            dyn_mesh = unreal.DynamicMesh()

    options = unreal.GeometryScriptPrimitiveOptions()
    transform = unreal.Transform()

    # Create base house box
    try:
        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=transform,
                dimension_x=400.0,
                dimension_y=300.0,
                dimension_z=200.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

            # Add roof (a wedge/cone/box depending on availability, using box rotated 45deg as simple roof)
            roof_transform = unreal.Transform()
            roof_transform.translation = [0, 0, 200.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=roof_transform,
                dimension_x=420.0,
                dimension_y=320.0,
                dimension_z=50.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
        else:
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=transform,
                dimension_x=400.0,
                dimension_y=300.0,
                dimension_z=200.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

            roof_transform = unreal.Transform()
            roof_transform.translation = [0, 0, 200.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=roof_transform,
                dimension_x=420.0,
                dimension_y=320.0,
                dimension_z=50.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

    except Exception as e:
        print(f"GeometryScript generation failed (perhaps plugin not enabled): {e}")
        return

    # Save Dynamic Mesh to Static Mesh Asset
    create_options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    try:
        if hasattr(unreal, "GeometryScriptLibrary_CreateNewStaticMeshAssetFromDynamicMesh"):
            static_mesh = unreal.GeometryScriptLibrary_CreateNewStaticMeshAssetFromDynamicMesh.create_new_static_mesh_asset_from_dynamic_mesh(
                dynamic_mesh=dyn_mesh,
                asset_path_and_name=mesh_path,
                options=create_options
            )
        else:
            static_mesh = unreal.GeometryScript_AssetUtils.create_new_static_mesh_asset_from_dynamic_mesh(
                dynamic_mesh=dyn_mesh,
                asset_path_and_name=mesh_path,
                options=create_options
            )

        # Apply material
        assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript mesh bake failed: {e}")

def generate_countryside_fence(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    try:
        dyn_mesh = unreal.GeometryScriptLibrary_CreateNewDynamicMesh.create_new_dynamic_mesh()
    except AttributeError:
        try:
            dyn_mesh = unreal.GeometryScript_AssetUtils.create_new_dynamic_mesh()
        except AttributeError:
            dyn_mesh = unreal.DynamicMesh()

    options = unreal.GeometryScriptPrimitiveOptions()

    # Create simple fence posts and rails
    try:
        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            # Post 1
            post1_transform = unreal.Transform()
            post1_transform.translation = [-100.0, 0.0, 0.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=post1_transform,
                dimension_x=10.0,
                dimension_y=10.0,
                dimension_z=100.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

            # Post 2
            post2_transform = unreal.Transform()
            post2_transform.translation = [100.0, 0.0, 0.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=post2_transform,
                dimension_x=10.0,
                dimension_y=10.0,
                dimension_z=100.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

            # Rail 1
            rail1_transform = unreal.Transform()
            rail1_transform.translation = [0.0, 0.0, 30.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=rail1_transform,
                dimension_x=220.0,
                dimension_y=5.0,
                dimension_z=10.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            # Rail 2
            rail2_transform = unreal.Transform()
            rail2_transform.translation = [0.0, 0.0, 80.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=rail2_transform,
                dimension_x=220.0,
                dimension_y=5.0,
                dimension_z=10.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )
        else:
            post1_transform = unreal.Transform()
            post1_transform.translation = [-100.0, 0.0, 0.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=post1_transform,
                dimension_x=10.0,
                dimension_y=10.0,
                dimension_z=100.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

            post2_transform = unreal.Transform()
            post2_transform.translation = [100.0, 0.0, 0.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=post2_transform,
                dimension_x=10.0,
                dimension_y=10.0,
                dimension_z=100.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

            rail1_transform = unreal.Transform()
            rail1_transform.translation = [0.0, 0.0, 30.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=rail1_transform,
                dimension_x=220.0,
                dimension_y=5.0,
                dimension_z=10.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            rail2_transform = unreal.Transform()
            rail2_transform.translation = [0.0, 0.0, 80.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=rail2_transform,
                dimension_x=220.0,
                dimension_y=5.0,
                dimension_z=10.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )
    except Exception as e:
        print(f"GeometryScript generation failed: {e}")
        return

    create_options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    try:
        if hasattr(unreal, "GeometryScriptLibrary_CreateNewStaticMeshAssetFromDynamicMesh"):
            static_mesh = unreal.GeometryScriptLibrary_CreateNewStaticMeshAssetFromDynamicMesh.create_new_static_mesh_asset_from_dynamic_mesh(
                dynamic_mesh=dyn_mesh,
                asset_path_and_name=mesh_path,
                options=create_options
            )
        else:
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
    base_dir = '/Game/Art/Models/Countryside'
    mat_dir = '/Game/Art/Materials'

    ensure_directory(base_dir)
    ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedCountryside"
    master_mat = create_master_material(master_mat_path)

    # Procedural Material Instances
    house_mat_path = f"{mat_dir}/MI_FarmHouse"
    house_mat = create_material_instance(house_mat_path, master_mat, [0.8, 0.2, 0.2, 1.0], 0.8) # Red barn-like color

    fence_mat_path = f"{mat_dir}/MI_CountrysideFence"
    fence_mat = create_material_instance(fence_mat_path, master_mat, [0.4, 0.2, 0.1, 1.0], 0.9) # Brown wood color

    # Programmatic 3D Modeling
    house_mesh_path = f"{base_dir}/SM_FarmHouse"
    generate_farm_house(house_mesh_path, house_mat)

    fence_mesh_path = f"{base_dir}/SM_CountrysideFence"
    generate_countryside_fence(fence_mesh_path, fence_mat)

    print("Asset generation for Countryside biome complete.")

if __name__ == '__main__':
    main()
