# Compiler and tools
CXX          := g++
RM           := rm -rf
MKDIR        := mkdir -p

# Directories
SRCDIR       := src
INCDIR       := inc
BUILDDIR     := build
BINDIR       := bin

# Targets
PROJECT_NAME := ImageProcessor.out
EXE_FILE     := $(BINDIR)/$(PROJECT_NAME)

# Flags
CXXFLAGS     := -std=c++11 -Wall -Wextra -pthread
CPPFLAGS     := -I$(INCDIR)
LDFLAGS      :=
LDLIBS       := -pthread

# Sources
SOURCES     := $(shell find $(SRCDIR) -type f -name '*.cpp')

# Objects
OBJECTS     := $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# Dependency files
DEPS        := $(OBJECTS:.o=.d)

.PHONY: all clean debug release

all: $(EXE_FILE)

# Link
$(EXE_FILE): $(OBJECTS) | $(BINDIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compile
$(BUILDDIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BINDIR):
	@$(MKDIR) $@

clean:
	$(RM) $(BUILDDIR) $(BINDIR)

debug: CXXFLAGS += -g -O0 -DDEBUG
debug: all

release: CXXFLAGS += -O2 -DNDEBUG
release: all

-include $(DEPS)
