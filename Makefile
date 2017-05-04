release:
	g++ -std=c++11 -Wall -o Novice -Ofast Program.cxx -pthread
debug:
	g++ -std=c++11 -O0 -o Novice -g Program.cxx -pthread
profile:
	g++ -std=c++11 -Ofast -o Novice -pg Program.cxx -pthread