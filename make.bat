@echo off

set CPP_INC=%MG_INCLUDE%
set CPP_FLAGS=/W4 /Zi /EHsc /wd4201 /wd4996 /MP /I %CPP_INC%
set CPP_SRC=..\src\*.cpp
set CPP_LIBS=d3d11.lib d3dcompiler.lib user32.lib

cl %CPP_FLAGS% %CPP_SRC% %CPP_LIBS%


for %%f in (..\src\shaders\*.vs) do (
    fxc /E:main /T:vs_5_0 %%f /Fo:%%~nf.cso
)

for %%f in (..\src\shaders\*.ps) do (
    fxc /E:main /T:ps_5_0 %%f /Fo:%%~nf.cso
)

for %%f in (..\src\shaders\*.cs) do (
    fxc /E:main /T:cs_5_0 %%f /Fo:%%~nf.cso
)

