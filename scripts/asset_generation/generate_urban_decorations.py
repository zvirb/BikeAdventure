import unreal
import asset_utils

def generate_urban_billboard(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # Central Pole
        pole_transform = unreal.Transform()
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=pole_transform,
            dimension_x=20.0,
            dimension_y=20.0,
            dimension_z=400.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Billboard Sign
        board_transform = unreal.Transform()
        board_transform.translation = [0.0, 0.0, 350.0]
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=board_transform,
            dimension_x=400.0,
            dimension_y=20.0,
            dimension_z=200.0,
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
        print(f"GeometryScript mesh bake failed: {e}")

def generate_urban_signpost(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # Signpost Pole
        pole_transform = unreal.Transform()
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=pole_transform,
            radius=4.0,
            height=200.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Sign 1
        sign1_transform = unreal.Transform()
        sign1_transform.translation = [0.0, 0.0, 180.0]
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=sign1_transform,
            dimension_x=60.0,
            dimension_y=4.0,
            dimension_z=20.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Sign 2
        sign2_transform = unreal.Transform()
        sign2_transform.translation = [0.0, 0.0, 150.0]
        sign2_transform.rotation = unreal.Rotator(0, 90, 0).quaternion()
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=sign2_transform,
            dimension_x=60.0,
            dimension_y=4.0,
            dimension_z=20.0,
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

    base_dir = '/Game/Art/Models/Urban'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedMaster"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    billboard_mat_path = f"{mat_dir}/MI_UrbanBillboard"
    # Grayish metal/concrete color
    billboard_mat = asset_utils.create_material_instance(billboard_mat_path, master_mat, [0.3, 0.3, 0.35, 1.0], 0.7)

    signpost_mat_path = f"{mat_dir}/MI_UrbanSignpost"
    # Darker grey metal color
    signpost_mat = asset_utils.create_material_instance(signpost_mat_path, master_mat, [0.2, 0.2, 0.25, 1.0], 0.6)

    # Programmatic 3D Modeling
    billboard_mesh_path = f"{base_dir}/SM_UrbanBillboard"
    generate_urban_billboard(billboard_mesh_path, billboard_mat)

    signpost_mesh_path = f"{base_dir}/SM_UrbanSignpost"
    generate_urban_signpost(signpost_mesh_path, signpost_mat)

    print("Asset generation for Urban Decorations complete.")

if __name__ == '__main__':
    main()
