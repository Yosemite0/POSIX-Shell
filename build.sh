g++ -std=c++17 -Wall -c env/ShellEnv.cpp -o env/ShellEnv.o
echo "g++ -std=c++17 -Wall -c env/ShellEnv.cpp -o env/ShellEnv.o"
g++ -std=c++17 -Wall  -c shell.cpp -o shell.o
echo "g++ -std=c++17 -Wall  -c shell.cpp -o shell.o"
g++ -std=c++17 -Wall  shell.o env/ShellEnv.o -lreadline -o shell
echo "g++ -std=c++17 -Wall  shell.o env/ShellEnv.o -lreadline -o shell"
"./shell"
