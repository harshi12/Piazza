//./cordinationServer 127.0.0.1:8080

#include <iostream>
#include <unistd.h>
#include<bits/stdc++.h>	
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <string.h> 
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include<unordered_map>
#include <string>
#include <fstream>
#include <unordered_map>
#include <set> 
#include "strtoken.hpp"
#include "BST.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#define RING_CAPACITY 16
using namespace rapidjson;
using namespace std;
int number_of_clients = 0;

Document document;

u_int64_t slave_uid = 9223372036854775;
u_int64_t client_uid = 1234567890123456;

int number_of_slave_servers; //to keep track of total number of slave servers
int number_of_slave_servers_alive; //to keep track of all the slave servers that are alive. Max of 1 slave can be down at a time

unordered_map <u_int64_t,string> slaveuid_to_ipport;
unordered_map <u_int64_t,string> clientuid_to_ipport;
unordered_map <int,u_int64_t> ipport_to_uid; //to keep track of all the slave servers that are already registered or are registering for the first time
unordered_map <u_int64_t, int> uid_to_socket; 

string client_acknowledge(u_int64_t id, string ipport){
	string mystring = " { \"request_type\" : \"acknowledge_client_registeration\", \"client_uid\" : "+to_string(id)+", \"client_ipport\" : \""+ipport+"\" } ";
	// string mystring = "{\"request_type\" : \"acknowledge_client_registeration\", \"client_uid\" : " +to_string(id)+ "\"client_ipport\" : "+ipport+ "}";
	return mystring;
}


 
#define NUM_THREADS 5
using namespace std;

    Node *root = NULL; 


