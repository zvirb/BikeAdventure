import unreal
import asset_utils

def generate_bike_mesh(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    dyn_mesh = asset_utils.create_dynamic_mesh()
    options = unreal.GeometryScriptPrimitiveOptions()
    primitive_functions = asset_utils.get_geometry_script_primitive_functions()

    try:
        # GeometryScript generation logic

        append_cylinder = primitive_functions.append_cylinder
        append_box = primitive_functions.append_box

        # Front Wheel
        front_wheel_transform = unreal.Transform()
        front_wheel_transform.translation = [50.0, 0.0, 30.0]
        front_wheel_transform.rotation = unreal.Rotator(0, 0, 90).quaternion() # Rotate to make it vertical, facing X
        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=front_wheel_transform,
            radius=30.0,
            height=5.0,
            radial_steps=12,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Back Wheel
        back_wheel_transform = unreal.Transform()
        back_wheel_transform.translation = [-50.0, 0.0, 30.0]
        back_wheel_transform.rotation = unreal.Rotator(0, 0, 90).quaternion() # Rotate to make it vertical
        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=back_wheel_transform,
            radius=30.0,
            height=5.0,
            radial_steps=12,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Main Frame Bar
        frame_transform = unreal.Transform()
        frame_transform.translation = [0.0, 0.0, 60.0]
        append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=frame_transform,
            dimension_x=80.0,
            dimension_y=4.0,
            dimension_z=4.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Seat Post
        seat_post_transform = unreal.Transform()
        seat_post_transform.translation = [-20.0, 0.0, 75.0]
        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=seat_post_transform,
            radius=2.0,
            height=30.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Seat
        seat_transform = unreal.Transform()
        seat_transform.translation = [-20.0, 0.0, 90.0]
        append_box(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=seat_transform,
            dimension_x=20.0,
            dimension_y=10.0,
            dimension_z=5.0,
            steps_x=1,
            steps_y=1,
            steps_z=1,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Handlebar Post
        handlebar_post_transform = unreal.Transform()
        handlebar_post_transform.translation = [40.0, 0.0, 75.0]
        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=handlebar_post_transform,
            radius=2.0,
            height=30.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

        # Handlebars
        handlebars_transform = unreal.Transform()
        handlebars_transform.translation = [40.0, 0.0, 90.0]
        handlebars_transform.rotation = unreal.Rotator(90, 0, 0).quaternion()
        append_cylinder(
            target_mesh=dyn_mesh,
            primitive_options=options,
            transform=handlebars_transform,
            radius=2.0,
            height=40.0,
            radial_steps=8,
            height_steps=1,
            capped=True,
            origin=unreal.GeometryScriptPrimitiveOriginMode.CENTER
        )

    except Exception as e:
        print(f"GeometryScript generation failed (perhaps plugin not enabled): {e}")
        return

    try:
        static_mesh = asset_utils.create_static_mesh_from_dynamic_mesh(dyn_mesh, mesh_path)
        asset_utils.assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript mesh bake failed: {e}")


def main():
    base_dir = '/Game/Art/Models/Character'
    mat_dir = '/Game/Art/Materials'

    asset_utils.ensure_directory(base_dir)
    asset_utils.ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedBike"
    master_mat = asset_utils.create_master_material(master_mat_path, default_roughness=0.3, noise_scale=0.05)

    # Procedural Material Instances
    bike_mat_path = f"{mat_dir}/MI_BikeBody"
    # Create a nice red stylized color for the bike
    bike_mat = asset_utils.create_material_instance(bike_mat_path, master_mat, [0.8, 0.1, 0.1, 1.0], 0.2)

    # Programmatic 3D Modeling
    bike_mesh_path = f"{base_dir}/SM_Bike"
    generate_bike_mesh(bike_mesh_path, bike_mat)

    print("Asset generation for Main Bike Character complete.")

if __name__ == '__main__':
    main()
