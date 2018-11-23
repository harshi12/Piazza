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
#include "strtoken.hpp"


#define PORT_CS 8080
using namespace std;
using namespace rapidjson;

Document document;

string register_with_coserver(string client_ip, string client_port){
	string mystring = " {  \"request_type\" : \"register_client\", \"client_ip\" : \""+client_ip+"\", \"client_port\" : \""+client_port+"\" } ";
	return mystring;
}

string put_slave_by_cordinator(string key,int sock_cs){
	// cout<<"inside put func\n";
	cout<<"key "<<key<<endl;
	// string p = "PUT own ";
	// string command;
	// command = p+key+" "+value;
	string command = "get_ipport "+key;
	// command = command + '\0';
	cout<<command<<endl;

	send(sock_cs , command.c_str() , command.length() , 0 );
 	//printf("%s , request sent\n", command ); 
 	cout<<"req sent "<<command<<endl;
	char buffer[1024] = {0};
	int valread = read( sock_cs , buffer, 1024); 
	cout<<" id of slave is received as :"<<buffer<<endl;
	sleep(2);

	close(sock_cs);

	cout<<"disconnected from CS"<<endl;
	sleep(2);
	return buffer;
	

}

int main(int argc, char const *argv[]) 
{ 
	//--------extract client ip port from command line arguments--------------
	string ipport = argv[1];
	string client_ip = ipport.substr(0,ipport.find(':'));
	string client_port = ipport.substr(ipport.find(':')+1);
	cout<<client_ip<<" "<<client_port<<endl;
	//--------extract client ip port from command line arguments--------------


	int sock = 0,sock1=0,sock2=0, valread,sock_cs; 
	struct sockaddr_in serv_addr, cs_serv_addr; 
	char buffer[1024] = {0}; 
	

while(1){


		cout<<"Please select any one choice: "<<endl;
		cout<<"1. PUT\n";
		cout<<"2. GET\n";
		cout<<"3. DELETE\n";

		int choice;
		string key,value,res_ip;
		vector <string> slave_ipport;
		cin>>choice;
		

		string command1,command2;
		if(choice == 1){
			cout<<"Please enter the key value pair: ";
			cin>>key>>value;
			command1 = "PUT own " + key + " " + value;
			command2 = "PUT previous " + key + " " + value; 
			cout<<"done from put------------------"<<endl;
		}

		else if (choice == 2)
		{
			cout<<"Please enter the key: ";
			cin>>key;
			command1 = "GET own " + key ;
			command2 = "GET previous " + key; 
			cout<<"done from get------------------"<<endl;

		}
		else if (choice == 3)
		{
			cout<<"Please enter the key: ";
			cin>>key;
			command1 = "DELETE own " + key ;
			command2 = "DELETE previous " + key; 
			cout<<"done from delete------------------"<<endl;

		}




	//------------------establish connection with the co-ordination server with port number 8080---------------
	if ((sock_cs = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 
	
	memset(&cs_serv_addr, '0', sizeof(cs_serv_addr)); 

	cs_serv_addr.sin_family = AF_INET; 
	cs_serv_addr.sin_port = htons(PORT_CS); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &cs_serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock_cs, (struct sockaddr *)&cs_serv_addr, sizeof(cs_serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 
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


		res_ip = put_slave_by_cordinator(key,sock_cs);
		

		const char delimiter = ':';
        
        tokenize(res_ip,delimiter,slave_ipport);



//------------------------establish connection with slave server with port number 1 received-------------
	if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		 
	}

	memset(&serv_addr, '0', sizeof(serv_addr)); 
	cout<<"heloooooooooooooooooooo1"<<endl;
	cout<<slave_ipport[1]<<endl;
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(stoi(slave_ipport[1])); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET,slave_ipport[0].c_str(), &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		
	} 

	if (connect(sock1, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		 
	}
	//------------------------establish connection with slave server with port number 1 received-------------


	send(sock1,command1.c_str(),command1.length(),0);
	valread = read(sock1,buffer,1024);
	cout<<"after coming back from slave1 "<<buffer<<endl;
	
	close(sock1);
	cout<<"disconnected from SS1"<<endl;

	sleep(2);


//------------------------establish connection with slave server with port number 2 received-------------
	if ((sock2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		 
	}

	memset(&serv_addr, '0', sizeof(serv_addr)); 
	cout<<"heloooooooooooooooooooo2"<<endl;
	cout<<slave_ipport[3]<<endl;
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(stoi(slave_ipport[3])); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET,slave_ipport[2].c_str(), &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		
	} 

	if (connect(sock2, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		 
	}
	//------------------------establish connection with slave server with port number 2 received-------------


	send(sock2,command2.c_str(),command2.length(),0);
	valread = read(sock2,buffer,1024);
	cout<<"after coming back from slave2 "<<buffer<<endl;
	
	close(sock2);
	cout<<"disconnected from SS2"<<endl;

		memset(buffer,0,sizeof(buffer));
 	}

	return 0; 
}