//./slaveServer1 127.0.0.1:8081

#include <iostream>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <string.h> 
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include <set> 
#include "strtoken.hpp"

#define CSPORT 8080 

#define NUM_THREADS 5
using namespace std;

unordered_map<string, string>own;

unordered_map<string, string>previous;

struct thread_data {
    int  thread_id,new_socket;
};




void* Service(void* t)
{
	
	struct thread_data *tid;
    tid = (struct thread_data *)t;
	cout<<"SERVICING request" <<endl;

while(1)
	{
		char Buffer[1024];
		int readval = read(tid->new_socket,Buffer,1024);
        const char delimiter = ' ';
        vector <string> cmd;
        tokenize(Buffer,delimiter,cmd);

        if (cmd[0] == "PUT")
            {
				cout << "adding ("<< cmd[2] <<","<<cmd[3]<<") to "<<cmd[1]<<endl;
				if(cmd[1] == "own")
				{
					cout<<"in own"<<endl;
					own[cmd[2]]= cmd[3];
				}
				if(cmd[1] == "previous")
				{
					previous[cmd[2]]= cmd[3];

				}

				cout <<cmd[2]<<"->"<<own[cmd[2]]<<endl;
				send(tid->new_socket,"add success.." , 13 ,0);


				cout << "finshed adding."<<endl;

            }
          if (cmd[0] == "GET")
		  	{

				cout<<"inside GET"<<endl;
				char return_value[1024]= {0};
				if(cmd[1] == "own")
				{
					cout<<"in own"<<endl;
					if(own.find(cmd[2])!=own.end())
						{
						strcpy(return_value,(own[cmd[2]]).c_str());
						cout<<" the key value is ------"<<(own[cmd[2]]).c_str()<<endl;
						}
					else
						{
						strcpy(return_value,"Not Found!!");
						cout<<"not found"<<endl;
						// cout<<" the key value is ------"<<(own[tid->key]).c_str()<<endl;
						}
				}
				else if(cmd[1] == "previous")
				{		
					if(previous.find(cmd[2])!=previous.end())
						{
						strcpy(return_value,(previous[cmd[2]]).c_str());
						cout<<" the key value is ------"<<(previous[cmd[2]]).c_str()<<endl;
						}
					else
						{
						strcpy(return_value,"Not Found!!");
						cout<<"not found"<<endl;
						}
				}

				//cout <<tid->key<<"->"<<own[tid->key]<<endl;
				send(tid->new_socket,return_value,strlen(return_value) ,0);

				cout <<"GET finished"<<endl;

          	}
          	if (cmd[0] == "DELETE")
		  	{

				cout<<"inside DELETE"<<endl;
				char return_value[1024]= {0};



				if(cmd[1] == "own")
				{
					cout<<"in own"<<endl;
					if(own.find(cmd[2])!=own.end())
						{
							// unordered_map<char,int>::iterator it;
							// it = own.find(cmd[2]);
							own.erase(cmd[2]);
							strcpy(return_value,"Deleted!!");
							
						}
					else
						{
						strcpy(return_value,"Not Found!!");
						cout<<"not found"<<endl;
						// cout<<" the key value is ------"<<(own[tid->key]).c_str()<<endl;
						}
				}
				else if(cmd[1] == "previous")
				{		
					if(previous.find(cmd[2])!=previous.end())
						{
							// unordered_map<char,int>::iterator it;
							// it = previous.find(cmd[2]);
							previous.erase(cmd[2]);
							strcpy(return_value,"Deleted!!");
							
						}
					else
						{
						strcpy(return_value,"Not Found!!");
						cout<<"not found"<<endl;
						// cout<<" the key value is ------"<<(own[tid->key]).c_str()<<endl;
						}
					
				}

				//cout <<tid->key<<"->"<<own[tid->key]<<endl;
				send(tid->new_socket,return_value,strlen(return_value) ,0);

				cout <<"GET finished"<<endl;

          	}
			  memset(Buffer,0,sizeof(Buffer));
	}

}



int main(int argc, char const *argv[]) 
{ 

				
				struct sockaddr_in serv_addr;
				int sock = 0;
				 	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
						{ 
							printf("\n Socket creation error \n"); 
							return -1; 
						} 

				memset(&serv_addr, '0', sizeof(serv_addr)); 

				serv_addr.sin_family = AF_INET; 
				serv_addr.sin_port = htons(CSPORT); 
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

				char cmd[1024] = "SS ";
				strcat(cmd,argv[1]);
				cout << cmd<<endl;
				send(sock , cmd , strlen(cmd) , 0 ); 
				cout << "request to CS sent" <<endl;
				char cmdBuffer[1024]={0};		
				int readval = read(sock,cmdBuffer,1024);

				cout << cmdBuffer <<"ADDED"<<endl;

				cout << "before while"<< endl;




	pthread_attr_t attr;	
	int server_fd,new_socket; 
	struct sockaddr_in slaveAddress; 

	int opt = 1; 
	int addrlen = sizeof(slaveAddress); 
	
	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   	
   	int rc;	
   	
   	string a1(argv[1]);
	string s1_ipadd = a1.substr(0,a1.find(':'));
	string s1_port = a1.substr(a1.find(':')+1,a1.length());
 	
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	slaveAddress.sin_family = AF_INET; 
	slaveAddress.sin_addr.s_addr = inet_addr(s1_ipadd.c_str()); 
	slaveAddress.sin_port = htons(stoi(s1_port)); 
	
	if (bind(server_fd, (struct sockaddr *)&slaveAddress,sizeof(slaveAddress))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 

			
	int i=0;
	while(1)
	{
		pthread_t threads[10];
		struct thread_data td[10];
		if (listen(server_fd, 3) < 0) 
		{ 
			cout <<"inside listen" <<endl;

			perror("listen"); 
			exit(EXIT_FAILURE); 
		} 

		cout << "before accept" << endl;
		if ((new_socket = accept(server_fd, (struct sockaddr *)&(slaveAddress),(socklen_t*)&(addrlen)))<0) 
			{ 	
				perror("accept"); 
				exit(EXIT_FAILURE); 
			}
		cout << "before read" << endl;
		
		td[i].thread_id = i;
      	td[i].new_socket=new_socket;
		rc = pthread_create(&threads[i], NULL, Service, (void *)&td[i]);
        if (rc) 
     	{
     		cout << "Error:unable to create thread," << rc << endl;
 		}

		
	   pthread_detach(threads[i]);
	  	i++;
	   // cout <<" i am destroyed"<<endl;
	
	} 
	
	return 0; 
} 
