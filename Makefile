# Compiler
CXX = clang++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
OBJCPPFLAGS = $(CXXFLAGS)

# macOS paths
SDL3_INCLUDE := /opt/homebrew/Cellar/sdl3/3.2.8/include
SDL3_LIB := /opt/homebrew/Cellar/sdl3/3.2.8/lib
SDL3_IMAGE_INCLUDE := /opt/homebrew/Cellar/sdl3_image/3.2.4/include
SDL3_IMAGE_LIB := /opt/homebrew/Cellar/sdl3_image/3.2.4/lib
SDL3_TTF_INCLUDE := /opt/homebrew/Cellar/sdl3_ttf/3.2.0/include
SDL3_TTF_LIB := /opt/homebrew/Cellar/sdl3_ttf/3.2.0/lib
# SDL3_MIXER paths look correct but verify they exist
SDL3_MIXER_INCLUDE := /usr/local/include/SDL3_mixer
SDL3_MIXER_LIB := /usr/local/lib

# FFmpeg Paths
FFMPEG_INCLUDE := /opt/homebrew/Cellar/ffmpeg/7.1.1_1/include
FFMPEG_LIB := /opt/homebrew/Cellar/ffmpeg/7.1.1_1/lib

# Platform libraries
PLATFORM_LIBS = -framework Cocoa -framework OpenGL -lobjc

# Header directories
HEADER = -isystem $(SDL3_INCLUDE) \
         -I$(SDL3_IMAGE_INCLUDE) \
         -I$(SDL3_TTF_INCLUDE) \
         -I$(FFMPEG_INCLUDE) \
         -Iinclude/cpp_headers \
         -Iinclude/objc_headers \
         -Isrc/objc \
         -Idatabase

# Library flags
LIB_FLAGS = -L$(SDL3_LIB) -L$(SDL3_IMAGE_LIB) -L$(SDL3_TTF_LIB) -L$(FFMPEG_LIB) \
            -lSDL3 -lSDL3_image -lSDL3_ttf \
            -lavcodec -lavformat -lavutil -lswscale -lswresample \
            $(PLATFORM_LIBS)

# Target and sources
TARGET = AtaraxiaSDK
SRC_CPP = src/cpp/main.cpp src/cpp/videoRendering.cpp database/SDLColors.cpp
SRC_OBJC = src/objc/cocoaToolbarUI.mm

# Object files
OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ_OBJC = $(SRC_OBJC:.mm=.o)
OBJS = $(OBJ_CPP) $(OBJ_OBJC)

# Entitlements file
ENTITLEMENTS = entitlements.plist

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "DEBUG: Building executable $(TARGET) with objects:" $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIB_FLAGS) -o $(TARGET)
	@echo "DEBUG: Executable $(TARGET) built successfully."

# Add entitlements file creation
$(ENTITLEMENTS):
	@echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" > $(ENTITLEMENTS)
	@echo "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" >> $(ENTITLEMENTS)
	@echo "<plist version=\"1.0\">" >> $(ENTITLEMENTS)
	@echo "<dict>" >> $(ENTITLEMENTS)
	@echo "    <key>com.apple.security.cs.allow-unsigned-executable-memory</key>" >> $(ENTITLEMENTS)
	@echo "    <true/>" >> $(ENTITLEMENTS)
	@echo "    <key>com.apple.security.cs.disable-library-validation</key>" >> $(ENTITLEMENTS)
	@echo "    <true/>" >> $(ENTITLEMENTS)
	@echo "</dict>" >> $(ENTITLEMENTS)
	@echo "</plist>" >> $(ENTITLEMENTS)

