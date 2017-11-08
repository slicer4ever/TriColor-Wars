BaseDir = ../../
LightWave = $(BaseDir)../Lightwave/
IncludeDirs = -I$(BaseDir)Includes/C++11/ -I$(BaseDir)Includes/C++11/ -I$(LightWave)Framework/Includes/C++11/ -I$(LightWave)Dependency/Includes/C++11/ -I$(LightWave)Engine/Includes/C++11/
CFlags = -std=c++14 -static-libstdc++ -pthread -Wall -Wfatal-errors $(IncludeDirs)
EFlags = -s USE_LIBPNG=1 -s USE_FREETYPE=1 -s USE_ZLIB=1 -s USE_LIBVORBIS=1 -s USE_LIBOGG=1 --preload-file ../../Content/ -s ALLOW_MEMORY_GROWTH=1
CC = emcc $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = emcc -g $(CFlags)
endif
PlatformTarget = JS
Target = $(BaseDir)/Binarys/$(Config)/$(PlatformTarget)/
LWFramework = $(LightWave)Framework/Binarys/$(Config)/$(PlatformTarget)/
LWEngine = $(LightWave)Engine/Binarys/$(Config)/$(PlatformTarget)/
Dependency = $(LightWave)/Dependency/Binarys/$(Config)/$(PlatformTarget)/


TargetName = TriColor-Wars
ELibs = $(LWFramework)libLWEngine.bc $(LWFramework)libLWCore.bc $(LWFramework)libLWPlatform.bc $(LWFramework)libLWVideo.bc $(LWFramework)libLWAudio.bc
Libs = -lX11
LibPath = -L$(LWFramework) -L$(Dependency) -L$(LWEngine)
Obj = $(Config)/$(PlatformTarget)/
Src = $(BaseDir)Source/

Sources = $(Src)C++11/App.cpp
Sources += $(Src)C++11/Renderer.cpp
Sources += $(Src)C++11/Sprite.cpp
Sources += $(Src)C++11/State_Game.cpp
Sources += $(Src)C++11/State_Menu.cpp
Sources += $(Src)C++11/main.cpp

all: Dirs $(Sources)
	$(CC) $(Sources) $(LibPath) $(Libs) $(ELibs) -o $(Target)$(TargetName).html
Dirs:
	mkdir -p $(Target)
clean:
	rm -f $(Target)$(TargetName).html
