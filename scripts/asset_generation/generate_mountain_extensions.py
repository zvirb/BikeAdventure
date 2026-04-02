import unreal
import asset_utils

def generate_mountain_snow(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # Snow pile
        snow_transform = unreal.Transform()
        primitive_functions.append_sphere(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=snow_transform,
            radius=40.0,
            steps_x=8,
            steps_y=8,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        snow_transform2 = unreal.Transform()
        snow_transform2.translation = [20.0, 10.0, 0.0]
        primitive_functions.append_sphere(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=snow_transform2,
            radius=30.0,
            steps_x=8,
            steps_y=8,
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

def generate_mountain_pebbles(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        pebble_transform1 = unreal.Transform()
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=pebble_transform1,
            dimension_x=10.0,
            dimension_y=15.0,
            dimension_z=5.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        pebble_transform2 = unreal.Transform()
        pebble_transform2.translation = [12.0, -8.0, 0.0]
        pebble_transform2.rotation = unreal.Rotator(0, 45, 0).quaternion()
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=pebble_transform2,
            dimension_x=8.0,
            dimension_y=8.0,
            dimension_z=4.0,
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
        print(f"GeometryScript mesh bake failed: {e}")


def main():
    import sys
    import os
    try:
        # If run directly
        script_dir = os.path.dirname(os.path.abspath(__file__))
    except NameError:
        # If run via exec(open(...).read()) in Unreal Editor
        import unreal
        script_dir = os.path.join(unreal.Paths.project_dir(), "scripts/asset_generation")

    if script_dir not in sys.path:
        sys.path.append(script_dir)

    import asset_utils

    base_dir = '/Game/Art/Models/Mountains'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedMaster"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    snow_mat_path = f"{mat_dir}/MI_MountainSnow"
    snow_mat = asset_utils.create_material_instance(snow_mat_path, master_mat, [0.9, 0.9, 0.95, 1.0], 0.8)

    pebble_mat_path = f"{mat_dir}/MI_MountainPebbles"
    pebble_mat = asset_utils.create_material_instance(pebble_mat_path, master_mat, [0.5, 0.45, 0.4, 1.0], 0.6)

    # Programmatic 3D Modeling
    snow_mesh_path = f"{base_dir}/SM_MountainSnow"
    generate_mountain_snow(snow_mesh_path, snow_mat)

    pebbles_mesh_path = f"{base_dir}/SM_MountainPebbles"
    generate_mountain_pebbles(pebbles_mesh_path, pebble_mat)

    print("Asset generation for Mountain Extensions complete.")

if __name__ == '__main__':
    main()