struct thread_data {
    int  thread_id,new_socket;
   	// char* ip_port;
    // int slaveid;
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


void* ServiceToAny(void * t)
{
    struct thread_data *tid=(struct thread_data *)t;
    tid = (struct thread_data *)t;
	cout<<"SERVICING request" <<endl;
	while(1)
		{
			char Buffer[1024];
			//cout<<"entered in while"<<endl;
			int readval = read(tid->new_socket,Buffer,1024);
			//cout<<"Buffer: "<<Buffer<<"\n";
			const char delimiter = ' ';
			vector <string> cmd;
			tokenize(Buffer,delimiter,cmd);

			//cout<<"cmd[0] "<<cmd[0]<<" cmd[1]: "<<cmd[1]<<"\n";
			
			if (cmd[0] == "SS")
			{


			//--------------------code to register a client with the co-ordination server-----------------
			// string buffer(Buffer);
			// cout<<"printing received msg after string conversion "<<buffer<<endl;
			// if (document.ParseInsitu(Buffer).HasParseError()){
			// 	cout<<"Error while parsing the json string while registeration of client"<<endl;
			// }
			// else if(strcmp(document["request_type"].GetString(),"register_client")==0){
			// 	assert(document.IsObject());
			// 	assert(document.HasMember("client_ip"));
			// 	assert(document.HasMember("client_port"));
			// 	assert(document["client_ip"].IsString());
			// 	assert(document["client_port"].IsString());

			// 	cout<<"Parsing of the document for client registeration is successful"<<endl;

			// 	int registeration_id = client_uid++;
			// 	char client_ipport[100];
			// 	strcpy(client_ipport,document["client_ip"].GetString());
			// 	strcat(client_ipport,":");
			// 	strcat(client_ipport,document["client_port"].GetString());
			// 	string cl_ipport(client_ipport);

			// 	cout<<"This is client ip:port: "<<cl_ipport<<endl;

			// 	clientuid_to_ipport[registeration_id] = client_ipport; //mapped client registeration id with its ip:port
			// 	cout<<"client registered but acknowledgement is left"<<endl;

			// 	string mystring_here = client_acknowledge(registeration_id,cl_ipport);
			// 	cout<<"json string to acknowledge client registeration "<<mystring_here<<endl<<endl;	
			// 	send(new_socket,mystring_here.c_str(),200,0);
			// 	cout<<"acknowledge successfully sent to the client"<<endl;
			// }

			//--------------------code to register a client with the co-ordination server-----------------




				cout<<"adding ipport: "<<cmd[1]<<" thread id: "<<tid->thread_id<<"\n";

				unsigned long slave_id = calculate_hash_value(cmd[1],RING_CAPACITY);


				root = insert(root,slave_id,cmd[1]);

				cout<<"inside tree id: "<<root->key<<" ipport: "<<root->ipport<<endl;
				cout <<"slave sever "<<slave_id << " with ip:port "<< cmd[1] <<" added"<<endl;
				number_of_clients++;
				cout<<"number_of_clients : "<<number_of_clients<<endl;
				send(tid->new_socket,root->ipport.c_str(),strlen(root->ipport.c_str()),0);
				// =======
				//     root = insert(root,tid->slaveid,tid->ip_port);

				//     cout<<"inside tree "<<root->key<<" : "<<root->ipport<<endl;
				//     cout <<"slave sever "<<tid->slaveid << " with ip:port "<< tid->ip_port <<" added"<<endl;
				//     unsigned long slave_id = calculate_hash_value(tid->ip_port,4);
				//     int suc=slave_id;
				//     Node *slave_node = findPreSuc(root,suc);
				//     if(slave_node == NULL)
				//     	slave_node = minValue(root);
				//    	cout<<"successor is : =============="<<slave_node->key<<endl;
				//     cout<<"slave node is : "<<slave_node->ipport<<"of id "<<slave_id<<endl;
				//     //char slaveserverid[1024]="";
				//     //strcat(slaveserverid,to_string(slave_id).c_str());
				//     // slaveserverid = to_string(slave_id).c_str();
				//     // strcat(reply,to_string(slave_id).c_str());
				//     send(tid->new_socket,slave_node->ipport,strlen(slave_node->ipport),0);
				// >>>>>>> b4923d16563ef3419aceab9d18ea744c4f481a79
			}

			if (cmd[0] == "get_ipport")
			{
				cout<<"inside naya wala put"<<endl;
			    unsigned long slave_id = calculate_hash_value(cmd[1],RING_CAPACITY);
			    int suc=slave_id;
			    Node *slave_node = findPreSuc(root,suc-1);
			    if(slave_node == NULL)
			    	slave_node = minValue(root);
			    Node *suc_of_slave = findPreSuc(root,slave_node->key);
			    if(suc_of_slave == NULL)
			    	suc_of_slave = minValue(root);
			   	cout<<"successor is : =============="<<slave_node->key<<endl;
			    cout<<"slave node is : "<<slave_node->ipport<<"of id "<<slave_id<<endl;
			   	cout<<"successor of slave_node is : =============="<<suc_of_slave->key<<endl;
			    cout<<"slave node is : "<<suc_of_slave->ipport<<endl;

			    string ip = slave_node->ipport+":"+suc_of_slave->ipport;
			    cout<<"resultant ip is: "<<ip<<endl;
			    //char slaveserverid[1024]="";
			    //strcat(slaveserverid,to_string(slave_id).c_str());
			    // slaveserverid = to_string(slave_id).c_str();
			    // strcat(reply,to_string(slave_id).c_str());
			    send(tid->new_socket,ip.c_str(),ip.length(),0);

			}

			memset(Buffer,0,sizeof(Buffer));
		}

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
		
		td[i].thread_id = i;
      	td[i].new_socket=new_socket;

		rc = pthread_create(&threads[i], NULL, ServiceToAny, (void *)&td[i]);
		if (rc){
			cout << "Error:unable to create thread," << rc << endl;
		}


	   pthread_detach(threads[i]);
	  	i++;
	   // cout <<" i am destroyed"<<endl;
	
	} 
	
	return 0; 
} 
