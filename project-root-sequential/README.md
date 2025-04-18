## FOR MAKEFILE

# from project-root/
make           # builds ./arbitrage
make run       # runs ./arbitrage (data/ must be next to this Makefile)
make clean     # removes .o files and the executable

cd /path/to/project-root




## FOR CMAKELISTS

# create an out‑of‑source build directory
mkdir -p build
cd build

# configure (generates VS solution or Makefiles, depending on your generator)
cmake ..

# build the Release configuration
cmake --build . --config Release

cd..

# run the new executable
.\build\Release\arbitrage.exe