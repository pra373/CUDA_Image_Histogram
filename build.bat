cls

cl.exe /c /EHsc src/main.cpp /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.5\include" /Fo:build/main.obj

cl.exe /c /EHsc imgui/imgui.cpp /I "imgui" /Fo:build/imgui.obj

cl.exe /c /EHsc imgui/imgui_draw.cpp /I "imgui" /Fo:build/imgui_draw.obj

cl.exe /c /EHsc imgui/imgui_tables.cpp /I "imgui" /Fo:build/imgui_tables.obj

cl.exe /c /EHsc imgui/imgui_widgets.cpp /I "imgui" /Fo:build/imgui_widgets.obj

cl.exe /c /EHsc imgui/backends/imgui_impl_win32.cpp /I "imgui" /I "imgui\backends" /Fo:build/imgui_impl_win32.obj

cl.exe /c /EHsc imgui/backends/imgui_impl_dx11.cpp /I "imgui" /I "imgui\backends" /Fo:build/imgui_impl_dx11.obj

cl.exe /c /EHsc implot/implot.cpp /I "imgui" /I "implot" /Fo:build/implot.obj

cl.exe /c /EHsc implot/implot_items.cpp /I "imgui" /I "implot" /Fo:build/implot_items.obj

nvcc.exe -c -o build/kernel.obj src/kernel.cu

rc.exe /fo build/icon.res resources/icon.rc

link.exe build/icon.res build/main.obj build/kernel.obj build/imgui.obj build/imgui_draw.obj build/imgui_tables.obj build/imgui_widgets.obj build/imgui_impl_win32.obj build/imgui_impl_dx11.obj build/implot.obj build/implot_items.obj user32.lib gdi32.lib /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.5\lib\x64" cudart.lib d3d11.lib dxgi.lib d3dcompiler.lib /SUBSYSTEM:WINDOWS /OUT:build\main.exe