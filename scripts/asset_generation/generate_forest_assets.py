import unreal
import asset_utils

def generate_pine_tree(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    try:
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            radius=15.0,
            height=150.0,
            radial_steps=6,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        leaf_transform = unreal.Transform()
        leaf_transform.translation = [0, 0, 80.0]
        primitive_functions.append_cone(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=leaf_transform,
            base_radius=60.0,
            top_radius=0.0,
            height=150.0,
            radial_steps=6,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        leaf_transform.translation = [0, 0, 140.0]
        primitive_functions.append_cone(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=leaf_transform,
            base_radius=50.0,
            top_radius=0.0,
            height=120.0,
            radial_steps=6,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        leaf_transform.translation = [0, 0, 190.0]
        primitive_functions.append_cone(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=leaf_transform,
            base_radius=35.0,
            top_radius=0.0,
            height=100.0,
            radial_steps=6,
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
        print(f"GeometryScript mesh bake failed: {e}")


def generate_forest_rock(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    try:
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            dimension_x=120.0,
            dimension_y=100.0,
            dimension_z=50.0,
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


def generate_forest_log(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    transform = unreal.Transform()
    # Rotate cylinder to lie horizontally
    transform.rotation = unreal.Rotator(0, 0, 90).quaternion()

    try:
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            radius=20.0,
            height=150.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
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
    base_dir = '/Game/Art/Models/Forest'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    master_mat_path = f"{mat_dir}/M_StylizedForest"
    master_mat = asset_utils.create_master_material(master_mat_path)

    pine_mat_path = f"{mat_dir}/MI_PineTree"
    pine_mat = asset_utils.create_material_instance(pine_mat_path, master_mat, [0.05, 0.25, 0.05, 1.0], 0.8)

    rock_mat_path = f"{mat_dir}/MI_ForestRock"
    rock_mat = asset_utils.create_material_instance(rock_mat_path, master_mat, [0.35, 0.35, 0.32, 1.0], 0.9)

    log_mat_path = f"{mat_dir}/MI_ForestLog"
    log_mat = asset_utils.create_material_instance(log_mat_path, master_mat, [0.25, 0.15, 0.05, 1.0], 0.8)

    pine_mesh_path = f"{base_dir}/SM_PineTree"
    generate_pine_tree(pine_mesh_path, pine_mat)

    rock_mesh_path = f"{base_dir}/SM_ForestRock"
    generate_forest_rock(rock_mesh_path, rock_mat)

    log_mesh_path = f"{base_dir}/SM_ForestLog"
    generate_forest_log(log_mesh_path, log_mat)

    print("Asset generation for Forest biome complete.")

if __name__ == '__main__':
    main()
