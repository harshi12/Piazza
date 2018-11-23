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
#define PORT_CS 8080
using namespace std;
using namespace rapidjson;

Document document;

string register_with_coserver(string client_ip, string client_port){
	string mystring = " {  \"request_type\" : \"register_client\", \"client_ip\" : \""+client_ip+"\", \"client_port\" : \""+client_port+"\" } ";
	return mystring;
}

int main(int argc, char const *argv[]) 
{ 
	//--------extract client ip port from command line arguments--------------
	string ipport = argv[1];
	string client_ip = ipport.substr(0,ipport.find(':'));
	string client_port = ipport.substr(ipport.find(':')+1);
	cout<<client_ip<<" "<<client_port<<endl;
	//--------extract client ip port from command line arguments--------------


	int sock = 0, valread,sock_cs; 
	struct sockaddr_in serv_addr, cs_serv_addr; 
	char buffer[1024] = {0}; 
	
	//------------------------establish connection with slave server with port number 8081-------------
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
	//------------------------establish connection with slave server with port number 8081-------------


	//------------------establish connection with the co-ordination server with port number 8080---------------
						// if ((sock_cs = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
						// { 
						// 	printf("\n Socket creation error \n"); 
						// 	return -1; 
						// } 
						
						// memset(&cs_serv_addr, '0', sizeof(cs_serv_addr)); 

						// cs_serv_addr.sin_family = AF_INET; 
						// cs_serv_addr.sin_port = htons(PORT_CS); 
						
						// // Convert IPv4 and IPv6 addresses from text to binary form 
						// if(inet_pton(AF_INET, "127.0.0.1", &cs_serv_addr.sin_addr)<=0) 
						// { 
						// 	printf("\nInvalid address/ Address not supported \n"); 
						// 	return -1; 
						// } 

						// if (connect(sock_cs, (struct sockaddr *)&cs_serv_addr, sizeof(cs_serv_addr)) < 0) 
						// { 
						// 	printf("\nConnection Failed \n"); 
						// 	return -1; 
						// } 
	//------------------establish connection with the co-ordination server with port number 8080---------------
 

	//---------------register the client with co-ordination server-----------------
	// string string_here = register_with_coserver(client_ip,client_port);
	// cout<<string_here<<endl;
	// cout<<string_here.length()<<endl;

	// send(sock_cs,string_here.c_str(),100,0);

	// char cs_ack[200];
	// recv(sock_cs, cs_ack, 200, 0);
    // string ackstring(cs_ack);
	// cout<<"Client successfully registered with the server: "<<ackstring<<endl;
	//---------------register the client with co-ordination server-------------------



    
    char opchar[1024]="PUT own 4 89";
    char gethash[1024]="GET own 4";
    char deletehash[1024]="DELETE own 5 77";
    
    send(sock , opchar , strlen(opchar) , 0 );
    printf("%s , request sent\n", opchar ); 
	valread = read( sock , buffer, 1024); 
	// sleep(4);
	cout<<" id of slave is received as :"<<buffer<<endl;
	memset(buffer,0,sizeof(buffer));
	cout<<gethash<<"gethash val"<<endl;
	send(sock , gethash , strlen(gethash) , 0 ); 
	printf("%s , request sent\n", gethash ); 

	valread = read( sock , buffer, 1024); 
	cout<<"request sent : "<<buffer<<endl;
	printf("%s \n",buffer ); 
	return 0; 
}