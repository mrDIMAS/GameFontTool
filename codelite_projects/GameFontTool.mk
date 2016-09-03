##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Sample
ProjectName            :=GameFontTool
ConfigurationName      :=Sample
WorkspacePath          :=C:/GameFontTool/codelite_projects
ProjectPath            :=C:/GameFontTool/codelite_projects
IntermediateDirectory  :=./Release
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
OutputFile             :=../samples/sample.exe
Preprocessors          :=$(PreprocessorSwitch)_TEST 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="GameFontTool.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=C:/MinGW-4.8.1/bin/windres.exe
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)C:/MinGW-4.8.1/include/ $(IncludeSwitch)C:/MinGW-4.8.1/include/freetype2 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)freetype $(LibrarySwitch)opengl32 $(LibrarySwitch)gdi32 $(LibrarySwitch)glu32 
ArLibs                 :=  "libfreetype.a" "libopengl32.a" "libgdi32.a" "libglu32.a" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := C:/MinGW-4.8.1/bin/ar.exe rcu
CXX      := C:/MinGW-4.8.1/bin/g++.exe
CC       := C:/MinGW-4.8.1/bin/gcc.exe
CXXFLAGS :=   $(Preprocessors)
CFLAGS   :=  -ggdb -O0 -Werror -ansi -pedantic  $(Preprocessors)
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
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@$(MakeDirCommand) "./Release"


$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "./Release"

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
	$(RM) -r ./Release/


