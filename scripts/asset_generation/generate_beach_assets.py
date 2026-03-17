import unreal
import asset_utils

def generate_palm_tree(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # Trunk
        trunk_transform = unreal.Transform()
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=trunk_transform,
            radius=20.0,
            height=400.0,
            radial_steps=6,
            height_steps=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )
        # Palm leaves (simple boxes crossing at the top)
        for i in range(4):
            leaf_transform = unreal.Transform()
            leaf_transform.translation = [0.0, 0.0, 380.0]
            leaf_transform.rotation = unreal.Rotator(0, i * 45, 20).quaternion()

            primitive_functions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=leaf_transform,
                dimension_x=150.0,
                dimension_y=30.0,
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


def generate_sandcastle(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # Base
        base_transform = unreal.Transform()
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=base_transform,
            dimension_x=100.0,
            dimension_y=100.0,
            dimension_z=40.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )
        # Towers
        for i in [-30, 30]:
            for j in [-30, 30]:
                tower_transform = unreal.Transform()
                tower_transform.translation = [i, j, 40.0]
                primitive_functions.append_cylinder(
                    target_mesh=dyn_mesh,
                    primitive_options=options,
                    transform=tower_transform,
                    radius=15.0,
                    height=50.0,
                    radial_steps=6,
                    height_steps=1,
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
    base_dir = '/Game/Art/Models/Beach'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Material setup
    master_mat_path = f"{mat_dir}/M_StylizedBeach"
    master_mat = asset_utils.create_master_material(master_mat_path, default_roughness=0.8, noise_scale=0.05)

    palm_mat_path = f"{mat_dir}/MI_PalmTree"
    palm_mat = asset_utils.create_material_instance(palm_mat_path, master_mat, [0.1, 0.4, 0.1, 1.0], 0.7)

    sandcastle_mat_path = f"{mat_dir}/MI_Sandcastle"
    sandcastle_mat = asset_utils.create_material_instance(sandcastle_mat_path, master_mat, [0.8, 0.7, 0.4, 1.0], 0.9)

    # Generation
    palm_mesh_path = f"{base_dir}/SM_PalmTree"
    generate_palm_tree(palm_mesh_path, palm_mat)

    sandcastle_mesh_path = f"{base_dir}/SM_Sandcastle"
    generate_sandcastle(sandcastle_mesh_path, sandcastle_mat)

    print("Asset generation for Beach biome complete.")

if __name__ == '__main__':
    main()
