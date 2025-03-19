# Compiler
CXX = clang++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# SDL3 Paths
SDL3_INCLUDE := /opt/homebrew/Cellar/sdl3/3.2.8/include
SDL3_LIB := /opt/homebrew/Cellar/sdl3/3.2.8/lib
SDL3_IMAGE_INCLUDE := /opt/homebrew/Cellar/sdl3_image/3.2.4/include
SDL3_IMAGE_LIB := /opt/homebrew/Cellar/sdl3_image/3.2.4/lib
# Third-party SDL3 Paths 
SDL3_TTF_INCLUDE := /opt/homebrew/Cellar/sdl3_ttf/3.2.0/include
SDL3_TTF_LIB := /opt/homebrew/Cellar/sdl3_ttf/3.2.0/lib

# macOS-specific libraries
PLATFORM_LIBS = -framework Cocoa -framework OpenGL -lobjc

# Header directories
HEADER = -isystem $(SDL3_INCLUDE) \
         -I$(SDL3_IMAGE_INCLUDE) \
         -I$(SDL3_TTF_INCLUDE) \
         -Idatabase

# Library flags (removed unused libraries)
LIB_FLAGS = -L$(SDL3_LIB) -L$(SDL3_IMAGE_LIB) -L$(SDL3_TTF_LIB) \
            -lSDL3 -lSDL3_image -lSDL3_ttf \
            $(PLATFORM_LIBS) -Wl,-undefined,dynamic_lookup

# Target and source files
TARGET = AtaraxiaSDK
SRC_CPP = main.cpp
# Alternatively, if your source is in a subfolder, uncomment:
# SRC_CPP = src/main.cpp

OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ_OBJC =
OBJS = $(OBJ_CPP) $(OBJ_OBJC)

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "DEBUG: Building executable $(TARGET) with objects:" $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIB_FLAGS) -o $(TARGET)
	@echo "DEBUG: Executable $(TARGET) built successfully."

run: $(TARGET)
	@echo "DEBUG: Running executable $(TARGET)..."
	./$(TARGET)

bundle: $(TARGET)
	@echo "DEBUG: Creating app bundle..."
	@rm -rf $(TARGET).app
	@mkdir -p $(TARGET).app/Contents/MacOS
	@mkdir -p $(TARGET).app/Contents/Resources
	@mkdir -p $(TARGET).app/Contents/Frameworks
	@echo "DEBUG: Copying executable to bundle..."
	@cp $(TARGET) $(TARGET).app/Contents/MacOS/
	@echo "DEBUG: Creating Info.plist..."
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $(TARGET).app/Contents/Info.plist
	@echo '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' >> $(TARGET).app/Contents/Info.plist
	@echo '<plist version="1.0">' >> $(TARGET).app/Contents/Info.plist
	@echo '<dict>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <key>CFBundleExecutable</key>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <string>$(TARGET)</string>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <key>CFBundleIdentifier</key>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <string>com.example.$(TARGET)</string>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <key>CFBundleName</key>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <string>$(TARGET)</string>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <key>CFBundlePackageType</key>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <string>APPL</string>' >> $(TARGET).app/Contents/Info.plist
	@echo '</dict>' >> $(TARGET).app/Contents/Info.plist
	@echo '</plist>' >> $(TARGET).app/Contents/Info.plist
	@echo "DEBUG: App bundle $(TARGET).app created successfully."

clean:
	@echo "DEBUG: Cleaning build artifacts..."
	rm -f $(OBJS) $(TARGET)
	rm -rf $(TARGET).app

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(HEADER) -c $< -o $@
	
.PHONY: all run bundle clean
