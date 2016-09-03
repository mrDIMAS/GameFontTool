##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## DynamicLibrary
ProjectName            :=GameFontTool
ConfigurationName      :=DynamicLibrary
WorkspacePath          :=C:/GameFontTool/codelite_projects
ProjectPath            :=C:/GameFontTool/codelite_projects
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=mrDIMAS
Date                   :=03/09/2016
CodeLitePath           :="C:/Program Files/CodeLite"
LinkerName             :=C:/MinGW-4.8.1/bin/g++.exe
SharedObjectLinkerName :=C:/MinGW-4.8.1/bin/g++.exe -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=../bin/libgamefonttool.dll
Preprocessors          :=$(PreprocessorSwitch)NDEBUG 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="GameFontTool.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=C:/MinGW-4.8.1/bin/windres.exe
LinkOptions            :=  -Wl,--out-implib,libgamefonttool.a 
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)C:/MinGW-4.8.1/include/freetype2 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)freetype 
ArLibs                 :=  "libfreetype.a" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := C:/MinGW-4.8.1/bin/ar.exe rcu
CXX      := C:/MinGW-4.8.1/bin/g++.exe
CC       := C:/MinGW-4.8.1/bin/gcc.exe
CXXFLAGS :=   $(Preprocessors)
CFLAGS   :=  -g -O3 -Wall -Werror -ansi -pedantic  $(Preprocessors)
ASFLAGS  := 
AS       := C:/MinGW-4.8.1/bin/as.exe


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files\CodeLite
Objects0=$(IntermediateDirectory)/gamefonttool.c$(ObjectSuffix) $(IntermediateDirectory)/sample.c$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
	@$(MakeDirCommand) "C:\GameFontTool\codelite_projects/.build-dynamiclibrary"
	@echo rebuilt > "C:\GameFontTool\codelite_projects/.build-dynamiclibrary/GameFontTool"

MakeIntermediateDirs:
	@$(MakeDirCommand) "./Debug"


$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "./Debug"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/gamefonttool.c$(ObjectSuffix): ../src/gamefonttool.c $(IntermediateDirectory)/gamefonttool.c$(DependSuffix)
	$(CC) $(SourceSwitch) "C:/GameFontTool/src/gamefonttool.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/gamefonttool.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/gamefonttool.c$(DependSuffix): ../src/gamefonttool.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/gamefonttool.c$(ObjectSuffix) -MF$(IntermediateDirectory)/gamefonttool.c$(DependSuffix) -MM ../src/gamefonttool.c

$(IntermediateDirectory)/gamefonttool.c$(PreprocessSuffix): ../src/gamefonttool.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/gamefonttool.c$(PreprocessSuffix)../src/gamefonttool.c

$(IntermediateDirectory)/sample.c$(ObjectSuffix): ../samples/sample.c $(IntermediateDirectory)/sample.c$(DependSuffix)
	$(CC) $(SourceSwitch) "C:/GameFontTool/samples/sample.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/sample.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/sample.c$(DependSuffix): ../samples/sample.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/sample.c$(ObjectSuffix) -MF$(IntermediateDirectory)/sample.c$(DependSuffix) -MM ../samples/sample.c

$(IntermediateDirectory)/sample.c$(PreprocessSuffix): ../samples/sample.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/sample.c$(PreprocessSuffix)../samples/sample.c


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


