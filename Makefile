CXX = g++
CXXFLAGS = -std=c++20 -Iinclude

SRCDIR = src
SRCS = $(SRCDIR)/main.cpp $(SRCDIR)/PhysicsBody.cpp $(SRCDIR)/Projectile.cpp $(SRCDIR)/Cannon.cpp $(SRCDIR)/Target.cpp $(SRCDIR)/Debris.cpp
OBJS = $(SRCS:.cpp=.o)
HEADERS = $(wildcard include/*.h)

# Platform detection: pick the right raylib include/lib paths, link flags, exe name,
# and clean command for the OS we're building on. Same `make` works everywhere.
ifeq ($(OS),Windows_NT)
    # Windows (MinGW-w64 / w64devkit). Override the raylib location if yours differs:
    #   make RAYLIB_PATH=C:/path/to/raylib
    RAYLIB_PATH ?= C:/raylib/raylib
    TARGET = sim.exe
    CXXFLAGS += -I$(RAYLIB_PATH)/include
    LDFLAGS = -L$(RAYLIB_PATH)/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    CLEAN = del /q $(subst /,\,$(OBJS)) $(TARGET)
else
    UNAME_S := $(shell uname -s)
    TARGET = sim
    ifeq ($(UNAME_S),Darwin)
        # macOS (Homebrew)
        CXXFLAGS += -I/opt/homebrew/include
        LDFLAGS = -L/opt/homebrew/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit
    else
        # Linux
        LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
    CLEAN = rm -f $(OBJS) $(TARGET)
endif

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# headers are listed as a prerequisite so editing a .h (e.g. a Constants.h value)
# forces every .o to recompile, not just the .cpp that happens to be newer
$(SRCDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(CLEAN)

# Auto-rebuild and rerun the sim whenever a source or header changes. (Unix only: macOS/Linux)
# Requires entr: brew install entr
watch:
	@command -v entr >/dev/null 2>&1 || { echo "entr not found. Install it with: brew install entr"; exit 1; }
	@echo "Watching $(SRCDIR)/ and include/ — save a file to rebuild & rerun. Ctrl-C to quit."
	@ls $(SRCDIR)/*.cpp include/*.h | entr -r sh -c '$(MAKE) && ./$(TARGET)'

.PHONY: clean watch
