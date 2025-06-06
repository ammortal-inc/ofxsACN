meta:
	ADDON_NAME = ofxsACN
	ADDON_DESCRIPTION = sACN E1.31 2018
	ADDON_AUTHOR = Ben Snell
	ADDON_TAGS = "sacn" "e131" "dmx" "e1.31" "streaming"
	ADDON_URL = https://github.com/ammortal-inc/ofxsACN

common:

	# dependencies with other addons, a list of them separated by spaces 
	# or use += in several lines
	# ADDON_DEPENDENCIES = 
	
	# include search paths, this will be usually parsed from the file system
	# but if the addon or addon libraries need special search paths they can be
	# specified here separated by spaces or one per line using +=
	ADDON_INCLUDES = 
	ADDON_INCLUDES += src
	ADDON_INCLUDES += include
	ADDON_INCLUDES += libs
	
	# any special flag that should be passed to the compiler when using this
	# addon
	# ADDON_CFLAGS = 
	
	# any special flag that should be passed to the linker when using this
	# addon, also used for system libraries with -lname
	# ADDON_LDFLAGS =
	
	# linux only, any library that should be included in the project using
	# pkg-config
	# ADDON_PKG_CONFIG_LIBRARIES =
	
	# osx/iOS only, any framework that should be included in the project
	# ADDON_FRAMEWORKS =
	
	# source files, these will be usually parsed from the file system looking
	# in the src folders in libs and the root of the addon. if your addon needs
	# to include files in different places or a different set of files per platform
	# they can be specified here
	# ADDON_SOURCES = 
	
	# some addons need resources to be copied to the bin/data folder of the project
	# specify here any files that need to be copied, you can use wildcards like * and ?
	# ADDON_DATA = 
	
	# a specific platform
	# ADDON_LIBS_EXCLUDE =

linux:
	ADDON_LIBS = libs/libsACN.a libs/libEtcPal.a

linux64:
	ADDON_LIBS = libs/libsACN.a libs/libEtcPal.a