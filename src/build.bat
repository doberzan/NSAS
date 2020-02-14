@echo off
windres.exe -i song_server.rc -o ../resources/song.res -F pe-i386 -O coff
REM cl song.res winmm.lib NSAS.cpp
g++ ../resources/song.res NSAS.cpp -lws2_32 -lwinmm -o ../bin/NSAS.exe
windres.exe -i song_client.rc -o ../resources/song.res -F pe-i386 -O coff
REM cl song.res winmm.lib NSASC.cpp
g++ ../resources/song.res NSASC.cpp -lws2_32 -lwinmm -o ../bin/NSASC.exe