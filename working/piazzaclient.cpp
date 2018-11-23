// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include<string>

#define PORT 8081 

using namespace std;
using namespace rapidjson;

Document document;

string register_with_coserver(string client_ip, string client_port){
	string mystring = "{\"request_type\" : \"register_client\", \"client_ip\" : " +client_ip+ ",\"client_port\" : "+client_port+ "}";
	return mystring;
}

int main(int argc, char const *argv[]) 
{ 
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char buffer[1024] = {0}; 
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	string ipport = argv[1];
	string client_ip = ipport.substr(0,ipport.find(':'));
	string client_port = ipport.substr(ipport.find(':')+1);
	cout<<client_ip<<" "<<client_port<<endl;

	string string_here = register_with_coserver(client_ip,client_port);
	cout<<string_here<<endl;

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 

    // string OP = "PUT 5 77";
    char opchar[1024]="PUT 5 77";
    // strcpy(opchar,OP.c_str());
	send(sock , opchar , strlen(opchar) , 0 ); 
	printf("%s , request sent\n", opchar ); 
	valread = read( sock , buffer, 1024); 
	printf("%s \n",buffer ); 
	return 0; 
} 
