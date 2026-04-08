import unreal

def generate_beach_umbrella(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    import asset_utils
    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # Pole
        pole_transform = unreal.Transform()
        pole_transform.translation = [0.0, 0.0, 75.0]
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=pole_transform,
            radius=3.0,
            height=150.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Canopy
        canopy_transform = unreal.Transform()
        canopy_transform.translation = [0.0, 0.0, 150.0]
        primitive_functions.append_cone(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=canopy_transform,
            base_radius=60.0,
            top_radius=5.0,
            height=20.0,
            radial_steps=16,
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

def generate_beach_surfboard(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    import asset_utils
    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # Body
        board_transform = unreal.Transform()
        # Scale to make it flat and oval
        board_transform.scale3d = [3.0, 1.0, 0.15]
        primitive_functions.append_sphere(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=board_transform,
            radius=40.0,
            steps_x=16,
            steps_y=16,
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
    import sys
    import os
    try:
        script_dir = os.path.dirname(os.path.abspath(__file__))
    except NameError:
        script_dir = os.path.join(unreal.Paths.project_dir(), "scripts/asset_generation")

    if script_dir not in sys.path:
        sys.path.append(script_dir)

    import asset_utils

    base_dir = '/Game/Art/Models/Beach'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedMaster"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    umbrella_mat_path = f"{mat_dir}/MI_BeachUmbrella"
    # Vivid orange/red
    umbrella_mat = asset_utils.create_material_instance(umbrella_mat_path, master_mat, [0.8, 0.2, 0.1, 1.0], 0.6)

    surfboard_mat_path = f"{mat_dir}/MI_Surfboard"
    # Bright cyan/blue
    surfboard_mat = asset_utils.create_material_instance(surfboard_mat_path, master_mat, [0.1, 0.7, 0.9, 1.0], 0.2)

    # Programmatic 3D Modeling
    umbrella_mesh_path = f"{base_dir}/SM_BeachUmbrella"
    generate_beach_umbrella(umbrella_mesh_path, umbrella_mat)

    surfboard_mesh_path = f"{base_dir}/SM_Surfboard"
    generate_beach_surfboard(surfboard_mesh_path, surfboard_mat)

    print("Asset generation for Beach Extensions complete.")

if __name__ == '__main__':
    main()
