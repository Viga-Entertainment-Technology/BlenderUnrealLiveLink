import bpy
from socket import *

class MyProperties(bpy.types.PropertyGroup):
    my_string : bpy.props.StringProperty(name ="Name")
    my_float : bpy.props.FloatProperty(name = "Enter Value")
    my_enum : bpy.props.EnumProperty(
        name = "Mesh Type",
        description = "enum desc",
        items = [("O","Static Mesh",""),
            ("A","Skeletal Mesh","")
        ]
    )
    my_enum1 : bpy.props.EnumProperty(
        name = "Bone Type",
        description = "enum desc",
        items = [("|","Bone Transform",""),
            ("||","Armature","")
        ]
    )
    
    
    
class BlenderUELiveLink(bpy.types.Panel):
    #bl_parent_id = "BlenderUE LiveLink"
    bl_idname = ""
    bl_label = "Blender Unreal Live Link"
    bl_space_type = "VIEW_3D"   
    bl_region_type = "UI"    
    bl_category  = "UE"
    bl_options = {"DEFAULT_CLOSED"}

    def draw(self, context):
        layout = self.layout
        scene = context.scene
        mytool  = scene.my_tool

        col = layout.column(align=True)
        row = col.row(align=True)
        row=layout.row()
        row.label(text="IP Address : 127.0.0.1")
        row=layout.row()
        row.label(text="Port : 2000")
        
        layout.prop(mytool,"my_enum")
        layout.prop(mytool,"my_enum1")        
        layout.prop(mytool,"my_string")
        row=layout.row()
        row.operator("wm.modal_timer_operator", text="Start Live Link")
        # row=layout.row()
        # row.prop(context.scene, prop_name)

class ModalTimerOperator(bpy.types.Operator):
    """Operator which runs its self from a timer"""
    bl_idname = "wm.modal_timer_operator"
    bl_label = "Modal Timer Operator"
    host = "127.0.0.1" # set to IP address of target computer
    port = 2000
    addr = (host, port)
    UDPSock = socket(AF_INET, SOCK_DGRAM) 
    _timer = None

    def modal(self, context, event):
        if event.type in {'RIGHTMOUSE', 'ESC'}:
            self.cancel(context)
            return {'CANCELLED'}

        if event.type == 'TIMER':
            mytool = context.scene.my_tool
            message = mytool.my_enum + "_"+f"{mytool.my_string}="
            #bpy.data.objects["Cube"] 
            if(mytool.my_enum=="O"):
                message+="(" + str(bpy.data.objects[mytool.my_string].location.x) + "," + str(bpy.data.objects[mytool.my_string].location.y) +  "," + str(bpy.data.objects[mytool.my_string].location.z) +  "," + str(bpy.data.objects[mytool.my_string].rotation_quaternion.x) +  "," + str(bpy.data.objects[mytool.my_string].rotation_quaternion.y )+  "," + str(bpy.data.objects[mytool.my_string].rotation_quaternion.z) + "," + str(bpy.data.objects[mytool.my_string].rotation_quaternion.w)+ ")" + "||"            
            else:
                count = 0
                for i in bpy.data.objects[mytool.my_string].pose.bones:
                    #if(count < 3):
                    #    count = count + 1
                    #else:
                    #    break
                    #boneEdit = bpy.data.armatures['root'].bones[i.name].matrix_local.to_quaternion()
                    obj = i.id_data
                    matrix_final = obj.matrix_world @ i.matrix
                    locationWS = i.location * 100.0
                    quaternionWS =i.rotation_quaternion# matrix_final.to_quaternion()
                    #quaternionWS = i.rotation_quaternion * boneEdit
                    #print(quaternionWS)
                    
                    message+=i.name + ":(" + "{:.9f}".format(locationWS.x)+ "," + "{:.9f}".format(locationWS.y) +  "," + "{:.9f}".format(locationWS.z) +  "," + "{:.9f}".format(-quaternionWS.x) +  "," + "{:.9f}".format(quaternionWS.y)+  "," + "{:.9f}".format(-quaternionWS.z)+ "," + "{:.9f}".format(quaternionWS.w)+ ")" + "|"
                message = message + "|"
            print(message)
            self.UDPSock.sendto(message.encode(), self.addr)
            # change theme color, silly!
            color = context.preferences.themes[0].view_3d.space.gradients.high_gradient
            color.s = 1.0
            color.h += 0.01
            
        return {'PASS_THROUGH'}

    def execute(self, context):
        wm = context.window_manager
        self._timer = wm.event_timer_add(0.1, window=context.window)
        wm.modal_handler_add(self)
        return {'RUNNING_MODAL'}

    def cancel(self, context):
        wm = context.window_manager
        wm.event_timer_remove(self._timer)



classes = [MyProperties,ModalTimerOperator,BlenderUELiveLink]
def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    bpy.types.Scene.my_tool = bpy.props.PointerProperty(type = MyProperties)
    

def unregister():
    for cls in classes:
        bpy.utils.unregister_class(cls)
    del bpy.types.Scene.my_tool


if __name__ == "__main__":
    register()

    # test call
