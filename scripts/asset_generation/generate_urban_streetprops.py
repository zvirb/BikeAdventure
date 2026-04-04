import unreal
import sys
import os

try:
    # Handle both explicit execution and UE5 Editor utility running
    script_dir = os.path.dirname(os.path.abspath(__file__))
    if script_dir not in sys.path:
        sys.path.append(script_dir)
except NameError:
    # __file__ is not defined when running via exec() in UE5 Python console
    fallback_path = os.path.join(unreal.Paths.project_dir(), "scripts/asset_generation")
    if fallback_path not in sys.path:
        sys.path.append(fallback_path)

import asset_utils

def generate_urban_firehydrant(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        append_cylinder = primitive_functions.append_cylinder
        append_sphere = primitive_functions.append_sphere

        # Base cylinder
        base_transform = unreal.Transform()
        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=base_transform,
            radius=12.0,
            height=40.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Top dome
        top_transform = unreal.Transform()
        top_transform.translation = [0.0, 0.0, 40.0]
        append_sphere(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=top_transform,
            radius=12.0,
            steps_x=8,
            steps_y=8,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Side nozzle 1
        nozzle1_transform = unreal.Transform()
        nozzle1_transform.translation = [12.0, 0.0, 25.0]
        nozzle1_transform.rotation = unreal.Rotator(0, 90, 0).quaternion()
        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=nozzle1_transform,
            radius=5.0,
            height=10.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Side nozzle 2
        nozzle2_transform = unreal.Transform()
        nozzle2_transform.translation = [-12.0, 0.0, 25.0]
        nozzle2_transform.rotation = unreal.Rotator(0, -90, 0).quaternion()
        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=nozzle2_transform,
            radius=5.0,
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

def generate_urban_mailbox(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        append_box = primitive_functions.append_box
        append_cylinder = primitive_functions.append_cylinder

        # Main box body
        box_transform = unreal.Transform()
        box_transform.translation = [0.0, 0.0, 20.0]
        append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=box_transform,
            dimension_x=24.0,
            dimension_y=24.0,
            dimension_z=40.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Curved top
        top_transform = unreal.Transform()
        top_transform.translation = [0.0, 0.0, 40.0]
        top_transform.rotation = unreal.Rotator(90, 0, 0).quaternion()
        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=top_transform,
            radius=12.0,
            height=24.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Leg / Post
        leg_transform = unreal.Transform()
        leg_transform.translation = [0.0, 0.0, -10.0]
        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=leg_transform,
            radius=4.0,
            height=20.0,
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
    base_dir = '/Game/Art/Models/Urban'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Use the existing M_StylizedUrban master material
    master_mat_path = f"{mat_dir}/M_StylizedUrban"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    firehydrant_mat_path = f"{mat_dir}/MI_UrbanFireHydrant"
    firehydrant_mat = asset_utils.create_material_instance(firehydrant_mat_path, master_mat, [0.8, 0.1, 0.1, 1.0], 0.3)

    mailbox_mat_path = f"{mat_dir}/MI_UrbanMailbox"
    mailbox_mat = asset_utils.create_material_instance(mailbox_mat_path, master_mat, [0.1, 0.2, 0.8, 1.0], 0.4)

    # Programmatic 3D Modeling
    firehydrant_mesh_path = f"{base_dir}/SM_UrbanFireHydrant"
    generate_urban_firehydrant(firehydrant_mesh_path, firehydrant_mat)

    mailbox_mesh_path = f"{base_dir}/SM_UrbanMailbox"
    generate_urban_mailbox(mailbox_mesh_path, mailbox_mat)

    print("Asset generation for Urban Street Props complete.")

if __name__ == '__main__':
    main()
