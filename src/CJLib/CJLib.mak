
# Makefile for C++ Programming on Linux

#VERBOSE := 1
ifndef VERBOSE
.SILENT :
endif

#
# The PROGRAM macro defines the name of the program or project.  It
# allows the program name to be changed by editing in only one
# location
#
SUBSYSTEM = CJLib
TESTPROGRAM = cjtest
BUILD_DIR = ../Build

#
# The INCLUDEDIRS macro contains a list of include directories
# to pass to the compiler so it can find necessary header files.
#
# The LIBDIRS macro contains a list of library directories
# to pass to the linker so it can find necessary libraries.
#
# The LIBS macro contains a list of libraries that the the
# executable must be linked against.
#

INCLUDEDIRS = -Iinclude



#
# The CXXSOURCES macro contains a list of source files.
#
# The CXXOBJECTS macro converts the CXXSOURCES macro into a list
# of object files.
#
# The CXXFLAGS macro contains a list of options to be passed to
# the compiler.  Adding "-g" to this line will cause the compiler
# to add debugging information to the executable.
#
# The CXX macro defines the C++ compiler.
#
# The LDFLAGS macro contains all of the library and library
# directory information to be passed to the linker.
#

CXXCJTESTAPPSOURCES =     CJLibTest_Main.cpp 


CXXCJLIBSOURCES_LINUX =   CJAssert.cpp                \
			  CJThreadMgr.cpp             \
			  CJThread.cpp                \
			  CJPool.cpp                  \
			  CJPoolBlocking.cpp          \
			  CJConsole.cpp               \
			  CJPersistFile.cpp           \
			  CJQueue.cpp                 \
			  CJTime.cpp                  \
			  CJSocket.cpp                \
			  CJLinuxSysCmd.cpp           \
			  CJLinuxSysCmdMgr.cpp        \
			  CJSem.cpp                   \
			  CJJob.cpp                   \


CXXCJLIBSOURCES_COMMON =  CJObject.cpp                \
			  CJTrace.cpp                 \
                          CJList.cpp                  \
		          CJCli.cpp                   \
                          CJCliDefaultCommandSet.cpp      





#
# Directory and Flags setup
#

CXXFLAGS = $(INCLUDEDIRS) -g -Wall

CXXSOURCES = $(patsubst %.cpp,linux/%.cpp,$(CXXCJLIBSOURCES_LINUX)) $(patsubst %.cpp,common/%.cpp,$(CXXCJLIBSOURCES_COMMON)) 

CXXAPPSOURCES = $(patsubst %.cpp,common/%.cpp,$(CXXCJTESTAPPSOURCES)) 
CXXAPPOBJECTS = $(patsubst %.cpp, $(OBJS_DIR)/%.o, $(CXXAPPSOURCES))
CXXAPPDEPENDS = $(patsubst %.cpp, $(OBJS_DIR)/%.d, $(CXXAPPSOURCES))

# Include the Common Subsystem Make file (where this include is placed matters)
-include $(BUILD_DIR)/subsys_common.mak



#
# Default Rule
#
default:	$(TESTPROGRAM)
cjtest:		
lib:   		$(LINKFILE) 
 


OBJ_TO_CLEAN = $(CXXAPPOBJECTS) $(CXXAPPDEPENDS) $(TESTPROGRAM)
clean:
	rm -f $(OBJ_TO_CLEAN) 
	rm -rf $(OBJS_DIR_BASE)
	@echo "** Clean Complete **"


help:
	@echo 	-------------- HYDROID OS CJ LIBRARY -------------- 
	@echo   Summary:
	@echo          This is a common library of components like linked lisks, CLI, threading, object pools, etc.  This library
	@echo          is compatible with Windows and raspberry pi 2.
	@echo
	@echo 	Targets:
	@echo           lib - Builds for linux .a library only -- DEFAULT OPTION
	@echo           cjtest - Builds a unit test program linked with the CJlib.a   
	@echo           clean - Cleans all build output files.  Removes the obj folder
	@echo
	@echo   SYSTYPE: The SYSTYPE variable can be changed to modify the toolset
	@echo           default or blank: standard GCC linux
	@echo           rpicc: Raspberry Pi 2 cross compiler.  Make sure the path to the tools is correct in subsys_common.mak.
	@echo
	@echo   Example:
	@echo      	make cjtest SYSTYPE=rpicc  	-- Builds the unit test with Raspberry Pi 2 cross compiler
	@echo           make      			-- Builds the CJLib.a file for std linux GCC



#setup:
#	@mkdir -p $(OBJS_DIR)
#	@mkdir -p $(OBJS_DIR)/win
#	@mkdir -p $(OBJS_DIR)/linux
#	@mkdir -p $(OBJS_DIR)/common
#	@sleep 1


#Create Directories
dummy1 := $(shell test -d $(OBJS_DIR)/linux || mkdir -p $(OBJS_DIR)/linux)
dummy2 := $(shell test -d $(OBJS_DIR)/common || mkdir -p $(OBJS_DIR)/common)
dummy3 := $(shell test -d $(OBJS_DIR)/common || mkdir -p $(OBJS_DIR)/common)


#
# Include Dependancy Files
#
ifneq ($(MAKECMDGOALS),clean)
-include $(CXXAPPDEPENDS)
endif


#
# Link the CJLIB Test Application
#
LIBS = $(LINKFILE) -lpthread -lrt


$(TESTPROGRAM): $(LINKFILE) $(CXXAPPOBJECTS) 
	@echo "Linking Test Application (SYSTYPE=$(SYSTYPE)): $(TESTPROGRAM)"
	@$(CXX) -o $@ $(CXXAPPOBJECTS) $(LIBS)
	@echo "** CJLIB TEST APP BUILD COMPLETE **"














##
## Link the CJLIB Library
##
#LDLIBFLAGS =
#$(CJLIBPROGRAM): $(CXXLIBOBJECTS) $(CXXLIBDEPENDS)
#	@echo "Linking CJLib: $(CJLIBPROGRAM)"
#	@ar rc $@ $(CXXLIBOBJECTS) 
#	@cp $@ ../Build
#	@echo "** CJLIB LIBRARY BUILD COMPLETE **"
#
#
##
## Generate the dependancies and .p rule files for each object
##
#$(OBJS_DIR)/%.d : %.cpp
#	@echo "Dependancies:   $<"
#	@$(CXX) -MM ${CXXFLAGS} $< -MF $(@:.d=.p)
#	@echo -n "$(OBJS_DIR)/" > $@
#	@cat $(@:.d=.p) >> $@
#	@rm -f $(@:.d=.p)
#	@echo "	@echo Compiling:      $<" >> $@
#	@echo "	@${CXX} ${CXXFLAGS} -c -o $(@:.d=.o) $<" >> $@
#
#-include $(CXXLIBDEPENDS)
#-include $(CXXAPPDEPENDS)
#
#
#setup:
#	@mkdir -p $(OBJS_DIR)
#
#OBJ_TO_CLEAN = $(CXXAPPOBJECTS) $(CXXLIBOBJECTS) $(CXXAPPDEPENDS) $(CXXLIBDEPENDS)
#clean:
#	$(RM) -f $(OBJ_TO_CLEAN) $(CJLIBPROGRAM) $(TESTPROGRAM)
#	@echo "** Clean Complete **"
#



