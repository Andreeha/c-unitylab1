g++ -o main main.cpp -I ./raylib/include -L ./raylib/lib -l:libraylib.a
x86_64-w64-mingw32-g++ -o main.exe -g main.cpp -I ./raylibwin64/include/ -L ./raylibwin64/lib/ -l:libraylib.a -l:libwinmm.a -mwindows -static

