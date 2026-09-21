cls

cl.exe /c /EHsc src/main.cpp /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.5\include" /Fo:build/main.obj

nvcc.exe -c -o build/kernel.obj src/kernel.cu

link.exe build/main.obj build/kernel.obj /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.5\lib\x64" cudart.lib /OUT:build/main.exe