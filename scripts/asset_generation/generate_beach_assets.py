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
    roughness_node.set_editor_property("default_value", 0.8)

    # Noise for stylized feel
    noise_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionNoise, -400, -200
    )
    noise_node.set_editor_property("scale", 0.05)

    multiply_node = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionMultiply, -200, 0
    )

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

    materials = static_mesh.get_editor_property('static_materials')

    if not materials or len(materials) == 0:
        new_mat = unreal.StaticMaterial()
        new_mat.material_interface = material_instance
        materials = [new_mat]
    else:
        materials[0].material_interface = material_instance

    static_mesh.set_editor_property('static_materials', materials)

def generate_palm_tree(mesh_path, mat_inst):
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
            # Trunk
            trunk_transform = unreal.Transform()
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_cylinder(
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

                unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
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
        else:
            # Trunk
            trunk_transform = unreal.Transform()
            unreal.GeometryScript_MeshPrimitiveFunctions.append_cylinder(
                target_mesh=dyn_mesh,
                primitive_options=options,
                transform=trunk_transform,
                radius=20.0,
                height=400.0,
                radial_steps=6,
                height_steps=1,
                origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
            )
            # Palm leaves
            for i in range(4):
                leaf_transform = unreal.Transform()
                leaf_transform.translation = [0.0, 0.0, 380.0]
                leaf_transform.rotation = unreal.Rotator(0, i * 45, 20).quaternion()

                unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
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

        assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")


def generate_sandcastle(mesh_path, mat_inst):
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
            # Base
            base_transform = unreal.Transform()
            unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_box(
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
                    unreal.GeometryScriptLibrary_MeshPrimitiveFunctions.append_cylinder(
                        target_mesh=dyn_mesh,
                        primitive_options=options,
                        transform=tower_transform,
                        radius=15.0,
                        height=50.0,
                        radial_steps=6,
                        height_steps=1,
                        origin=unreal.GeometryScriptPrimitiveOriginMode.BASE
                    )
        else:
            # Base
            base_transform = unreal.Transform()
            unreal.GeometryScript_MeshPrimitiveFunctions.append_box(
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
                    unreal.GeometryScript_MeshPrimitiveFunctions.append_cylinder(
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

        assign_material_to_mesh(static_mesh, mat_inst)
    except Exception as e:
        print(f"GeometryScript bake failed: {e}")


def main():
    base_dir = '/Game/Art/Models/Beach'
    mat_dir = '/Game/Art/Materials'

    ensure_directory(base_dir)
    ensure_directory(mat_dir)

    # Material setup
    master_mat_path = f"{mat_dir}/M_StylizedBeach"
    master_mat = create_master_material(master_mat_path)

    palm_mat_path = f"{mat_dir}/MI_PalmTree"
    palm_mat = create_material_instance(palm_mat_path, master_mat, [0.1, 0.4, 0.1, 1.0], 0.7)

    sandcastle_mat_path = f"{mat_dir}/MI_Sandcastle"
    sandcastle_mat = create_material_instance(sandcastle_mat_path, master_mat, [0.8, 0.7, 0.4, 1.0], 0.9)

    # Generation
    palm_mesh_path = f"{base_dir}/SM_PalmTree"
    generate_palm_tree(palm_mesh_path, palm_mat)

    sandcastle_mesh_path = f"{base_dir}/SM_Sandcastle"
    generate_sandcastle(sandcastle_mesh_path, sandcastle_mat)

    print("Asset generation for Beach biome complete.")

if __name__ == '__main__':
    main()
