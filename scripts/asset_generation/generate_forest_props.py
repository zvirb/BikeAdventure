import unreal
import asset_utils

def generate_forest_bush(mesh_path, mat_inst):
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

    try:
        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            # Bush core
            core_transform = unreal.Transform()
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_sphere(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=core_transform,
                radius=40.0,
                steps_x=10,
                steps_y=10,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
            # Add some variations to make it look bushy
            for i in range(3):
                part_transform = unreal.Transform()
                part_transform.translation = [(i - 1) * 20.0, (i % 2) * 20.0, 10.0]
                unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_sphere(
                    target_mesh=dyn_mesh,
                    primitive_options=options,
                    transform=part_transform,
                    radius=25.0,
                    steps_x=8,
                    steps_y=8,
                    origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
                )
        else:
            # Bush core
            core_transform = unreal.Transform()
            unreal.GeometryScript_MeshPrimitiveFunctions.append_sphere(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=core_transform,
                radius=40.0,
                steps_x=10,
                steps_y=10,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
            # Add some variations to make it look bushy
            for i in range(3):
                part_transform = unreal.Transform()
                part_transform.translation = [(i - 1) * 20.0, (i % 2) * 20.0, 10.0]
                unreal.GeometryScript_MeshPrimitiveFunctions.append_sphere(
                    target_mesh=dyn_mesh,
                    primitive_options=options,
                    transform=part_transform,
                    radius=25.0,
                    steps_x=8,
                    steps_y=8,
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
        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")

def generate_forest_mushroom(mesh_path, mat_inst):
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

    try:
        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            # Stem
            stem_transform = unreal.Transform()
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=stem_transform,
                radius=5.0,
                height=20.0,
                radial_steps=8,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
            # Cap
            cap_transform = unreal.Transform()
            cap_transform.translation = [0.0, 0.0, 15.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_cone(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=cap_transform,
                base_radius=15.0,
                top_radius=2.0,
                height=10.0,
                radial_steps=12,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
        else:
            # Stem
            stem_transform = unreal.Transform()
            unreal.GeometryScript_MeshPrimitiveFunctions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=stem_transform,
                radius=5.0,
                height=20.0,
                radial_steps=8,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
            # Cap
            cap_transform = unreal.Transform()
            cap_transform.translation = [0.0, 0.0, 15.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_cone(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=cap_transform,
                base_radius=15.0,
                top_radius=2.0,
                height=10.0,
                radial_steps=12,
                height_steps=1,
                capped=True,
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
        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")


def main():
    base_dir = '/Game/Art/Models/Forest'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Note: Use the existing master material if present, otherwise create a new one.
    master_mat_path = f"{mat_dir}/M_StylizedForest"
    master_mat = asset_utils.create_master_material(master_mat_path, default_roughness=0.8, noise_scale=0.05)

    # Procedural Material Instances
    bush_mat_path = f"{mat_dir}/MI_ForestBush"
    # Darker green for bush
    bush_mat = asset_utils.create_material_instance(bush_mat_path, master_mat, [0.08, 0.35, 0.08, 1.0], 0.9)

    mushroom_mat_path = f"{mat_dir}/MI_ForestMushroom"
    # Red with some brightness for the mushroom
    mushroom_mat = asset_utils.create_material_instance(mushroom_mat_path, master_mat, [0.8, 0.1, 0.1, 1.0], 0.6)

    # Programmatic 3D Modeling
    bush_mesh_path = f"{base_dir}/SM_ForestBush"
    generate_forest_bush(bush_mesh_path, bush_mat)

    mushroom_mesh_path = f"{base_dir}/SM_ForestMushroom"
    generate_forest_mushroom(mushroom_mesh_path, mushroom_mat)

    print("Asset generation for Forest biome extensions complete.")

if __name__ == '__main__':
    main()
