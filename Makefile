OBJS = main.cpp
CC = g++
INCLUDE_PATHS = -IC:\mingw64\include\SDL2 -I${CURDIR}/include/
LOCAL_LDFLAGS = -Wl,--exclude-libs,ALL, -Wl,--gc-sections -s -fvisibility=hidden
LIBRARY_PATHS = -LC:\mingw64\lib
#-fno-rtti 
COMPILER_FLAGS = -w -Os -g0 -Wl,-subsystem,windows -Wl,--exclude-libs,ALL -Wl,--gc-sections -shared -masm=intel
#LINKER_FLAGS = -fvisibility=hidden -s -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lopengl32 -lMinHook 
LINKER_FLAGS = -s -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lopengl32 -lMinHook -lglew32 -lglew32s
OBJ_NAME = "C:\Program Files (x86)\Steam\steamapps\common\Banished\sandbox.asi"
all : $(OBJS)
	cls && $(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)