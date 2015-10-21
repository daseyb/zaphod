import bpy
import os

def write_mesh(name, mesh):
    f = open(name, 'w', encoding='utf-8')
    
    f.close()

def write_material(f, mat):
    f.write("Material %s \n" % mat.name)
    f.write("  " + mat.node_tree.type + "\n")
    for node in mat.node_tree.nodes:
        f.write("  " + node.name + "\n")
        
    f.write("\n")
    
def write_camera(f, cam):
    f.write("Camera %s \n" % cam.name)
    f.write("  fov: %f \n" % cam.angle)
    f.write("  dof_distance: %f \n" % cam.dof_distance)
    f.write("  lense_radius: %f \n" % bpy.context.scene.camera.data.cycles.aperture_size)

def write_object(f, obj, path):
    mesh_file_name = obj.name + ".obj"
    
    f.write("Object %s \n" % obj.name)
    f.write("  file: %s \n" % mesh_file_name)
    if obj.active_material is not None:
        f.write("  material: %s \n" % obj.active_material.name)
    
    mesh = obj.to_mesh(bpy.context.scene, True, "RENDER")
    write_mesh(path + "/" + mesh_file_name, mesh)
    
def write_zaphod_scene_file(context, filepath, use_some_setting):
    print("running write_zaphod_scene_file...")
    path = os.path.dirname(filepath)
    
    f = open(filepath, 'w', encoding='utf-8')
    
    for mat in bpy.data.materials:
        write_material(f, mat)
    
    for obj in bpy.data.objects:
        if obj.type == "MESH":
            write_object(f, obj, path)
        elif obj.type == "CAMERA":
            write_camera(f, obj.data)
            
        f.write("  position: %f %f %f \n" % (obj.location[0], obj.location[1], obj.location[2]))
        f.write("  rotation: %f %f %f \n" % (obj.rotation_euler[0], obj.rotation_euler[1], obj.rotation_euler[2])) 
        f.write("  scale: %f %f %f \n" % (obj.scale[0], obj.scale[1], obj.scale[2]))
        f.write("\n")
        
    f.close()

    return {'FINISHED'}

# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator


class ExportZaphodSceneFile(Operator, ExportHelper):
    """This appears in the tooltip of the operator and in the generated docs"""
    bl_idname = "export_test.zaphod_scene_file"  # important since its how bpy.ops.import_test.some_data is constructed
    bl_label = "Export Zaphod Scene File"

    # ExportHelper mixin class uses this
    filename_ext = ".zsf"

    filter_glob = StringProperty(
            default="*.zsf",
            options={'HIDDEN'},
            )

    # List of operator properties, the attributes will be assigned
    # to the class instance from the operator settings before calling.
    use_setting = BoolProperty(
            name="Example Boolean",
            description="Example Tooltip",
            default=True,
            )

    type = EnumProperty(
            name="Example Enum",
            description="Choose between two items",
            items=(('OPT_A', "First Option", "Description one"),
                   ('OPT_B', "Second Option", "Description two")),
            default='OPT_A',
            )

    def execute(self, context):
        return write_zaphod_scene_file(context, self.filepath, self.use_setting)


# Only needed if you want to add into a dynamic menu
def menu_func_export(self, context):
    self.layout.operator(ExportZaphodSceneFile.bl_idname, text="Zaphod Export")


def register():
    bpy.utils.register_class(ExportZaphodSceneFile)
    bpy.types.INFO_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportZaphodSceneFile)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()

    # test call
    bpy.ops.export_test.zaphod_scene_file('INVOKE_DEFAULT')
    

