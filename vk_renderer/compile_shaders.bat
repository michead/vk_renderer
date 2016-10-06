%cd%\glslangValidator.exe -V shaders/geometry/shader.vert
%cd%\glslangValidator.exe -V shaders/geometry/shader.frag
move /y %cd%\vert.spv %cd%\shaders\geometry\vert.spv
move /y %cd%\frag.spv %cd%\shaders\geometry\frag.spv

%cd%\glslangValidator.exe -V shaders/lighting/shader.vert
%cd%\glslangValidator.exe -V shaders/lighting/shader.frag
move /y %cd%\vert.spv %cd%\shaders\lighting\vert.spv
move /y %cd%\frag.spv %cd%\shaders\lighting\frag.spv

%cd%\glslangValidator.exe -V shaders/shadow/shader.vert
%cd%\glslangValidator.exe -V shaders/shadow/shader.frag
move /y %cd%\vert.spv %cd%\shaders\shadow\vert.spv
move /y %cd%\frag.spv %cd%\shaders\shadow\frag.spv

%cd%\glslangValidator.exe -V shaders/ssao-main/shader.vert
%cd%\glslangValidator.exe -V shaders/ssao-main/shader.frag
move /y %cd%\vert.spv %cd%\shaders\ssao-main\vert.spv
move /y %cd%\frag.spv %cd%\shaders\ssao-main\frag.spv

%cd%\glslangValidator.exe -V shaders/ssao-blur/shader.vert
%cd%\glslangValidator.exe -V shaders/ssao-blur/shader.frag
move /y %cd%\vert.spv %cd%\shaders\ssao-blur\vert.spv
move /y %cd%\frag.spv %cd%\shaders\ssao-blur\frag.spv

REM pause
