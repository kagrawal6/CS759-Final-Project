# 1) From project-root:
mkdir build      # only once
cd build

# 2) Generate
cmake ..

# 3) Build your Release binary
cmake --build . --config Release

# 4) Run it from project-root
cd ..
.\arbitrage.exe
