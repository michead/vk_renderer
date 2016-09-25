%cd%\glslangValidator.exe -V shaders/geometry/shader.vert
%cd%\glslangValidator.exe -V shaders/geometry/shader.frag
move /y %cd%\vert.spv %cd%\shaders\geometry\vert.spv
move /y %cd%\frag.spv %cd%\shaders\geometry\frag.spv

%cd%\glslangValidator.exe -V shaders/lighting/shader.vert
%cd%\glslangValidator.exe -V shaders/lighting/shader.frag
move /y %cd%\vert.spv %cd%\shaders\lighting\vert.spv
move /y %cd%\frag.spv %cd%\shaders\lighting\frag.spv

REM pause
