# Microsoft Developer Studio Project File - Name="vst" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=vst - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "vst.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "vst.mak" CFG="vst - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "vst - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vst - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "max/vst"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vst - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pd-msvc/r"
# PROP Intermediate_Dir "pd-msvc/r"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "c:\programme\audio\pd\src" /I "f:\prog\max\flext\source" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D FLEXT_SYS=2 /D "FLEXT_THREADS" /D "_WINDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "NDEBUG"
# ADD RSC /l 0xc07 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 pd.lib flext_t-pdwin.lib pthreadVC.lib /nologo /dll /machine:I386 /out:"pd-msvc/vst~.dll" /libpath:"c:\programme\audio\pd\bin" /libpath:"f:\prog\max\flext\pd-msvc"

!ELSEIF  "$(CFG)" == "vst - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "pd-msvc/d"
# PROP Intermediate_Dir "pd-msvc/d"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "c:\programme\audio\pd\src" /I "f:\prog\max\flext\source" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D FLEXT_SYS=2 /D "FLEXT_THREADS" /D "_WINDLL" /D "_AFXDLL" /D "FLEXT_LOGGING" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "_DEBUG"
# ADD RSC /l 0xc07 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 pd.lib flext_tdl-pdwin.lib pthreadVC.lib /nologo /dll /debug /machine:I386 /out:"pd-msvc/d/vst~.dll" /pdbtype:sept /libpath:"c:\programme\audio\pd\bin" /libpath:"f:\prog\max\flext\pd-msvc"

!ENDIF 

# Begin Target

# Name "vst - Win32 Release"
# Name "vst - Win32 Debug"
# Begin Group "vst"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\vst\AEffect.h
# End Source File
# Begin Source File

SOURCE=.\src\vst\AEffectx.h
# End Source File
# Begin Source File

SOURCE=.\src\vst\AEffEditor.h
# End Source File
# Begin Source File

SOURCE=.\src\vst\AudioEffect.hpp
# End Source File
# Begin Source File

SOURCE=.\src\vst\audioeffectx.h
# End Source File
# End Group
# Begin Group "alt"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\vst.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\vst~.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\vst~.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "mfc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\Resource.h
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.h
# End Source File
# End Group
# Begin Group "host"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\EditorThread.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EditorThread.h
# End Source File
# Begin Source File

SOURCE=.\src\PopupWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PopupWindow.h
# End Source File
# Begin Source File

SOURCE=.\src\vst.rc
# End Source File
# Begin Source File

SOURCE=.\src\VstHost.cpp
# End Source File
# Begin Source File

SOURCE=.\src\VstHost.h
# End Source File
# End Group
# Begin Group "doc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gpl.txt
# End Source File
# Begin Source File

SOURCE=.\license.txt
# End Source File
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\main.h
# End Source File
# Begin Source File

SOURCE=.\src\vst.h
# End Source File
# End Target
# End Project
