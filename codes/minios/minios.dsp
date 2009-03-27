# Microsoft Developer Studio Project File - Name="minios" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=minios - Win32 vmware
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "minios.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "minios.mak" CFG="minios - Win32 vmware"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "minios - Win32 vmware" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "minios - Win32 virtualpc" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "minios - Win32 vmware"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "minios___Win32_vmware"
# PROP BASE Intermediate_Dir "minios___Win32_vmware"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "vmware_Release"
# PROP Intermediate_Dir "vmware_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Ot /Ow /Og /Op /Oy /Ob1 /X /I "include" /FR /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ot /Ow /Og /Op /Oy /Ob1 /X /I "include" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /x /d "NDEBUG"
# ADD RSC /l 0x804 /x /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /base:"0x400000" /entry:"main" /dll /map:"../bin/minios.map" /machine:I386 /nodefaultlib /out:"../bin/minios.dll"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 /nologo /base:"0x400000" /entry:"main" /dll /map:"../bin/minios.map" /machine:I386 /nodefaultlib /out:"../bin/minios.dll"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds="../bin/relocate.exe" ../bin/minios.dll ../bin/bootsector ../bin/minios.img
# End Special Build Tool

!ELSEIF  "$(CFG)" == "minios - Win32 virtualpc"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "minios___Win32_virtualpc"
# PROP BASE Intermediate_Dir "minios___Win32_virtualpc"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "virtualpc_Release"
# PROP Intermediate_Dir "virtualpc_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Ot /Ow /Og /Op /Oy /Ob1 /X /I "include" /FR /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ot /Ow /Og /Op /Oy /Ob1 /X /I "include" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /x /d "NDEBUG"
# ADD RSC /l 0x804 /x /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /base:"0x400000" /entry:"main" /dll /map:"../bin/minios.map" /machine:I386 /nodefaultlib /out:"../bin/minios.dll"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 /nologo /base:"0x400000" /entry:"main" /dll /map:"../bin/minios.map" /machine:I386 /nodefaultlib /out:"../bin/minios.dll"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds="../bin/relocate.exe" ../bin/minios.dll ../bin/bootsector ../bin/minios.img
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "minios - Win32 vmware"
# Name "minios - Win32 virtualpc"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\disasm\asmserv.c
# End Source File
# Begin Source File

SOURCE=.\disasm\assembl.c
# End Source File
# Begin Source File

SOURCE=.\platform\console.c
# End Source File
# Begin Source File

SOURCE=.\disasm\disasm.c
# End Source File
# Begin Source File

SOURCE=.\platform\eth.c
# End Source File
# Begin Source File

SOURCE=.\platform\ide.c
# End Source File
# Begin Source File

SOURCE=.\kernel\keDebug.c
# End Source File
# Begin Source File

SOURCE=.\kernel\keDriverModel.c
# End Source File
# Begin Source File

SOURCE=.\kernel\keEntryMain.c
# End Source File
# Begin Source File

SOURCE=.\kernel\keMemory.c
# End Source File
# Begin Source File

SOURCE=.\kernel\keNet.c
# End Source File
# Begin Source File

SOURCE=.\kernel\keSched.c
# End Source File
# Begin Source File

SOURCE=.\kernel\keSem.c
# End Source File
# Begin Source File

SOURCE=.\kernel\keTimerIsr.c
# End Source File
# Begin Source File

SOURCE=.\platform\keyboard.c
# End Source File
# Begin Source File

SOURCE=.\platform\main.c
# End Source File
# Begin Source File

SOURCE=.\platform\pci.c
# End Source File
# Begin Source File

SOURCE=.\libc\stdio.c
# End Source File
# Begin Source File

SOURCE=.\libc\string.c
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\device.h
# End Source File
# Begin Source File

SOURCE=.\include\disasm.h
# End Source File
# Begin Source File

SOURCE=.\include\kernel.h
# End Source File
# Begin Source File

SOURCE=.\include\list.h
# End Source File
# Begin Source File

SOURCE=.\include\platform.h
# End Source File
# Begin Source File

SOURCE=.\include\stdarg.h
# End Source File
# Begin Source File

SOURCE=.\include\stdio.h
# End Source File
# Begin Source File

SOURCE=.\include\string.h
# End Source File
# Begin Source File

SOURCE=.\include\types.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\platform\platform.obj
# End Source File
# End Target
# End Project