bundle: $(TARGET) $(ENTITLEMENTS)
	@echo "DEBUG: Creating app bundle..."
	@rm -rf $(TARGET).app
	@mkdir -p $(TARGET).app/Contents/{MacOS,Resources,Frameworks}
	
	# Copy executable
	@echo "DEBUG: Copying executable..."
	@install -m 0755 $(TARGET) $(TARGET).app/Contents/MacOS/
	
	# Create a more robust launcher script
	@echo "DEBUG: Creating launcher script..."
	@echo "#!/bin/bash" > $(TARGET).app/Contents/MacOS/$(TARGET)_launcher
	@echo "# Get the directory where this script is located" >> $(TARGET).app/Contents/MacOS/$(TARGET)_launcher
	@echo "SCRIPT_DIR=\"\$$(cd \"\$$(dirname \"\$$0\")\" && pwd)\"" >> $(TARGET).app/Contents/MacOS/$(TARGET)_launcher
	@echo "RESOURCES_DIR=\"\$${SCRIPT_DIR}/../Resources\"" >> $(TARGET).app/Contents/MacOS/$(TARGET)_launcher
	@echo "cd \"\$$RESOURCES_DIR\"" >> $(TARGET).app/Contents/MacOS/$(TARGET)_launcher
	@echo "# Set environment variables to help find libraries" >> $(TARGET).app/Contents/MacOS/$(TARGET)_launcher
	@echo "export DYLD_LIBRARY_PATH=\"\$${SCRIPT_DIR}/../Frameworks:\$$DYLD_LIBRARY_PATH\"" >> $(TARGET).app/Contents/MacOS/$(TARGET)_launcher
	@echo "export SDL_AUDIODRIVER=coreaudio" >> $(TARGET).app/Contents/MacOS/$(TARGET)_launcher
	@echo "# Run the app with error logging" >> $(TARGET).app/Contents/MacOS/$(TARGET)_launcher
	@echo "\"\$$SCRIPT_DIR/$(TARGET)\" 2>&1 | tee -a \"\$$HOME/Desktop/$(TARGET)_error.log\"" >> $(TARGET).app/Contents/MacOS/$(TARGET)_launcher
	@chmod +x $(TARGET).app/Contents/MacOS/$(TARGET)_launcher
	
	# Copy assets with better debugging
	@echo "DEBUG: Copying assets..."
	@if [ -d "assets" ]; then \
		echo "DEBUG: Assets directory found, copying..."; \
		mkdir -p $(TARGET).app/Contents/Resources/assets/fonts; \
		cp -Rv assets/* $(TARGET).app/Contents/Resources/assets/; \
		ls -la $(TARGET).app/Contents/Resources/assets/fonts/; \
	else \
		echo "WARNING: Assets directory not found!"; \
	fi
	
	# Also copy assets to the MacOS directory as fallback
	@if [ -d "assets" ]; then \
		echo "DEBUG: Also copying assets to MacOS directory..."; \
		mkdir -p $(TARGET).app/Contents/MacOS/assets/fonts; \
		cp -Rv assets/* $(TARGET).app/Contents/MacOS/assets/; \
	fi
	
	# Create Info.plist
	@echo "DEBUG: Creating Info.plist..."
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $(TARGET).app/Contents/Info.plist
	@echo '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' >> $(TARGET).app/Contents/Info.plist
	@echo '<plist version="1.0">' >> $(TARGET).app/Contents/Info.plist
	@echo '<dict>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <key>CFBundleExecutable</key>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <string>$(TARGET)_launcher</string>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <key>CFBundleIdentifier</key>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <string>com.example.$(TARGET)</string>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <key>CFBundleName</key>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <string>$(TARGET)</string>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <key>CFBundlePackageType</key>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <string>APPL</string>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <key>CFBundleVersion</key>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <string>1.0</string>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <key>NSHighResolutionCapable</key>' >> $(TARGET).app/Contents/Info.plist
	@echo '    <true/>' >> $(TARGET).app/Contents/Info.plist
	@echo '</dict>' >> $(TARGET).app/Contents/Info.plist
	@echo '</plist>' >> $(TARGET).app/Contents/Info.plist
	
	# Copy libraries - use exact filenames
	@echo "DEBUG: Copying libraries..."
	@cp -f $(SDL3_LIB)/libSDL3.0.dylib $(TARGET).app/Contents/Frameworks/ || echo "WARNING: Could not copy libSDL3.0.dylib"
	@cp -f $(SDL3_IMAGE_LIB)/libSDL3_image.0.dylib $(TARGET).app/Contents/Frameworks/ || echo "WARNING: Could not copy libSDL3_image.0.dylib"
	@cp -f $(SDL3_TTF_LIB)/libSDL3_ttf.0.dylib $(TARGET).app/Contents/Frameworks/ || echo "WARNING: Could not copy libSDL3_ttf.0.dylib"
	@chmod 755 $(TARGET).app/Contents/Frameworks/*.dylib
	
	# Fix library paths - more comprehensive approach
	@echo "DEBUG: Updating library references..."
	@install_name_tool -id @executable_path/../Frameworks/libSDL3.0.dylib $(TARGET).app/Contents/Frameworks/libSDL3.0.dylib
	@install_name_tool -id @executable_path/../Frameworks/libSDL3_image.0.dylib $(TARGET).app/Contents/Frameworks/libSDL3_image.0.dylib
	@install_name_tool -id @executable_path/../Frameworks/libSDL3_ttf.0.dylib $(TARGET).app/Contents/Frameworks/libSDL3_ttf.0.dylib
	
	@install_name_tool -change /opt/homebrew/opt/sdl3/lib/libSDL3.0.dylib @executable_path/../Frameworks/libSDL3.0.dylib $(TARGET).app/Contents/MacOS/$(TARGET)
	@install_name_tool -change /opt/homebrew/opt/sdl3_image/lib/libSDL3_image.0.dylib @executable_path/../Frameworks/libSDL3_image.0.dylib $(TARGET).app/Contents/MacOS/$(TARGET)
	@install_name_tool -change /opt/homebrew/opt/sdl3_ttf/lib/libSDL3_ttf.0.dylib @executable_path/../Frameworks/libSDL3_ttf.0.dylib $(TARGET).app/Contents/MacOS/$(TARGET)
	
	# Fix dependencies between libraries
	@install_name_tool -change /opt/homebrew/opt/sdl3/lib/libSDL3.0.dylib @executable_path/../Frameworks/libSDL3.0.dylib $(TARGET).app/Contents/Frameworks/libSDL3_image.0.dylib
	@install_name_tool -change /opt/homebrew/opt/sdl3/lib/libSDL3.0.dylib @executable_path/../Frameworks/libSDL3.0.dylib $(TARGET).app/Contents/Frameworks/libSDL3_ttf.0.dylib
	
	# Print library dependencies for debugging
	@echo "DEBUG: Library dependencies for executable:"
	@otool -L $(TARGET).app/Contents/MacOS/$(TARGET)
	@echo "DEBUG: Library dependencies for SDL3_image:"
	@otool -L $(TARGET).app/Contents/Frameworks/libSDL3_image.0.dylib
	@echo "DEBUG: Library dependencies for SDL3_ttf:"
	@otool -L $(TARGET).app/Contents/Frameworks/libSDL3_ttf.0.dylib
	
	# Sign bundle - improved version with better error handling
	@echo "DEBUG: Signing with entitlements..."
	@codesign --force --options runtime --entitlements $(ENTITLEMENTS) --sign - $(TARGET).app/Contents/Frameworks/*.dylib || echo "WARNING: Failed to sign frameworks"
	@codesign --force --options runtime --entitlements $(ENTITLEMENTS) --sign - $(TARGET).app/Contents/MacOS/$(TARGET) || echo "WARNING: Failed to sign executable"
	@codesign --force --options runtime --entitlements $(ENTITLEMENTS) --sign - $(TARGET).app/Contents/MacOS/$(TARGET)_launcher || echo "WARNING: Failed to sign launcher"
	@echo "DEBUG: Signing complete app bundle (deep)..."
	@codesign --force --options runtime --entitlements $(ENTITLEMENTS) --deep --sign - $(TARGET).app || echo "WARNING: Failed to sign app bundle, but continuing anyway"
	@echo "DEBUG: Bundle created at $(TARGET).app"
	@echo "DEBUG: Error logs will be written to ~/Desktop/$(TARGET)_error.log"

%.o: %.cpp
	@echo "DEBUG: Compiling $< ..."
	$(CXX) $(CXXFLAGS) $(HEADER) -c $< -o $@

database/%.o: database/%.cpp
	@echo "DEBUG: Compiling $< ..."
	$(CXX) $(CXXFLAGS) $(HEADER) -c $< -o $@

src/objc/%.o: src/objc/%.mm
	@echo "DEBUG: Compiling $< ..."
	$(CXX) $(OBJCPPFLAGS) $(HEADER) -c $< -o $@

run: $(TARGET)
	@echo "DEBUG: Running executable..."
	./$(TARGET)

clean:
	@echo "DEBUG: Cleaning..."
	rm -f $(OBJS) $(TARGET) $(ENTITLEMENTS)
	rm -rf $(TARGET).app

.PHONY: all clean run bundle
