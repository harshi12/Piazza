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
#define PORT 8081 

using namespace std;

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

    
    char opchar[1024]="PUT own 5 79";
    char gethash[1024]="GET own 5";
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
