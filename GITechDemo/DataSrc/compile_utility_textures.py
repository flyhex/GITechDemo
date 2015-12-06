﻿#=============================================================================
#	This file is part of the "GITechDemo" application
#	Copyright (C) 2015 Iftode Bogdan-Marius <iftode.bogdan@gmail.com>
#
#		File:	compile_utility_textures.py
#		Author:	Bogdan Iftode
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program. If not, see <http://www.gnu.org/licenses/>.
#=============================================================================

import os
import subprocess
import time
import sys
from collections import defaultdict

#############################################################################
#		Arguments:															#
#---------------------------------------------------------------------------#
#	'win32' to use the 32 bit version of the tools							#
#	'x64' to use the 64 bit version of the tools (default)					#
#	'rebuild' to force rebuilding all data assets							#
#############################################################################

defaultArchitecture = "x64"
defaultForceRebuild = False

# Process command arguments
for opt in sys.argv:
	if(opt.lower() == "x64"):
		defaultArchitecture = "x64"
	if(opt.lower() == "win32"):
		defaultArchitecture = "Win32"
	if(opt.lower() == "rebuild"):
		defaultForceRebuild = True

# Set paths
pathToTextureFiles = "textures/"
outputPath = "../Data/textures/"
textureCompilerExe = "../Bin/" + defaultArchitecture + "/Release/Synesthesia3DTools/TextureCompiler.exe"

# Set custom arguments for individual files
customArgs = defaultdict(lambda: "-q -f A8R8G8B8", \
{
	"sky.dds"					: "-q -f A16B16G16R16F",
	"LensFlareDirt.png"			: "-q -f A8R8G8B8 -mip 1",
	"LensFlareStarBurst.png"	: "-q -f A8R8G8B8 -mip 1",
	"bayer_matrix.dds"			: "-q -f L8 -mip 1",
	"noise.dds"					: "-q -f L8 -mip 1"
})

# Detect modification of texture compiler executable
if os.path.getmtime(textureCompilerExe) > os.path.getmtime(os.path.realpath(__file__)):
	print "Texture compiler (\"" + textureCompilerExe + "\") modification detected. Forcing rebuild of all texture assets"
	defaultForceRebuild = True

start = time.clock()

# Compile textures
for root, dir, files in os.walk(pathToTextureFiles):
	for name in files:
		if name != 'Thumbs.db':
			sourceFileIsNewer = os.path.getmtime(os.path.join(root, name)) > os.path.getmtime(os.path.realpath(__file__))
			compiledFileExists = os.path.isfile(outputPath + os.path.splitext(name)[0] + ".s3dtex")
			if sourceFileIsNewer or not compiledFileExists or defaultForceRebuild:
				print "Compiling texture \"" + os.path.join(root, name) + "\""
				subprocess.call(textureCompilerExe + " " + customArgs[name] + " -d " + outputPath + " " + os.path.join(root, name))
			else:
				print "Texture \"" + os.path.join(root, name) + "\" is up-to-date"

print "Done in " + str(time.clock() - start) + " seconds.";

os.utime(os.path.realpath(__file__), None)
