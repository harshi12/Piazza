//./cordinationServer 127.0.0.1:8080

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
#include "BST.hpp"

#define NUM_THREADS 5
using namespace std;

    Node *root = NULL; 


struct thread_data {
    int  thread_id,new_socket;
    string ip_port;
    int slaveid;
};


void *insertNewSlave(void *t)
{
    cout << "adding new slave"<<endl;
    struct thread_data *tid;
    tid = (struct thread_data *)t;
    insert(root,tid->slaveid,tid->ip_port);
    cout <<"slave sever "<<tid->slaveid << " with ip:port "<< tid->ip_port <<" added"<<endl;
    char reply[1024] = "CS accepted you. ";
    send(tid->new_socket,reply,strlen(reply),0);
    sleep(2);
   	cout << "Thread with id : " << tid->thread_id << "  ...exiting " << endl;
    pthread_exit(NULL);

}


int main(int argc, char const *argv[]) 
{ 

	pthread_attr_t attr;
   	void *status;	
	int server_fd,new_socket; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
			char Buffer[1024]={0};

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
	
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = inet_addr(s1_ipadd.c_str()); 
	address.sin_port = htons(stoi(s1_port)); 
	
	if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	int i=0;
			cout << "SERVER is online" <<endl;

	while(1)
	{
		pthread_t threads[10];
		struct thread_data td[10];
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 

	if ((new_socket = accept(server_fd, (struct sockaddr *)&(address),(socklen_t*)&(addrlen)))<0) 
		{ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}

		int readval = read(new_socket,Buffer,1024);
		cout << "before cmd " <<Buffer<<" <-Value of buffer "<<endl;
        const char delimiter = ' ';
        vector <string> cmd;
        tokenize(Buffer,delimiter,cmd);

        td[i].thread_id = i;
      	td[i].new_socket=new_socket;
        if (cmd[0] == "SS")
            {
                td[i].ip_port=cmd[1];
                td[i].slaveid=stoi(cmd[2]);
			    rc = pthread_create(&threads[i], NULL, insertNewSlave, (void *)&td[i]);
                if (rc) 
				     	{
			         	cout << "Error:unable to create thread," << rc << endl;
			     		}
            }

	   pthread_detach(threads[i]);
	  	i++;
	   // cout <<" i am destroyed"<<endl;
	
	} 
	
	return 0; 
} 
