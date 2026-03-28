import unreal
import sys
import os

# Ensure we can import asset_utils regardless of how this script is executed
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.append(current_dir)

import asset_utils

def generate_urban_trash_can(mesh_path, mat_inst):
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
        # Main body
        body_transform = unreal.Transform()
        body_transform.translation = [0.0, 0.0, 50.0]
        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=body_transform,
                radius=30.0,
                height=100.0,
                radial_steps=12,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            # Top lid
            lid_transform = unreal.Transform()
            lid_transform.translation = [0.0, 0.0, 105.0]
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=lid_transform,
                radius=32.0,
                height=10.0,
                radial_steps=12,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )
        else:
            unreal.GeometryScript_MeshPrimitiveFunctions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=body_transform,
                radius=30.0,
                height=100.0,
                radial_steps=12,
                height_steps=1,
                capped=True,
                origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
            )

            # Top lid
            lid_transform = unreal.Transform()
            lid_transform.translation = [0.0, 0.0, 105.0]
            unreal.GeometryScript_MeshPrimitiveFunctions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=lid_transform,
                radius=32.0,
                height=10.0,
                radial_steps=12,
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


def generate_urban_bus_stop(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # Roof
        roof_transform = unreal.Transform()
        roof_transform.translation = [0.0, 0.0, 250.0]
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=roof_transform,
            dimension_x=200.0,
            dimension_y=150.0,
            dimension_z=10.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Back wall
        back_wall_transform = unreal.Transform()
        back_wall_transform.translation = [-90.0, 0.0, 125.0]
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=back_wall_transform,
            dimension_x=10.0,
            dimension_y=140.0,
            dimension_z=250.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Side panel 1
        side1_transform = unreal.Transform()
        side1_transform.translation = [0.0, 70.0, 125.0]
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=side1_transform,
            dimension_x=180.0,
            dimension_y=10.0,
            dimension_z=250.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Side panel 2
        side2_transform = unreal.Transform()
        side2_transform.translation = [0.0, -70.0, 125.0]
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=side2_transform,
            dimension_x=180.0,
            dimension_y=10.0,
            dimension_z=250.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Bench seat
        seat_transform = unreal.Transform()
        seat_transform.translation = [-60.0, 0.0, 50.0]
        primitive_functions.append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=seat_transform,
            dimension_x=40.0,
            dimension_y=120.0,
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


def main():
    base_dir = '/Game/Art/Models/Urban'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedUrban"
    master_mat = asset_utils.create_master_material(master_mat_path)

    # Procedural Material Instances
    trash_can_mat_path = f"{mat_dir}/MI_UrbanTrashCan"
    trash_can_mat = asset_utils.create_material_instance(trash_can_mat_path, master_mat, [0.2, 0.2, 0.2, 1.0], 0.4)

    bus_stop_mat_path = f"{mat_dir}/MI_UrbanBusStop"
    bus_stop_mat = asset_utils.create_material_instance(bus_stop_mat_path, master_mat, [0.3, 0.4, 0.5, 1.0], 0.3)

    # Programmatic 3D Modeling
    trash_can_mesh_path = f"{base_dir}/SM_UrbanTrashCan"
    generate_urban_trash_can(trash_can_mesh_path, trash_can_mat)

    bus_stop_mesh_path = f"{base_dir}/SM_UrbanBusStop"
    generate_urban_bus_stop(bus_stop_mesh_path, bus_stop_mat)

    print("Asset generation for Urban Props complete.")

if __name__ == '__main__':
    main()
