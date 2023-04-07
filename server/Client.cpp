#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

int main(void)
{
	std::cout << "I am a client" << std::endl;
	
	std::string ip = "127.0.0.1";
	int port = 2036;
	
	Socket sock(ip, port);
	sock.Open();
	
	while (true) {
		std::cout << "Please enter input: ";
		std::string input;
		getline(std::cin, input);
		
		if (input == "done")
			break;
		
		ByteArray buffer(input);
		
		sock.Write(buffer);
		
		ByteArray response;
		
		if (sock.Read(response) == 0) {
			std::cout << "Server is terminated" << std::endl;
			break;		
		}
		
		std::string output = response.ToString();
		
		std::cout << "Server sends: " << output << std::endl;
	}
	
	sock.Close();
	
}
