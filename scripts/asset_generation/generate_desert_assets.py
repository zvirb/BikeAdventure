import unreal
import asset_utils

def generate_desert_cactus(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    # Create cactus base
    try:
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            radius=30.0,
            height=200.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Left arm
        arm1_transform = unreal.Transform()
        arm1_transform.translation = [-40.0, 0.0, 100.0]
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=arm1_transform,
            radius=20.0,
            height=80.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Right arm
        arm2_transform = unreal.Transform()
        arm2_transform.translation = [40.0, 0.0, 130.0]
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=arm2_transform,
            radius=20.0,
            height=60.0,
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

def generate_desert_rock(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    # Create sandstone rock base
    try:
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            dimension_x=120.0,
            dimension_y=150.0,
            dimension_z=60.0,
            steps_x=3,
            steps_y=3,
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

def main():
    base_dir = '/Game/Art/Models/Desert'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedDesert"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    cactus_mat_path = f"{mat_dir}/MI_DesertCactus"
    cactus_mat = create_material_instance(cactus_mat_path, master_mat, [0.3, 0.6, 0.2, 1.0], 0.7)

    rock_mat_path = f"{mat_dir}/MI_DesertRock"
    rock_mat = create_material_instance(rock_mat_path, master_mat, [0.8, 0.6, 0.4, 1.0], 0.9)

    # Programmatic 3D Modeling
    cactus_mesh_path = f"{base_dir}/SM_DesertCactus"
    generate_desert_cactus(cactus_mesh_path, cactus_mat)

    rock_mesh_path = f"{base_dir}/SM_DesertRock"
    generate_desert_rock(rock_mesh_path, rock_mat)

    print("Asset generation for Desert biome complete.")

if __name__ == '__main__':
    main()
