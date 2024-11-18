#
# Copyright (c) 2024 Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
import bpy
import json
import os
import sys
import math
import mathutils
import subprocess

bl_info = {
    "name": "ZeroZero Engine",
    "description": "Scene edition & export for the ZeroZero engine",
    "author": "Henri Michelon",
    "doc_url": "https://henrimichelon.github.io/ZeroZero/",
    "tracker_url" : "https://github.com/HenriMichelon/zero_zero/issues",
    "version": (1, 0, 0),
    "blender": (4, 2, 0),
    "category": "Game Engine",
}

#----------------------------------------------------------------------------------------------------------------------
RESOURCES_ID = "resources"
NODES_TYPE   = {
    "EMPTY" : "node",
    "MESH"  : "mesh"
}
APP_URI      = "app://"
EXPORT_EXT   = ".glb"
Z_EXPORT_EXT = ".zscene"

# Drop down list value of ZeroZero built-in classes for nodes properties
NODES_CLASSES = [
    ("Camera", "Camera", "Camera"),
    ("Character", "Character", "Character"),
    ("CollisionArea", "CollisionArea", "CollisionArea"),
    ("DirectionalLight", "DirectionalLight", "DirectionalLight"),
    ("Environment", "Environment", "Environment"),
    ("KinematicBody", "KinematicBody", "KinematicBody"),
    ("MeshInstance", "MeshInstance", "MeshInstance"),
    ("Node", "Node", "Node"),
    ("OmniLight", "OmniLight", "OmniLight"),
    ("PhysicsBody", "PhysicsBody", "PhysicsBody"),
    ("RayCast", "RayCast", "RayCast"),
    ("RigidBody", "RigidBody", "RigidBody"),
    ("Skybox", "Skybox", "Skybox"),
    ("SpotLight", "SpotLight", "SpotLight"),
    ("StaticBody", "StaticBody", "StaticBody"),
    ("Viewport", "Viewport", "Viewport")
]

# Drop down list value of BCn compression formats for scene export
COMPRESSION_FORMATS = [
    ("bc1", "BC1 (DXT1)", "bc1"), 
    ("bc2", "BC2 (DXT3)", "bc2"),
    ("bc3", "BC3 (DXT5)", "bc3"),
    ("bc7", "BC7", "bc7") 
  ]


#----------------------------------------------------------------------------------------------------------------------

# converts a vec into a string
def convert_vector(vec):
    return str(vec.x) + "," + str(vec.z) + "," + str(-vec.y)

# converts a vec in degrees then into a string
def convert_vector_degrees(vec):
    return str(-math.degrees(vec.x)) + "," + str(math.degrees(vec.z)) + "," + str(-math.degrees(vec.y))

def show_message(message="", title="Error", icon='ERROR'):
    def draw(self, context):
        self.layout.label(text=message)
    bpy.context.window_manager.popup_menu(draw, title=title, icon=icon)


#----------------------------------------------------------------------------------------------------------------------

# adds a resource or a mesh node to the JSON scene file
def add_resource(nodes, obj, parent):
    if parent is None:
        path=obj.name
    else:
        path=parent + "/" + obj.name
    if obj.type == "MESH":
        nodes.append({
            "id": obj.name + ".mesh",
            "type" : NODES_TYPE[obj.type],
            "resource" : RESOURCES_ID,
            "path": path
        })
    for child in obj.children:
        add_resource(nodes, child, path)

# adds a node to the JSON scene file
def add_node(obj):
#    print(obj.name + ":" + obj.type)
    node = { "id": obj.name }
    if "zero_zero_props" in obj:
        props = obj.zero_zero_props
        if "custom_class_name" in props and props.custom_class_name != "":
            node["class"] = props.custom_class_name
        else:
            if "class_name" in props:
                if props.class_name != "Node":
                    node["class"] = props.class_name
        if "properties" in props:
            custom_props = {}
            for custom_prop in props.properties:
                custom_props[custom_prop.name] = custom_prop.value.replace("$$", obj.name)
            node["properties"] = custom_props
    if obj.type == "MESH":
        node["child"] = { "id": obj.name + ".mesh" }
    if obj.type == "LIGHT":
        original_rotation = obj.rotation_euler
        conversion_matrix = mathutils.Matrix.Rotation(math.radians(-90), 3, 'X')
        new_rotation = original_rotation.to_matrix() @ conversion_matrix
        node["properties"] = {
            "color" : str(obj.data.color.r) + "," + str(obj.data.color.g) + "," + str(obj.data.color.b) + "," + str(obj.data.energy/10.0),
            "position" : convert_vector(obj.location),
            "rotation" : convert_vector_degrees(new_rotation.to_euler('XYZ'))
        }
        if obj.data.use_shadow:
            node["properties"]["cast_shadows"] = "true"
        if obj.data.type == "POINT":
            node["class"] = "OmniLight"
            if obj.data.use_custom_distance:
                node["properties"]["range"] = str(obj.data.cutoff_distance)
        if obj.data.type == "SUN":
            node["class"] = "DirectionalLight"
        if obj.data.type == "SPOT":
            node["class"] = "SpotLight"
            node["properties"]["fov"] = str(obj.data.spot_size)
    if obj.children:
        node["children"] = [add_node(child) for child in obj.children]
    return node

