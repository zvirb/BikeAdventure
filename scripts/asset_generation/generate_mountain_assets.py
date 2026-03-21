import unreal
import asset_utils

def generate_alpine_plant(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    # Add trunk
    try:
        primitive_functions.append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            radius=10.0,
            height=50.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # Add leaves layers
        leaf_transform = unreal.Transform()
        leaf_transform.translation = [0, 0, 40.0]
        primitive_functions.append_cone(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=leaf_transform,
            base_radius=40.0,
            top_radius=0.0,
            height=60.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        leaf_transform.translation = [0, 0, 80.0]
        primitive_functions.append_cone(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=leaf_transform,
            base_radius=30.0,
            top_radius=0.0,
            height=50.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )
    except Exception as e:
        print(f"GeometryScript generation failed (perhaps plugin not enabled): {e}")
        return

    try:
        static_mesh = asset_utils.create_static_mesh_from_dynamic_mesh(dyn_mesh, mesh_path)
        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript mesh bake failed: {e}")


def generate_mountain_cliff(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    try:
        dyn_mesh = unreal.GeometryScriptLibrary_CreateNewDynamicMesh.create_new_dynamic_mesh()
    except AttributeError:
        try:
            dyn_mesh = unreal.GeometryScript_AssetUtils.create_new_dynamic_mesh()
        except AttributeError:
            dyn_mesh = unreal.DynamicMesh()

    options = unreal.GeometryScriptPrimitiveOptions()
    transform = unreal.Transform()

    try:
        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=transform,
                dimension_x=400.0,
                dimension_y=200.0,
                dimension_z=500.0,
                steps_x=3,
                steps_y=3,
                steps_z=3,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
        else:
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=transform,
                dimension_x=400.0,
                dimension_y=200.0,
                dimension_z=500.0,
                steps_x=3,
                steps_y=3,
                steps_z=3,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
    except Exception as e:
        print(f"GeometryScript generation failed: {e}")
        return

    create_options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    try:
        if hasattr(unreal, "GeometryScriptLibrary_CreateNewStaticMeshAssetFromDynamicMesh"):
            static_mesh = unreal.GeometryScriptLibrary_CreateNewStaticMeshAssetFromDynamicMesh.create_new_static_mesh_asset_from_dynamic_mesh(
                dynamic_mesh=dyn_mesh,
                asset_path_and_name=mesh_path,
                options=create_options
            )
        else:
            static_mesh = unreal.GeometryScript_AssetUtils.create_new_static_mesh_asset_from_dynamic_mesh(
                dynamic_mesh=dyn_mesh,
                asset_path_and_name=mesh_path,
                options=create_options
            )

        # Apply material
        assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")


def generate_mountain_rock(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()
    transform = unreal.Transform()

    # Create base box to act as a rock
    try:
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=transform,
            dimension_x=100.0,
            dimension_y=120.0,
            dimension_z=80.0,
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

def generate_mountain_cave(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    try:
        dyn_mesh = unreal.GeometryScriptLibrary_CreateNewDynamicMesh.create_new_dynamic_mesh()
        dyn_mesh_subtract = unreal.GeometryScriptLibrary_CreateNewDynamicMesh.create_new_dynamic_mesh()
    except AttributeError:
        try:
            dyn_mesh = unreal.GeometryScript_AssetUtils.create_new_dynamic_mesh()
            dyn_mesh_subtract = unreal.GeometryScript_AssetUtils.create_new_dynamic_mesh()
        except AttributeError:
            dyn_mesh = unreal.DynamicMesh()
            dyn_mesh_subtract = unreal.DynamicMesh()

    options = unreal.GeometryScriptPrimitiveOptions()

    try:
        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            # Outer Cave Rock Structure
            base_transform = unreal.Transform()
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=base_transform,
                dimension_x=300.0,
                dimension_y=350.0,
                dimension_z=200.0,
                steps_x=3,
                steps_y=3,
                steps_z=3,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

            # Inner empty space to subtract
            subtract_transform = unreal.Transform()
            subtract_transform.translation = [50.0, 0.0, 0.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh_subtract,
                primitive_options=options,
                transform=subtract_transform,
                dimension_x=220.0,
                dimension_y=150.0,
                dimension_z=120.0,
                steps_x=3,
                steps_y=3,
                steps_z=3,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

            if hasattr(unreal, "GeometryScriptLibrary_MeshBooleanFunctions"):
                unreal.GeometryScriptLibrary_MeshBooleanFunctions.apply_mesh_boolean(
                    target_mesh=dyn_mesh,
                    tool_mesh=dyn_mesh_subtract,
                    operation=unreal.GeometryScriptBooleanOperation.SUBTRACT
                )
        else:
            # Outer Cave Rock Structure
            base_transform = unreal.Transform()
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=base_transform,
                dimension_x=300.0,
                dimension_y=350.0,
                dimension_z=200.0,
                steps_x=3,
                steps_y=3,
                steps_z=3,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

            # Inner empty space to subtract
            subtract_transform = unreal.Transform()
            subtract_transform.translation = [50.0, 0.0, 0.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
                target_mesh=dyn_mesh_subtract,
                primitive_options=options,
                transform=subtract_transform,
                dimension_x=220.0,
                dimension_y=150.0,
                dimension_z=120.0,
                steps_x=3,
                steps_y=3,
                steps_z=3,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )

            if hasattr(unreal, "GeometryScript_MeshBooleanFunctions"):
                unreal.GeometryScript_MeshBooleanFunctions.apply_mesh_boolean(
                    target_mesh=dyn_mesh,
                    tool_mesh=dyn_mesh_subtract,
                    operation=unreal.GeometryScriptBooleanOperation.SUBTRACT
                )
    except Exception as e:
        print(f"GeometryScript generation failed: {e}")
        return

    create_options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    try:
        if hasattr(unreal, "GeometryScriptLibrary_CreateNewStaticMeshAssetFromDynamicMesh"):
            static_mesh = unreal.GeometryScriptLibrary_CreateNewStaticMeshAssetFromDynamicMesh.create_new_static_mesh_asset_from_dynamic_mesh(
                dynamic_mesh=dyn_mesh,
                asset_path_and_name=mesh_path,
                options=create_options
            )
        else:
            static_mesh = unreal.GeometryScript_AssetUtils.create_new_static_mesh_asset_from_dynamic_mesh(
                dynamic_mesh=dyn_mesh,
                asset_path_and_name=mesh_path,
                options=create_options
            )

        # Apply material
        assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")

def main():
    base_dir = '/Game/Art/Models/Mountains'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedMaster"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    plant_mat_path = f"{mat_dir}/MI_AlpinePlant"
    plant_mat = asset_utils.create_material_instance(plant_mat_path, master_mat, [0.1, 0.4, 0.1, 1.0], 0.8)

    rock_mat_path = f"{mat_dir}/MI_MountainRock"
    rock_mat = asset_utils.create_material_instance(rock_mat_path, master_mat, [0.3, 0.3, 0.3, 1.0], 0.9)

    cliff_mat_path = f"{mat_dir}/MI_MountainCliff"
    cliff_mat = asset_utils.create_material_instance(cliff_mat_path, master_mat, [0.4, 0.35, 0.35, 1.0], 0.9)

    cave_mat_path = f"{mat_dir}/MI_MountainCave"
    cave_mat = asset_utils.create_material_instance(cave_mat_path, master_mat, [0.15, 0.15, 0.15, 1.0], 0.95)

    # Programmatic 3D Modeling
    plant_mesh_path = f"{base_dir}/SM_AlpinePlant"
    generate_alpine_plant(plant_mesh_path, plant_mat)

    rock_mesh_path = f"{base_dir}/SM_MountainRock"
    generate_mountain_rock(rock_mesh_path, rock_mat)

    cliff_mesh_path = f"{base_dir}/SM_MountainCliff"
    generate_mountain_cliff(cliff_mesh_path, cliff_mat)

    cave_mesh_path = f"{base_dir}/SM_MountainCave"
    generate_mountain_cave(cave_mesh_path, cave_mat)

    print("Asset generation for Mountains biome complete.")

if __name__ == '__main__':
    main()
