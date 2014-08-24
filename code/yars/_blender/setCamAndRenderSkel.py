import bpy
import sys
import os

##############################################################
#sandboxpath = os.path.join(os.path.dirname(bpy.data.filepath), "..") + "\\"
sandboxpath = 'C:/dati_cloud/GitHub/YARS/code/yars/'
filename  = 'sponza'

#filename = bpy.path.basename(bpy.data.filepath)
#filename = os.path.splitext(filename)[0]

##############################################################

pi = 3.14159265

scene = bpy.data.scenes["Scene"]

# Set render resolution
scene.render.resolution_x = WIDTH
scene.render.resolution_y = HEIGHT

# Set camera fov in degrees
scene.camera.data.angle = FOV * (pi/180.0)

# Set camera rotation in euler angles
scene.camera.rotation_mode = 'XYZ'
scene.camera.rotation_euler[0] = ROT_X * (pi/180.0)
scene.camera.rotation_euler[1] = ROT_Y * (pi/180.0)
scene.camera.rotation_euler[2] = ROT_Z * (pi/180.0)

# Set camera translation
scene.camera.location.x = POS_X
scene.camera.location.y = POS_Y
scene.camera.location.z = POS_Z

##############################################################

scene.world.ambient_color = (0.0, 0.0, 0.0)

scene.world.light_settings.use_ambient_occlusion = True
scene.world.light_settings.ao_factor = 1.0
scene.world.light_settings.ao_blend_type = 'ADD'

scene.world.light_settings.use_environment_light = True
scene.world.light_settings.environment_energy = 1.0
scene.world.light_settings.environment_color  = 'PLAIN' # 'SKY_COLOR' | 'SKY_TEXTURE'

scene.world.light_settings.gather_method = 'RAYTRACE' # 'APPROXIMATE' 

scene.world.light_settings.sample_method = 'CONSTANT_QMC' # 'CONSTANT_JITTERED' | 'ADAPTIVE_QMC'
scene.world.light_settings.samples = 64 # SAMPLES

scene.world.light_settings.distance = DIST_MAX

scene.world.light_settings.use_falloff = False
scene.world.light_settings.falloff_strength = 0.0

scene.render.use_shadows  = True
scene.render.use_textures = False
##############################################################

outfolderpath = sandboxpath + '/_test/'
ssimpath = sandboxpath + '/_test/SSIM/'



fileprops = ("(%.3f_%.3f_%.3f)" % (POS_X, POS_Y, POS_Z) +
             "(%.3f_%.3f_%.3f)" % (ROT_X, ROT_Y, ROT_Z) +
			 "%dx%d" % (WIDTH, HEIGHT) + '_' +
#			 "samples%d" % (SAMPLES) + '_' +
			 "maxdist%.3f" % (DIST_MAX) + '_'
			 )
fileGLsuffix = 'GL'
fileRTsuffix = 'RT'
			  
fullbasename = outfolderpath + filename + fileprops

print(fullbasename + fileRTsuffix+'.png')

bpy.context.scene.render.filepath = fullbasename + fileRTsuffix+'.png'
#bpy.ops.render.render(write_still = True)

#ssimcmd = (ssimpath + 'ssim.exe '
#          + fullbasename + fileGLsuffix+'.png '
#		  + fullbasename + fileRTsuffix+'.png '
#		  + ' > ' + fullbasename + 'CMP.txt')
		  
ssimcmd = ('for %%f in ("' + fullbasename + fileGLsuffix + '*.png") do (\n'
          + 'echo %%f >> "' + fullbasename + 'CMP.txt"\n'
		  +	ssimpath + 'ssim.exe '
          + '%%f '
		  + '"' + fullbasename + fileRTsuffix+'.png" '
		  + ' >> "' + fullbasename + 'CMP.txt"\n'
          + 'type %%f.csv >> "' + fullbasename + 'CMP.txt"\n'
		  + ')')
print(ssimcmd)

text_file = open(fullbasename + "CMP.bat", "w")
text_file.write(ssimcmd)
text_file.close()

#os.system(ssimcmd)
#os.system(fullbasename+"CMP.bat")
