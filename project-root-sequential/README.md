## FOR MAKEFILE 

(Works fine) 

*from project-root/*

make           # builds ./arbitrage <br/>
make run       # runs ./arbitrage (data/ must be next to this Makefile) <br/>
make clean     # removes .o files and the executable



## FOR CMAKELISTS 

(Stops in Time series analysis at timestamp 46200/66753)

*from project-root/*

mkdir -p build<br/>
cd build<br/>
cmake ..<br/>
cmake --build . --config Release<br/>
cd..<br/>
.\build\Release\arbitrage.exe<br/>



*Only running pairs USD/SGD SGD/JPY USD/JPY for now*  
