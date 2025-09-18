OUTPUTDIR :=./bin
SOURCEDIR :=./src
HEADERDIR :=./include
PROGRAM_NAME = main

CXXFLAGS  +=-Iinclude -std=c++17 -pthread

#link with other libraries
LD_FLAGS :=-lm

#source files for the platform
SOURCE_FILES := $(wildcard $(SOURCEDIR)/*.cpp)

# include headers in the dependecy check
HEADER_FILES := $(wildcard $(HEADERDIR)/*.hpp)

#compiler an instance of the simulator
$(OUTPUTDIR)/$(PROGRAM_NAME): $(SOURCE_FILES) $(HEADER_FILES)
	g++ $(SOURCE_FILES) -o $(OUTPUTDIR)/$(PROGRAM_NAME) $(LD_FLAGS) $(CXXFLAGS)
	./bin/main | tee output_file.txt

#remove previously generated files
clean:
	$(Q)rm -rf $(OUTPUTDIR)/$(PROGRAM_NAME)

