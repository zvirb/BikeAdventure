import unreal
import asset_utils

def generate_urban_building(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    # Create a stylized building (a large box)
    try:
        primitive_functions.append_box(
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
        primitive_functions.append_box(
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

    try:
        static_mesh = asset_utils.create_static_mesh_from_dynamic_mesh(dyn_mesh, mesh_path)
        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript mesh bake failed: {e}")

def generate_urban_bench(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    # Create a bench using boxes
    try:
        # Seat
        seat_transform = unreal.Transform()
        seat_transform.translation = [0.0, 0.0, 40.0]
        primitive_functions.append_box(
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
        primitive_functions.append_box(
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
        primitive_functions.append_box(
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

    try:
        static_mesh = asset_utils.create_static_mesh_from_dynamic_mesh(dyn_mesh, mesh_path)
        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")

def generate_urban_traffic_light(mesh_path, mat_inst):
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
        # Post
        post_transform = unreal.Transform()
        post_transform.translation = [0.0, 0.0, 150.0]
        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=post_transform,
                radius=10.0,
                height=300.0,
                radial_steps=8,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )
            # Box head
            head_transform = unreal.Transform()
            head_transform.translation = [15.0, 0.0, 260.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=head_transform,
                dimension_x=20.0,
                dimension_y=40.0,
                dimension_z=100.0,
                steps_x=1,
                steps_y=1,
                steps_z=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )
        else:
            unreal.GeometryScript_MeshPrimitiveFunctions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=post_transform,
                radius=10.0,
                height=300.0,
                radial_steps=8,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )
            # Box head
            head_transform = unreal.Transform()
            head_transform.translation = [15.0, 0.0, 260.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=head_transform,
                dimension_x=20.0,
                dimension_y=40.0,
                dimension_z=100.0,
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


def generate_urban_park_tree(mesh_path, mat_inst):
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
            # Trunk
            trunk_transform = unreal.Transform()
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=trunk_transform,
                radius=15.0,
                height=150.0,
                radial_steps=8,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
            # Leaves
            leaves_transform = unreal.Transform()
            leaves_transform.translation = [0.0, 0.0, 120.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_sphere(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=leaves_transform,
                radius=100.0,
                steps_x=12,
                steps_y=12,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
        else:
            # Trunk
            trunk_transform = unreal.Transform()
            unreal.GeometryScript_MeshPrimitiveFunctions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=trunk_transform,
                radius=15.0,
                height=150.0,
                radial_steps=8,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
            # Leaves
            leaves_transform = unreal.Transform()
            leaves_transform.translation = [0.0, 0.0, 120.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_sphere(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=leaves_transform,
                radius=100.0,
                steps_x=12,
                steps_y=12,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
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

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedUrban"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    building_mat_path = f"{mat_dir}/MI_UrbanBuilding"
    building_mat = asset_utils.create_material_instance(building_mat_path, master_mat, [0.7, 0.7, 0.7, 1.0], 0.7)

    bench_mat_path = f"{mat_dir}/MI_UrbanBench"
    bench_mat = asset_utils.create_material_instance(bench_mat_path, master_mat, [0.4, 0.25, 0.1, 1.0], 0.6)

    traffic_light_mat_path = f"{mat_dir}/MI_UrbanTrafficLight"
    traffic_light_mat = asset_utils.create_material_instance(traffic_light_mat_path, master_mat, [0.1, 0.1, 0.1, 1.0], 0.3)

    park_tree_mat_path = f"{mat_dir}/MI_UrbanParkTree"
    park_tree_mat = asset_utils.create_material_instance(park_tree_mat_path, master_mat, [0.1, 0.5, 0.1, 1.0], 0.8)

    # Programmatic 3D Modeling
    building_mesh_path = f"{base_dir}/SM_UrbanBuilding"
    generate_urban_building(building_mesh_path, building_mat)

    bench_mesh_path = f"{base_dir}/SM_UrbanBench"
    generate_urban_bench(bench_mesh_path, bench_mat)

    traffic_light_mesh_path = f"{base_dir}/SM_UrbanTrafficLight"
    generate_urban_traffic_light(traffic_light_mesh_path, traffic_light_mat)

    park_tree_mesh_path = f"{base_dir}/SM_UrbanParkTree"
    generate_urban_park_tree(park_tree_mesh_path, park_tree_mat)

    print("Asset generation for Urban biome complete.")

if __name__ == '__main__':
    main()
