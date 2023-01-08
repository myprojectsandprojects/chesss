/usr/bin/x86_64-w64-mingw32-g++-posix -o build/chess-windows.exe\
	-mwindows --static\
	chesss-windows.cpp -lwinmm\
&& cp build/chess-windows.exe windows-share/
