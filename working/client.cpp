// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include<iostream>
#include<bits/stdc++.h>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include<string>

using namespace std;
using namespace rapidjson;
Document document;

string register_with_coserver(string client_ip, string client_port){
	string mystring = "{\"request_type\" : \"register_client\", \"client_ip\" : " +client_ip+ ",\"client_port\" : "+client_port+ "}";
	return mystring;
}

#define PORT 8080 

int main(int argc, char const *argv[]) 
{ 
	struct sockaddr_in address; 
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char *hello = "Hello from client"; 
	char buffer[1024] = {0}; 
	
	string ipport = argv[1];
	string client_ip = ipport.substr(0,ipport.find(':'));
	string client_port = ipport.substr(ipport.find(':'));
	cout<<client_ip<<" "<<client_port<<endl;

	string string_here = register_with_coserver(client_ip,client_port);
	cout<<string_here<<endl;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

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
	send(sock , hello , strlen(hello) , 0 ); 
	printf("Hello message sent\n"); 
	valread = read( sock , buffer, 1024); 
	printf("%s\n",buffer ); 
	return 0; 
} 
