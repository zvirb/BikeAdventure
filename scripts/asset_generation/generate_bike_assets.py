import unreal
import sys

def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

def create_master_material(mat_path):
    if unreal.EditorAssetLibrary.does_asset_exist(mat_path):
        return unreal.EditorAssetLibrary.load_asset(mat_path)

    mat_factory = unreal.MaterialFactoryNew()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    pkg_path = '/'.join(mat_path.split('/')[:-1])
    asset_name = mat_path.split('/')[-1]

    mat = asset_tools.create_asset(
        asset_name=asset_name,
        package_path=pkg_path,
        asset_class=unreal.Material,
        factory=mat_factory
    )

    if not mat:
        return None

    # Base Color Parameter
    base_color_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionVectorParameter, -400, 0
    )
    base_color_node.set_editor_property("parameter_name", "BaseColor")
    base_color_node.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))

    # Roughness Parameter
    roughness_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionScalarParameter, -400, 200
    )
    roughness_node.set_editor_property("parameter_name", "Roughness")
    roughness_node.set_editor_property("default_value", 0.3)

    # Simple Noise Node to add stylized gradient/noise feel
    noise_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionNoise, -400, -200
    )
    noise_node.set_editor_property("scale", 0.05)

    # Multiply Noise with BaseColor
    multiply_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionMultiply, -200, 0
    )

    # Connect BaseColor and Noise to Multiply
    unreal.MaterialEditingLibrary.connect_material_expressions(
        base_color_node, "", multiply_node, "A"
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(
        noise_node, "", multiply_node, "B"
    )

    unreal.MaterialEditingLibrary.connect_material_property(
        multiply_node, "", unreal.MaterialProperty.MP_BASE_COLOR
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness_node, "", unreal.MaterialProperty.MP_ROUGHNESS
    )

    unreal.MaterialEditingLibrary.recompile_material(mat)
    return mat

def create_material_instance(mat_inst_path, parent_mat, color, roughness):
    if unreal.EditorAssetLibrary.does_asset_exist(mat_inst_path):
        mat_inst = unreal.EditorAssetLibrary.load_asset(mat_inst_path)
    else:
        mat_inst_factory = unreal.MaterialInstanceConstantFactoryNew()
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

        pkg_path = '/'.join(mat_inst_path.split('/')[:-1])
        asset_name = mat_inst_path.split('/')[-1]

        mat_inst = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=pkg_path,
            asset_class=unreal.MaterialInstanceConstant,
            factory=mat_inst_factory
        )

        if mat_inst and parent_mat:
            unreal.MaterialEditingLibrary.set_material_instance_parent(mat_inst, parent_mat)

    if mat_inst:
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            mat_inst, "BaseColor", unreal.LinearColor(color[0], color[1], color[2], color[3])
        )
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            mat_inst, "Roughness", roughness
        )
        unreal.MaterialEditingLibrary.update_material_instance(mat_inst)

    return mat_inst

def assign_material_to_mesh(static_mesh, material_instance):
    if not static_mesh or not material_instance:
        return

    # Standard approach to set material on a static mesh in UE5 Python
    materials = static_mesh.get_editor_property('static_materials')

    if not materials or len(materials) == 0:
        # Create a new element
        new_mat = unreal.StaticMaterial()
        new_mat.material_interface = material_instance
        materials = [new_mat]
    else:
        # Update first element
        materials[0].material_interface = material_instance

    static_mesh.set_editor_property('static_materials', materials)

def generate_bike_mesh(mesh_path, mat_inst):
    if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        print(f"Asset already exists: {mesh_path}")
        return

    try:
        # Using correct UE5 Blueprint Library naming convention for Python
        dyn_mesh = unreal.GeometryScriptLibrary_CreateNewDynamicMesh.create_new_dynamic_mesh()
    except AttributeError:
        try:
            dyn_mesh = unreal.GeometryScript_AssetUtils.create_new_dynamic_mesh()
        except AttributeError:
            dyn_mesh = unreal.DynamicMesh()

    options = unreal.GeometryScriptPrimitiveOptions()

    try:
        # GeometryScript generation logic

        append_cylinder = None
        append_box = None

        if hasattr(unreal, "GeometryScriptLibrary_MeshPrimitiveFunctions"):
            append_cylinder = unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_cylinder
            append_box = unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box
        else:
            append_cylinder = unreal.GeometryScript_MeshPrimitiveFunctions.append_cylinder
            append_box = unreal.GeometryScript_MeshPrimitiveFunctions.append_box

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

    # Save Dynamic Mesh to Static Mesh Asset
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
        print(f"GeometryScript mesh bake failed: {e}")


def main():
    base_dir = '/Game/Art/Models/Character'
    mat_dir = '/Game/Art/Materials'

    ensure_directory(base_dir)
    ensure_directory(mat_dir)

    # Procedural Material Generation
    master_mat_path = f"{mat_dir}/M_StylizedBike"
    master_mat = create_master_material(master_mat_path)

    # Procedural Material Instances
    bike_mat_path = f"{mat_dir}/MI_BikeBody"
    # Create a nice red stylized color for the bike
    bike_mat = create_material_instance(bike_mat_path, master_mat, [0.8, 0.1, 0.1, 1.0], 0.2)

    # Programmatic 3D Modeling
    bike_mesh_path = f"{base_dir}/SM_Bike"
    generate_bike_mesh(bike_mesh_path, bike_mat)

    print("Asset generation for Main Bike Character complete.")

if __name__ == '__main__':
    main()
