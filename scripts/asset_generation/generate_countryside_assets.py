import unreal
import asset_utils

def generate_farm_house(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    # Create base house box
    try:
        primitive_functions.append_box(
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
        primitive_functions.append_box(
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

    try:
        static_mesh = asset_utils.create_static_mesh_from_dynamic_mesh(dyn_mesh, mesh_path)
        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript mesh bake failed: {e}")

def generate_countryside_fence(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    # Create simple fence posts and rails
    try:
        # Post 1
        post1_transform = unreal.Transform()
        post1_transform.translation = [-100.0, 0.0, 0.0]
        primitive_functions.append_box(
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
        primitive_functions.append_box(
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
        primitive_functions.append_box(
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
        primitive_functions.append_box(
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

    try:
        static_mesh = asset_utils.create_static_mesh_from_dynamic_mesh(dyn_mesh, mesh_path)
        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")

def main():
    base_dir = '/Game/Art/Models/Countryside'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedCountryside"
    master_mat = asset_utils.create_master_material(master_mat_path)

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
