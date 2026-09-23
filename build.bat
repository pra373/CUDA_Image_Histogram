cls

cl.exe /c /EHsc src/main.cpp /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.5\include" /Fo:build/main.obj

nvcc.exe -c -o build/kernel.obj src/kernel.cu

rc.exe /fo build/icon.res resources/icon.rc

link.exe build/icon.res build/main.obj build/kernel.obj user32.lib gdi32.lib /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.5\lib\x64" cudart.lib /SUBSYSTEM:WINDOWS /OUT:build\main.exe