# Exports the Blender scene to a JSON scene
def export_json():
    settings = bpy.context.scene.zero_zero_settings
    directory = settings.models_directory
    filename = os.path.splitext(os.path.basename(bpy.data.filepath))[0]
    if settings.convert_zscene:
        ext = Z_EXPORT_EXT
    else:
        ext = EXPORT_EXT
    filename = APP_URI + directory + "/" + filename.replace("\\", "/") + ext
    nodes = [{
        "id" : RESOURCES_ID,
        "type" : "model",
        "resource" : filename
    }]
    for obj in bpy.context.scene.objects:
        if obj.parent is None:
            add_resource(nodes, obj, obj.parent)
    
    blend_file_path = bpy.data.filepath
    blend_file_name = os.path.basename(blend_file_path)
    scene_name = os.path.splitext(blend_file_name)[0]
    for obj in bpy.context.scene.objects:
        if obj.parent is None:
            nodes.append(add_node(obj))
    scene = { "id": scene_name }
    if nodes:
        scene["children"] = nodes
    return { "nodes" : nodes}



#----------------------------------------------------------------------------------------------------------------------

class ExportOperator(bpy.types.Operator):
    bl_idname = "object.zero_zero_export_operator"
    bl_label = "Export as ZeroZero scene"

    def execute(self, context):
        settings = bpy.context.scene.zero_zero_settings
        blend_file_path = bpy.data.filepath
        #print("file: " + blend_file_path)
        if blend_file_path == "":
            show_message("Please save the blender project first")
            return {'FINISHED'}
        if settings.project_directory == "":
            show_message("Please set the project directory in the scene properties first")
            return {'FINISHED'}
        if settings.scene_directory == "":
            show_message("Please set the project scene directory in the scene properties first")
            return {'FINISHED'}
        if settings.models_directory == "":
            show_message("Please set the project models directory in the scene properties first")
            return {'FINISHED'}
        gltf2zscene = settings.gltf2zscene_path + "/gltf2zscene.exe";
        if settings.convert_zscene and not os.path.isfile(gltf2zscene):
            show_message("Please set the gltf2zscene directory in the scene properties first")
            return {'FINISHED'}
        blend_file_name = os.path.basename(blend_file_path)
        file_name = os.path.splitext(blend_file_name)[0];
        export_file_name = file_name + EXPORT_EXT
        scene_dir = settings.scene_directory.replace("/", "\\")
        models_dir = settings.models_directory.replace("/", "\\")
        export_models_path = os.path.join(settings.project_directory, models_dir)
        export_scene_path = os.path.join(settings.project_directory, scene_dir)
        glb_export_path = os.path.join(export_models_path, export_file_name)
        json_scene_export_path = os.path.join(export_scene_path, file_name + ".json")

        print("--------------------------------------------")
        bpy.context.window.cursor_set("WAIT")
        #self.report({'INFO'}, "Saving " + blend_file_name);
        #bpy.ops.wm.save_mainfile()

        #self.report({'INFO'}, "Exporting to " + glb_export_path);
        bpy.ops.export_scene.gltf(filepath=glb_export_path, export_format='GLB')

        #self.report({'INFO'}, "Generating " + json_scene_export_path);
        result = export_json()
        with open(json_scene_export_path, 'w') as json_file:
            json.dump(result, json_file, indent=2)
        
        if settings.convert_zscene:
            zscene_export_path = os.path.join(export_models_path,  file_name + Z_EXPORT_EXT)
            bpy.context.window.cursor_set("WAIT")
            subprocess.run([
                gltf2zscene,
                "-v",
                "-t", str(settings.gltf2zscene_threads),
                "-f", settings.gltf2zscene_format,
                glb_export_path, 
                zscene_export_path])
            os.remove(glb_export_path)

        self.report({'INFO'}, "Exported to " + settings.scene_directory)
        bpy.context.window.cursor_set("DEFAULT")
        return {'FINISHED'}


class CustomSettings(bpy.types.PropertyGroup):
    models_directory: bpy.props.StringProperty(
        name="Models",
        description="The models resource directory, relative to the project directory",
        default="res/models"
    )
    scene_directory: bpy.props.StringProperty(
        name="Scene",
        description="The scene resource directory, relative to the project directory",
        default="res/scenes"
    )
    project_directory: bpy.props.StringProperty(
        name="Project",
        description="The game project directory",
        default=""
    )
    convert_zscene: bpy.props.BoolProperty(
        name="Export to ZScene",
        description="Convert the exported GLB file to a ZScene file then delete the GLB",
        default=False
    )
    gltf2zscene_path: bpy.props.StringProperty(
        name="gltf2zcene",
        description="The directory of the gltf2zscene executable",
        default=""
    )
    gltf2zscene_threads: bpy.props.IntProperty(
        name="Threads",
        description="Number of threads for gltf2zscene executable (0 = auto)",
        default=0,
        min=0,
        max=20
    )
    gltf2zscene_format: bpy.props.EnumProperty(
        name="Format",
        description="Compression format for color textures",
        items=COMPRESSION_FORMATS,
        default="bc7"
    )


