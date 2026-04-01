import unreal
import asset_utils

def generate_desert_shrub(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    try:
        # Create base spherical bush for shrub
        primitive_functions.append_sphere(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            radius=40.0,
            steps_x=8,
            steps_y=8,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Add a couple of smaller spheres to make it irregular
        blob1 = unreal.Transform()
        blob1.translation = [20.0, 15.0, 10.0]
        primitive_functions.append_sphere(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=blob1,
            radius=25.0,
            steps_x=6,
            steps_y=6,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        blob2 = unreal.Transform()
        blob2.translation = [-15.0, -20.0, 5.0]
        primitive_functions.append_sphere(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=blob2,
            radius=30.0,
            steps_x=6,
            steps_y=6,
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

def generate_oasis_palm_tree(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    try:
        # Trunk
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            radius=15.0,
            height=300.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Canopy / Leaves
        canopy_transform = unreal.Transform()
        canopy_transform.translation = [0.0, 0.0, 280.0]
        primitive_functions.append_sphere(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=canopy_transform,
            radius=120.0,
            steps_x=10,
            steps_y=10,
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
        # A simple flat disc/cylinder for the oasis water pool
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            radius=400.0,
            height=10.0,
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
    # We assume master_mat exists from previous run, but we can recreate or just get it.
    # For safety, we will just call create_master_material which should be idempotent or just return it.
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    shrub_mat_path = f"{mat_dir}/MI_DesertShrub"
    shrub_mat = asset_utils.create_material_instance(shrub_mat_path, master_mat, [0.4, 0.5, 0.2, 1.0], 0.8) # Dry green

    palm_mat_path = f"{mat_dir}/MI_OasisPalm"
    palm_mat = asset_utils.create_material_instance(palm_mat_path, master_mat, [0.2, 0.6, 0.2, 1.0], 0.7) # Brighter green for oasis

    water_mat_path = f"{mat_dir}/MI_OasisWater"
    water_mat = asset_utils.create_material_instance(water_mat_path, master_mat, [0.1, 0.5, 0.8, 1.0], 0.1) # Blue, low roughness

    # Programmatic 3D Modeling
    shrub_mesh_path = f"{base_dir}/SM_DesertShrub"
    generate_desert_shrub(shrub_mesh_path, shrub_mat)

    palm_mesh_path = f"{base_dir}/SM_OasisPalmTree"
    generate_oasis_palm_tree(palm_mesh_path, palm_mat)

    water_mesh_path = f"{base_dir}/SM_OasisWater"
    generate_oasis_water(water_mesh_path, water_mat)

    print("Asset generation for Desert extensions complete.")

if __name__ == '__main__':
    main()
