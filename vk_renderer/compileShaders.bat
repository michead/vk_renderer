C:/VulkanSDK/1.0.21.1/Bin/glslangValidator.exe -V shaders/shader.vert
C:/VulkanSDK/1.0.21.1/Bin/glslangValidator.exe -V shaders/shader.frag
move /y %cd%\vert.spv %cd%\shaders\vert.spv
move /y %cd%\frag.spv %cd%\shaders\frag.spv
REM pause