class CustomPropertyProperties(bpy.types.PropertyGroup):
    name: bpy.props.StringProperty(
        name="Name",
        default=""
    )
    value: bpy.props.StringProperty(
        name="Value",
        default=""
    )

class CustomProperties(bpy.types.PropertyGroup):
    class_name: bpy.props.EnumProperty(
        name="Class",
        description="The Node's clas",
        items=NODES_CLASSES,
        default="Node"
    )
    custom_class_name: bpy.props.StringProperty(
        name="Custom class",
        description="The Node's class",
        default="",
    )
    properties: bpy.props.CollectionProperty(type=CustomPropertyProperties)


# Operator to add a custom property
class OBJECT_OT_AddCustomProperty(bpy.types.Operator):
    bl_label = "Add Custom Property"
    bl_idname = "object.zero_zero_add_custom_property"

    def execute(self, context):
        obj = context.object
        prop = obj.zero_zero_props.properties.add()
        prop.key = ""
        prop.value = ""
        return {'FINISHED'}

# Operator to remove a custom property
class OBJECT_OT_RemoveCustomProperty(bpy.types.Operator):
    bl_label = "Remove Custom Property"
    bl_idname = "object.zero_zero_remove_custom_property"
    index: bpy.props.IntProperty()

    def execute(self, context):
        obj = context.object
        obj.zero_zero_props.properties.remove(self.index)
        return {'FINISHED'}


class ObjectPanel(bpy.types.Panel):
    """Creates a Panel in the Object properties window"""
    bl_label = "ZeroZero Node"
    bl_idname = "OBJECT_PT_custom"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    def draw(self, context):
        layout = self.layout
        object = context.object
        props = object.zero_zero_props
        layout.prop(props, "class_name")
        layout.prop(props, "custom_class_name")

        index = 0
        for prop in object.zero_zero_props.properties:
            row = layout.row()
            row.prop(prop, "name")
            row.prop(prop, "value")
            row.operator("object.zero_zero_remove_custom_property", text="Remove").index = index
            index = index + 1
        layout.operator("object.zero_zero_add_custom_property")


class ScenePanel(bpy.types.Panel):
    """Creates a Panel in the Scene properties window"""
    bl_label = "ZeroZero project"
    bl_idname = "SCENE_PT_custom"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"

    def draw(self, context):
        layout = self.layout
        scene = context.scene
        settings = scene.zero_zero_settings
        layout.prop(settings, "project_directory")
        layout.prop(settings, "scene_directory")
        layout.prop(settings, "models_directory")
        layout.prop(settings, "convert_zscene")
        if settings.convert_zscene:
            layout.prop(settings, "gltf2zscene_path")
            layout.prop(settings, "gltf2zscene_format")
            layout.prop(settings, "gltf2zscene_threads")
        layout.operator("object.zero_zero_export_operator")


def add_keymap():
    wm = bpy.context.window_manager
    km = wm.keyconfigs.active.keymaps.get("3D View")
    # Define a new keymap item
    if km:
        kmi = km.keymap_items.new(ExportOperator.bl_idname, 'E', 'PRESS', ctrl=True)
        kmi.active = True

def register():
    bpy.utils.register_class(ExportOperator)
    bpy.utils.register_class(CustomSettings)
    bpy.utils.register_class(CustomPropertyProperties)
    bpy.utils.register_class(CustomProperties)
    bpy.types.Object.zero_zero_props = bpy.props.PointerProperty(type=CustomProperties)
    bpy.types.Scene.zero_zero_settings = bpy.props.PointerProperty(type=CustomSettings)
    bpy.utils.register_class(ScenePanel)
    bpy.utils.register_class(ObjectPanel)
    bpy.utils.register_class(OBJECT_OT_AddCustomProperty)
    bpy.utils.register_class(OBJECT_OT_RemoveCustomProperty)
    add_keymap()

def unregister():
    bpy.utils.unregister_class(OBJECT_OT_AddCustomProperty)
    bpy.utils.unregister_class(OBJECT_OT_RemoveCustomProperty)
    bpy.utils.unregister_class(ObjectPanel)
    bpy.utils.unregister_class(ScenePanel)
    bpy.utils.unregister_class(CustomSettings)
    bpy.utils.unregister_class(CustomProperties)
    bpy.utils.unregister_class(CustomPropertyProperties)
    bpy.utils.unregister_class(ExportOperator)


#-------------------------------------------------------------------------------------
print("---------------------------")
if __name__ == "__main__":
    register()
#    result = export_json()
#    json.dump(result, sys.stdout, indent=4);
    

