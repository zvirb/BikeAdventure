import unreal
import sys
import os

# Ensure asset_utils can be imported even when run via exec() in editor
try:
    import asset_utils
except NameError:
    sys.path.append(os.path.join(unreal.Paths.project_dir(), "scripts/asset_generation"))
    import asset_utils

def generate_oasis_palm_tree(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    try:
        # Create trunk
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            radius=20.0,
            height=300.0,
            radial_steps=8,
            height_steps=3,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Create basic leaves
        for i in range(4):
            leaf_transform = unreal.Transform()
            leaf_transform.translation = [0.0, 0.0, 280.0]
            # Rotate each leaf 90 degrees around Z, and angle down slightly around Y
            leaf_transform.rotation = unreal.Rotator(0, i * 90.0, -30.0).quaternion()
            primitive_functions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=leaf_transform,
                dimension_x=120.0,
                dimension_y=40.0,
                dimension_z=10.0,
                steps_x=2,
                steps_y=2,
                steps_z=2,
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

def generate_oasis_water(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    try:
        # Create simple flat disc for water
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            radius=250.0,
            height=5.0,
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

def main():
    base_dir = '/Game/Art/Models/Desert'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedDesert"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    palm_mat_path = f"{mat_dir}/MI_OasisPalmTree"
    # stylized green
    palm_mat = asset_utils.create_material_instance(palm_mat_path, master_mat, [0.1, 0.5, 0.2, 1.0], 0.8)

    water_mat_path = f"{mat_dir}/MI_OasisWater"
    # stylized blue/cyan
    water_mat = asset_utils.create_material_instance(water_mat_path, master_mat, [0.2, 0.6, 0.9, 1.0], 0.1)

    # Programmatic 3D Modeling
    palm_mesh_path = f"{base_dir}/SM_OasisPalmTree"
    generate_oasis_palm_tree(palm_mesh_path, palm_mat)

    water_mesh_path = f"{base_dir}/SM_OasisWater"
    generate_oasis_water(water_mesh_path, water_mat)

    print("Asset generation for Desert Oasis complete.")

if __name__ == '__main__':
    main()
