cd ..
md Exe
cd Source
mingw32-gcc SockServer.c -l ws2_32 -I ..\Include -o ..\Exe\SockServer.exe
cd Build
