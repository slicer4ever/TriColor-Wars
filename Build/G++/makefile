BaseDir = ../../
LightWave = $(BaseDir)../Lightwave/
IncludeDirs = -I$(BaseDir)Includes/C++11/ -I$(BaseDir)Includes/C++11/ -I$(LightWave)Framework/Includes/C++11/ -I$(LightWave)Dependency/Includes/C++11/ -I$(LightWave)Engine/Includes/C++11/
CFlags = -std=c++14 -static-libstdc++ -pthread -Wall -Wfatal-errors $(IncludeDirs)
CC = g++ $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = g++ -g $(CFlags)
endif
PlatformTarget = $(shell arch)
Target = $(BaseDir)/Binarys/$(Config)/$(PlatformTarget)/
LWFramework = $(LightWave)Framework/Binarys/$(Config)/$(PlatformTarget)/
LWEngine = $(LightWave)Engine/Binarys/$(Config)/$(PlatformTarget)/
Dependency = $(LightWave)/Dependency/Binarys/$(Config)/$(PlatformTarget)/

TargetName = Neosnake
Libs = -lLWEngine -lLWNetwork -lLWCore -lLWPlatform -lLWVideo -lLWAudio -lvorbis -logg -lGLEW -lpng -lz -lX11 -lXrandr -lGL -lpulse -lfreetype -lresolv
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
	$(CC) $(Sources) $(LibPath) $(Libs) -o $(Target)$(TargetName)
Dirs:
	mkdir -p $(Target)
clean:
	rm -f $(Target)$(TargetName)
