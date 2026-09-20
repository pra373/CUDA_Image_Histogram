cls

cl.exe /c /EHsc src/main.cpp /Fo:build/main.obj

link.exe build/main.obj /OUT:build/main.exe