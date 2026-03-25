import unreal
import asset_utils

def generate_forest_flower(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # Stem
        stem_transform = unreal.Transform()
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=stem_transform,
            radius=2.0,
            height=30.0,
            radial_steps=6,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Flower petals (simple sphere at the top)
        petal_transform = unreal.Transform()
        petal_transform.translation = [0.0, 0.0, 30.0]
        primitive_functions.append_sphere(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=petal_transform,
            radius=8.0,
            steps_x=6,
            steps_y=6,
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


def generate_forest_fern(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # We can approximate a fern with some radiating cones
        for i in range(5):
            angle = (i / 5.0) * 360.0
            leaf_transform = unreal.Transform()
            # Rotate outwards
            leaf_transform.rotation = unreal.Rotator(60, angle, 0).quaternion()
            primitive_functions.append_cone(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=leaf_transform,
                base_radius=5.0,
                top_radius=0.0,
                height=40.0,
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
        print(f"GeometryScript bake failed: {e}")


def main():
    base_dir = '/Game/Art/Models/Forest'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedMaster"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    flower_mat_path = f"{mat_dir}/MI_ForestFlower"
    flower_mat = asset_utils.create_material_instance(flower_mat_path, master_mat, [0.8, 0.2, 0.6, 1.0], 0.6)

    fern_mat_path = f"{mat_dir}/MI_ForestFern"
    fern_mat = asset_utils.create_material_instance(fern_mat_path, master_mat, [0.2, 0.6, 0.2, 1.0], 0.8)

    # Programmatic 3D Modeling
    flower_mesh_path = f"{base_dir}/SM_ForestFlower"
    generate_forest_flower(flower_mesh_path, flower_mat)

    fern_mesh_path = f"{base_dir}/SM_ForestFern"
    generate_forest_fern(fern_mesh_path, fern_mat)

    print("Asset generation for Forest Extensions complete.")

if __name__ == '__main__':
    main()
