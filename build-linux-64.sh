# g++ -g -o build/chess-linux chesss-linux.cpp -lX11 -lasound
g++ -Wall -Wextra -pedantic -Wno-unused -Wno-unused-result -O1 -o build/chess-linux chesss-linux.cpp -lX11 -lasound
# g++ -Wall -Wextra -pedantic -g -o build/chess-linux chesss-linux.cpp -lX11 -lasound
#g++ -E -o build/chess-linux chesss-linux.cpp