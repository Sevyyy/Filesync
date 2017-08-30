g++ -o server Server.cpp Communication.cpp HandleRequest.cpp -std=c++11 -lws2_32
g++ -o client Client.cpp Communication.cpp HandleRequest.cpp -std=c++11 -lws2_32
copy server.exe server\
copy client.exe client1\
copy client.exe client2\