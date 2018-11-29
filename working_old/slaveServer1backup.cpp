// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <unordered_map>
#include <iterator>
#include <string>
#include <iostream>
#include "strtoken.hpp"

#define PORT 8081 
#define MAX_TCP_SIZE = 45*1024

using namespace std;

int main(int argc, char const *argv[]) 
{ 
    unordered_map<string, string>own;
    unordered_map<string, string>prev;
    

	int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	char outBuffer[1024] = {0}; 

	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8081 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}
    cout << "waiting for request"<<endl;

	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen))<0) 
	{ 
		perror("accept"); 
		exit(EXIT_FAILURE); 
	}
	valread = read( new_socket , buffer, 1024); 
    
    // cout <<buffer <<endl;
    string OP(buffer);
    // cout <<OP<<endl;

    const char delimiter = ' ';
    vector <string> token;
    tokenize(OP,delimiter,token);
    cout << token[0] <<endl;
    if (token[0] == "PUT")
        {
            own[token[1]]= token[2];
        // cout << token[1]<<endl;
        // cout << token[2]<<endl;
        // cout <<"map value at 5: "<<own["5"] <<endl;

    printf("(%s , %s) inserted\n", token[1].c_str(),token[2].c_str() ); 
    snprintf( outBuffer,1024,"(%s , %s) inserted successfully\n", token[1].c_str(),token[2].c_str() ); 

	send(new_socket , outBuffer , strlen(outBuffer) , 0 ); 
	printf("request served successfully\n"); 

        }
    else
       { cout <<"failed"<<endl;
        send(new_socket , "failed" , 6 , 0 ); 
       }
	return 0; 
} 


