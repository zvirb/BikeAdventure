import unreal
import sys
import os

# Ensure we can import asset_utils regardless of how this script is executed
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.append(current_dir)

import asset_utils

def generate_wetlands_log(mesh_path, mat_inst):
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
            append_cylinder = unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_cylinder
        else:
            append_cylinder = unreal.GeometryScript_MeshPrimitiveFunctions.append_cylinder

        # Base log cylinder
        log_transform = unreal.Transform()
        log_transform.translation = [0.0, 0.0, 15.0]
        # Rotate 90 degrees to lay flat
        log_transform.rotation = unreal.Rotator(0, 90, 0).quaternion()

        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=log_transform,
            radius=15.0,
            height=120.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Apply noise for a more organic look if available
        if hasattr(unreal, "GeometryScriptLibrary_MeshDeformFunctions"):
            noise_options = unreal.GeometryScriptPerlinNoiseOptions()
            noise_options.magnitude = 5.0
            noise_options.frequency = 0.1
            unreal.GeometryScriptLibrary_MeshDeformFunctions.apply_perlin_noise_to_mesh(
                target_mesh=dyn_mesh,
                options=noise_options
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

        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")


def generate_wetlands_lilypad(mesh_path, mat_inst):
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
            append_cylinder = unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_cylinder
        else:
            append_cylinder = unreal.GeometryScript_MeshPrimitiveFunctions.append_cylinder

        # Base lilypad disc
        disc_transform = unreal.Transform()
        disc_transform.translation = [0.0, 0.0, 1.0]

        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=disc_transform,
            radius=20.0,
            height=2.0,
            radial_steps=12,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
        )

        # To make it more like a lilypad, we'd boolean out a wedge, but a simple flat disc works for low poly stylized

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

        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")


def main():
    base_dir = '/Game/Art/Models/Wetlands'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedWetlands"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    log_mat_path = f"{mat_dir}/MI_WetlandsLog"
    # Brown stylized look for the log
    log_mat = asset_utils.create_material_instance(log_mat_path, master_mat, [0.3, 0.2, 0.1, 1.0], 0.9)

    lilypad_mat_path = f"{mat_dir}/MI_WetlandsLilypad"
    # Vibrant green stylized look for the lilypad
    lilypad_mat = asset_utils.create_material_instance(lilypad_mat_path, master_mat, [0.1, 0.6, 0.2, 1.0], 0.5)

    # Programmatic 3D Modeling
    log_mesh_path = f"{base_dir}/SM_WetlandsLog"
    generate_wetlands_log(log_mesh_path, log_mat)

    lilypad_mesh_path = f"{base_dir}/SM_WetlandsLilypad"
    generate_wetlands_lilypad(lilypad_mesh_path, lilypad_mat)

    print("Asset generation for Wetlands Extensions complete.")

if __name__ == '__main__':
    main()
