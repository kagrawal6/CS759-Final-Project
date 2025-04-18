# FOR MAKEFILE (Works fine)

*from project-root/*

make           # builds ./arbitrage
make run       # runs ./arbitrage (data/ must be next to this Makefile)
make clean     # removes .o files and the executable


# FOR CMAKELISTS (Stops in Time series analysis at timestamp 46200/66753)

*from project-root/*

mkdir -p build
cd build
cmake ..
cmake --build . --config Release
cd..
.\build\Release\arbitrage.exe
