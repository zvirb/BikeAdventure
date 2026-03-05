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

def generate_urban_building(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    try:
        # Using correct UE5 Blueprint Library naming convention for Python
        dyn_mesh = unreal.GeometryScriptLibrary_CreateNewDynamicMesh.create_new_dynamic_mesh()
    except AttributeError:
        try:
            dyn_mesh = unreal.GeometryScript_AssetUtils.create_new_dynamic_mesh()
        except AttributeError:
            dyn_mesh = unreal.DynamicMesh()

    options = unreal.GeometryScriptPrimitiveOptions()
    transform = unreal.Transform()

    # Create a stylized building (a large box)
    try:
        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=transform,
                dimension_x=400.0,
                dimension_y=400.0,
                dimension_z=1000.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
            # Add a small top feature (like an elevator shaft or AC unit)
            top_transform = unreal.Transform()
            top_transform.translation = [50.0, 50.0, 1000.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=top_transform,
                dimension_x=100.0,
                dimension_y=100.0,
                dimension_z=80.0,
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
                dimension_y=400.0,
                dimension_z=1000.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
            top_transform = unreal.Transform()
            top_transform.translation = [50.0, 50.0, 1000.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=top_transform,
                dimension_x=100.0,
                dimension_y=100.0,
                dimension_z=80.0,
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

def generate_urban_bench(mesh_path, mat_inst):
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

    # Create a bench using boxes
    try:
        # Seat
        seat_transform = unreal.Transform()
        seat_transform.translation = [0.0, 0.0, 40.0]
        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=seat_transform,
                dimension_x=50.0,
                dimension_y=150.0,
                dimension_z=5.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )
            # Legs
            leg1_transform = unreal.Transform()
            leg1_transform.translation = [0.0, -60.0, 20.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=leg1_transform,
                dimension_x=5.0,
                dimension_y=5.0,
                dimension_z=40.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            leg2_transform = unreal.Transform()
            leg2_transform.translation = [0.0, 60.0, 20.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=leg2_transform,
                dimension_x=5.0,
                dimension_y=5.0,
                dimension_z=40.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )
        else:
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=seat_transform,
                dimension_x=50.0,
                dimension_y=150.0,
                dimension_z=5.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )
            # Legs
            leg1_transform = unreal.Transform()
            leg1_transform.translation = [0.0, -60.0, 20.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=leg1_transform,
                dimension_x=5.0,
                dimension_y=5.0,
                dimension_z=40.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            leg2_transform = unreal.Transform()
            leg2_transform.translation = [0.0, 60.0, 20.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=leg2_transform,
                dimension_x=5.0,
                dimension_y=5.0,
                dimension_z=40.0,
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
    base_dir = '/Game/Art/Models/Urban'
    mat_dir = '/Game/Art/Materials'

    ensure_directory(base_dir)
    ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedUrban"
    master_mat = create_master_material(master_mat_path)

    # Procedural Material Instances
    building_mat_path = f"{mat_dir}/MI_UrbanBuilding"
    building_mat = create_material_instance(building_mat_path, master_mat, [0.7, 0.7, 0.7, 1.0], 0.7)

    bench_mat_path = f"{mat_dir}/MI_UrbanBench"
    bench_mat = create_material_instance(bench_mat_path, master_mat, [0.4, 0.25, 0.1, 1.0], 0.6)

    # Programmatic 3D Modeling
    building_mesh_path = f"{base_dir}/SM_UrbanBuilding"
    generate_urban_building(building_mesh_path, building_mat)

    bench_mesh_path = f"{base_dir}/SM_UrbanBench"
    generate_urban_bench(bench_mesh_path, bench_mat)

    print("Asset generation for Urban biome complete.")

if __name__ == '__main__':
    main()
