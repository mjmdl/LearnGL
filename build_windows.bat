@ECHO OFF

SET cflags=/nologo /MT /Gm- /Oi /GR- /EHa- /WX /W4 /wd4211 /wd4100 /wd4189 /wd4201 /wd4505
SET lflags=/opt:ref user32.lib gdi32.lib opengl32.lib winmm.lib

where /q cl
IF %ErrorLevel% NEQ 0 (
    CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

IF NOT EXIST build MKDIR build

PUSHD build
cl %cflags% /DLEARNGL_DEBUG=1 /DLEARNGL_DEVELOP /Fewindows_learngl /Fmwindows_learngl.map /FC /Z7 ..\src\windows.cpp /link %lflags%
POPD
