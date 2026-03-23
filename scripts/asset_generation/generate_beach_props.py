import unreal
import asset_utils

def generate_beach_rock(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    # Create base box to act as a rock
    try:
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            dimension_x=130.0,
            dimension_y=110.0,
            dimension_z=60.0,
            steps_x=3,
            steps_y=3,
            steps_z=3,
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

def generate_beach_chair(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # Seat
        seat_transform = unreal.Transform()
        seat_transform.translation = [0.0, 0.0, 30.0]
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=seat_transform,
            dimension_x=60.0,
            dimension_y=80.0,
            dimension_z=5.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Backrest
        backrest_transform = unreal.Transform()
        backrest_transform.translation = [-30.0, 0.0, 60.0]
        backrest_transform.rotation = unreal.Rotator(0, 0, -20).quaternion()
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=backrest_transform,
            dimension_x=5.0,
            dimension_y=80.0,
            dimension_z=60.0,
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
        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")

def main():
    base_dir = '/Game/Art/Models/Beach'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Use the existing master material created by generate_beach_assets.py
    master_mat_path = f"{mat_dir}/M_StylizedBeach"
    master_mat = asset_utils.create_master_material(master_mat_path, default_roughness=0.8, noise_scale=0.05)

    # Create Material Instances for the Rock and Chair
    rock_mat_path = f"{mat_dir}/MI_BeachRock"
    rock_mat = asset_utils.create_material_instance(rock_mat_path, master_mat, [0.4, 0.4, 0.4, 1.0], 0.9)

    chair_mat_path = f"{mat_dir}/MI_BeachChair"
    # A bright color for the beach chair, e.g., cyan/blue
    chair_mat = asset_utils.create_material_instance(chair_mat_path, master_mat, [0.1, 0.6, 0.8, 1.0], 0.6)

    # Generation
    rock_mesh_path = f"{base_dir}/SM_BeachRock"
    generate_beach_rock(rock_mesh_path, rock_mat)

    chair_mesh_path = f"{base_dir}/SM_BeachChair"
    generate_beach_chair(chair_mesh_path, chair_mat)

    print("Asset generation for new Beach props complete.")

if __name__ == '__main__':
    main()
