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
   	char* ip_port;
    int slaveid;
};

unsigned long calculate_hash_value(int str1,int size) {
	//cout<<"string "<<str<<endl;
	string str;
	str=to_string(str1);
    unsigned long hash_value = 5381;
    int chr;
    int  i=0;
    while (chr = str[i++])
        hash_value = (((hash_value << 5) + hash_value)+chr); 
    	
    //cout<<"hash before return "<<hash% size<<endl;
    /* hash * 33 + c */
    return hash_value % size;

}

unsigned long calculate_hash_value(string str,int size) {
		//cout<<"string "<<str<<endl;
	    unsigned long hash_value = 5381;
        int chr;
        int  i=0;
        while (chr = str[i++])
            hash_value = (((hash_value << 5) + hash_value)+chr); 
        	
        //cout<<"hash before return "<<hash% size<<endl;
        /* hash * 33 + c */
        return hash_value % size;

}

void *insertNewSlave(void *t)
{
    cout << "adding new slave"<<endl;

    struct thread_data *tid=(struct thread_data *)t;
    tid = (struct thread_data *)t;
    cout<<"tid ip: "<<tid->ip_port<<" tid "<<tid->thread_id<<"\n";
    // tid->ip_port="x";
    root = insert(root,tid->slaveid,tid->ip_port);

    cout<<"inside tree "<<root->key<<" : "<<root->ipport<<endl;
    cout <<"slave sever "<<tid->slaveid << " with ip:port "<< tid->ip_port <<" added"<<endl;
    unsigned long slave_id = calculate_hash_value(tid->ip_port,4);
    Node *suc=NULL;
    Node *slave_node = preorder(root,suc);
    cout<<"slave node is : "<<slave_node->ipport<<"of id "<<slave_id<<endl;
    //char slaveserverid[1024]="";
    //strcat(slaveserverid,to_string(slave_id).c_str());
    // slaveserverid = to_string(slave_id).c_str();
    // strcat(reply,to_string(slave_id).c_str());
    send(tid->new_socket,slave_node->ipport,strlen(slave_node->ipport),0);

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
	
	if (bind(server_fd, (struct sockaddr *)&address,sizeof(address)) < 0) 
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

		char Buffer[1024];

		int readval = read(new_socket,Buffer,1024);
		cout<<"readval\n";
		cout<<Buffer<<"\n";

        const char delimiter = ' ';
        vector <string> cmd;
        tokenize(Buffer,delimiter,cmd);
        cout<<"cmd[0] "<<cmd[0]<<" cmd[1]: "<<cmd[1]<<"\n";
        td[i].thread_id = i;
      	td[i].new_socket=new_socket;
        if (cmd[0] == "SS")
            {
				char *ipport = new char[cmd[1].length()];
				strcpy(ipport,cmd[1].c_str());
               	td[i].ip_port=ipport;
				  
                td[i].slaveid=stoi(cmd[2]);
                cout<<"--pthread_create--i-"<<i<<"\n";
			    rc = pthread_create(&threads[i], NULL, insertNewSlave, (void *)&td[i]);
                if (rc){
			      	cout << "Error:unable to create thread," << rc << endl;
			    }
        }

	   pthread_detach(threads[i]);
	  	i++;
	   // cout <<" i am destroyed"<<endl;
	
	} 
	
	return 0; 
} 
