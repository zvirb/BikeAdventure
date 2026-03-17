import unreal
import asset_utils

def generate_water_body(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    # Create base box to act as a water body
    try:
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            dimension_x=400.0,
            dimension_y=400.0,
            dimension_z=10.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )
    except Exception as e:
        print(f"GeometryScript generation failed: {e}")
        return

    try:
        static_mesh = asset_utils.create_static_mesh_from_dynamic_mesh(dyn_mesh, mesh_path)
        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")

def generate_wetlands_bridge(mesh_path, mat_inst):
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

    try:
        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            # Main walkway
            walkway_transform = unreal.Transform()
            walkway_transform.translation = [0.0, 0.0, 10.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=walkway_transform,
                dimension_x=400.0,
                dimension_y=100.0,
                dimension_z=10.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            # Left rail
            left_rail_transform = unreal.Transform()
            left_rail_transform.translation = [0.0, -45.0, 30.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=left_rail_transform,
                dimension_x=400.0,
                dimension_y=5.0,
                dimension_z=10.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            # Right rail
            right_rail_transform = unreal.Transform()
            right_rail_transform.translation = [0.0, 45.0, 30.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=right_rail_transform,
                dimension_x=400.0,
                dimension_y=5.0,
                dimension_z=10.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )
        else:
            # Main walkway
            walkway_transform = unreal.Transform()
            walkway_transform.translation = [0.0, 0.0, 10.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=walkway_transform,
                dimension_x=400.0,
                dimension_y=100.0,
                dimension_z=10.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            # Left rail
            left_rail_transform = unreal.Transform()
            left_rail_transform.translation = [0.0, -45.0, 30.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=left_rail_transform,
                dimension_x=400.0,
                dimension_y=5.0,
                dimension_z=10.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            # Right rail
            right_rail_transform = unreal.Transform()
            right_rail_transform.translation = [0.0, 45.0, 30.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=right_rail_transform,
                dimension_x=400.0,
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

def generate_marsh_plant(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        for i in range(3):
            transform = unreal.Transform()
            transform.translation = [i * 10.0 - 10.0, 0.0, 0.0]
            primitive_functions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=transform,
                radius=2.0,
                height=40.0,
                radial_steps=8,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

            # Add top cattail
            top_transform = unreal.Transform()
            top_transform.translation = [i * 10.0 - 10.0, 0.0, 30.0]
            primitive_functions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=top_transform,
                radius=4.0,
                height=10.0,
                radial_steps=8,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
    except Exception as e:
        print(f"GeometryScript generation failed: {e}")
        return

    try:
        static_mesh = asset_utils.create_static_mesh_from_dynamic_mesh(dyn_mesh, mesh_path)
        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")

def main():
    base_dir = '/Game/Art/Models/Wetlands'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedWetlands"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    water_mat_path = f"{mat_dir}/MI_WaterBody"
    water_mat = create_material_instance(water_mat_path, master_mat, [0.1, 0.3, 0.7, 1.0], 0.1) # Blue, low roughness

    plant_mat_path = f"{mat_dir}/MI_MarshPlant"
    plant_mat = create_material_instance(plant_mat_path, master_mat, [0.2, 0.5, 0.2, 1.0], 0.8) # Green, high roughness

    # Programmatic 3D Modeling
    water_mesh_path = f"{base_dir}/SM_WaterBody"
    generate_water_body(water_mesh_path, water_mat)

    plant_mesh_path = f"{base_dir}/SM_MarshPlant"
    generate_marsh_plant(plant_mesh_path, plant_mat)

    bridge_mat_path = f"{mat_dir}/MI_WetlandsBridge"
    bridge_mat = create_material_instance(bridge_mat_path, master_mat, [0.3, 0.2, 0.1, 1.0], 0.8) # Brown, rough wood

    bridge_mesh_path = f"{base_dir}/SM_WetlandsBridge"
    generate_wetlands_bridge(bridge_mesh_path, bridge_mat)

    print("Asset generation for Wetlands biome complete.")

if __name__ == '__main__':
    main()
