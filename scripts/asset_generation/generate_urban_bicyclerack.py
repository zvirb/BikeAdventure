import unreal
import sys
import os

# Ensure we can import asset_utils regardless of how this script is executed
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.append(current_dir)

import asset_utils

def generate_urban_bicycle_rack(mesh_path, mat_inst):
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

        # Base rail
        base_transform = unreal.Transform()
        base_transform.translation = [0.0, 0.0, 5.0]
        base_transform.rotation = unreal.Rotator(0, 90, 0).quaternion()
        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=base_transform,
            radius=5.0,
            height=200.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Hoops
        for i in range(5):
            y_offset = -80.0 + (i * 40.0)

            # Left vertical bar
            left_bar_transform = unreal.Transform()
            left_bar_transform.translation = [-15.0, y_offset, 30.0]
            append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=left_bar_transform,
                radius=3.0,
                height=50.0,
                radial_steps=8,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            # Right vertical bar
            right_bar_transform = unreal.Transform()
            right_bar_transform.translation = [15.0, y_offset, 30.0]
            append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=right_bar_transform,
                radius=3.0,
                height=50.0,
                radial_steps=8,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            # Top horizontal bar
            top_bar_transform = unreal.Transform()
            top_bar_transform.translation = [0.0, y_offset, 55.0]
            top_bar_transform.rotation = unreal.Rotator(0, 0, 90).quaternion()
            append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=top_bar_transform,
                radius=3.0,
                height=30.0,
                radial_steps=8,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
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


def main():
    base_dir = '/Game/Art/Models/Urban'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedUrban"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    rack_mat_path = f"{mat_dir}/MI_UrbanBicycleRack"
    # Silver/Grey metallic-looking color
    rack_mat = asset_utils.create_material_instance(rack_mat_path, master_mat, [0.6, 0.6, 0.65, 1.0], 0.3)

    # Programmatic 3D Modeling
    rack_mesh_path = f"{base_dir}/SM_UrbanBicycleRack"
    generate_urban_bicycle_rack(rack_mesh_path, rack_mat)

    print("Asset generation for Urban Bicycle Rack complete.")

if __name__ == '__main__':
    main()
