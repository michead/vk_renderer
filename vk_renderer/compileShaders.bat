%cd%\glslangValidator.exe -V shaders/geom/shader.vert
%cd%\glslangValidator.exe -V shaders/geom/shader.frag
move /y %cd%\vert.spv %cd%\shaders\geom\vert.spv
move /y %cd%\frag.spv %cd%\shaders\geom\frag.spv

%cd%\glslangValidator.exe -V shaders/final/shader.vert
%cd%\glslangValidator.exe -V shaders/final/shader.frag
move /y %cd%\vert.spv %cd%\shaders\final\vert.spv
move /y %cd%\frag.spv %cd%\shaders\final\frag.spv

REM pause
