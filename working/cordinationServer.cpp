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
#define UDP_PORT 15200
using namespace rapidjson;
using namespace std;
int number_of_clients = 0;
int icount=-1;
Document document;
map <int,int> timeout; //which threads have timeout
map <int,bool> islive; //true if timeout is non-zero, else false

u_int64_t slave_uid = 9223372036854775;
u_int64_t client_uid = 1234567890123456;

int number_of_slave_servers; //to keep track of total number of slave servers
int number_of_slave_servers_alive; //to keep track of all the slave servers that are alive. Max of 1 slave can be down at a time

unordered_map <u_int64_t,string> slaveuid_to_ipport;
// unordered_map <u_int64_t,string> clientuid_to_ipport;
unordered_map <string,u_int64_t> ipport_to_uid; //to keep track of all the slave servers that are already registered, if new slave then add to this map as well
unordered_map <u_int64_t, int> uid_to_socket; 

string client_acknowledge(){
	string mystring = " { \"request_type\" : \"acknowledge_client_registeration\" } ";
	return mystring;
}

string slave_acknowledge(u_int64_t id, string ipport){
	string mystring = " { \"request_type\" : \"acknowledge_slave_registeration\", \"slave_uid\" : "+to_string(id)+", \"slave_ipport\" : \""+ipport+"\" } ";
	return mystring;
}

string put_request_slave(string key, string value){
	string mystring = " {  \"request_type\" : \"put_request\", \"key\" : \""+key+"\", \"value\" : \""+value+"\" } ";
	return mystring;
}

string slave_commit(int status){
	string mystring = " {  \"request_type\" : \"commit_operation\", \"commit_status\" :"+to_string(status)+" } ";
	return mystring;
}

#define NUM_THREADS 5

Node *root = NULL; 

struct thread_data {
    int  thread_id,new_socket;
	string request_string;
   	// char* ip_port;
    // int slaveid;
};

//function to listen to heart beat signals
//udp connection!

//sleeps and checks if thread is alive or not
void* timer(void* arg)
{ 
	cout<<"in timer!\n";
	sleep(30);
	while(1){
		for(int i=0;i<RING_CAPACITY;i++){
			// cout<<"i: "<<i<<"\n";
			if(timeout[i]==0&&islive[i]==true) {
				islive[i]=false;
				cout<<"slave "<<i <<"died\n";
			}			
			timeout[i]=0;
		}
		sleep(20);
	}

}

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

void* heartbeatListener(void* arg)
{
	cout<<"inside heartbeatListener\n";
	//convert void* to int
	int port_addr=*((int*)arg);
	cout<<"port_addr: "<<port_addr<<"\n";
	//make connection using udp;
	int server_fd,new_socket,valread; 
	struct sockaddr_in address;
	int opt = 1; 
	int addrlen = sizeof(address); 

	if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) //udp
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port_addr); 
	cout<<"before udp bind\n";
	if (bind(server_fd, (struct sockaddr *)&address,sizeof(address)) < 0) 
	{ 
		perror("udp bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	cout<<"before udp while\n";
	while(1){
		cout<<"in while udp\n";
		char buffer[1024]={0};

		// read(server_fd,buffer,1024);
		recv(server_fd, buffer, 1024, 0);
		cout<<"Buffer: "<<buffer<<"\n";                   
		int index=calculate_hash_value(buffer,RING_CAPACITY); 
		cout<<"index: "<<index<<"\n";
		islive[index]=true;
		timeout[index]++;
	}
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
			cout<<"this is received string: "<<Buffer<<endl;
			//cout<<"Buffer: "<<Buffer<<"\n";
			// const char delimiter = ' ';
			// vector <string> cmd;
			// tokenize(Buffer,delimiter,cmd);
			//cout<<"cmd[0] "<<cmd[0]<<" cmd[1]: "<<cmd[1]<<"\n";
			/*
			if (cmd[0] == "SS")
			{
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
			    Node *slave_node = findPreSuc(root,suc);
			    if(slave_node == NULL)
			    	slave_node = minValue(root);
			    Node *suc_of_slave = findPreSuc(root,slave_node->key+1);
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
			}*/
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
		
   	for(int i=0;i<RING_CAPACITY;i++){
   		timeout[i]=0;
   	}

   	for(int i=0;i<RING_CAPACITY;i++){
   		islive[i]=false;
   	}
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
	int i=1;
			cout << "SERVER is online" <<endl;

		pthread_t hb_thread;
	int port=UDP_PORT;
	cout<<"CALLING heartbeatListener\n";
	if(pthread_create(&hb_thread,NULL,heartbeatListener,(void*)&port)<0){
		perror("Error! ");
	}

//a thread which checks live status every 5 secs
	pthread_t timer_thread;
	cout<<"calling timer\n";
	if(pthread_create(&timer_thread,NULL,timer,(void*)&port)<0){
		perror("Error!");
	}

	while(1)
	{
		pthread_t threads[10];
		struct thread_data td[10];
		char Buffer[1024]={0};

		if (listen(server_fd, 3) < 0){ 
			perror("listen"); 
			exit(EXIT_FAILURE); 
		} 
		if ((new_socket = accept(server_fd, (struct sockaddr *)&(address),(socklen_t*)&(addrlen)))<0){ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}
		td[i].thread_id = i;
      	td[i].new_socket=new_socket;

		int readval = read(new_socket,Buffer,100);

		//------------------parsing json document-----------------------------------
		string buffer(Buffer);
		cout<<"printing received msg after string conversion "<<buffer<<endl;
		if (document.ParseInsitu(Buffer).HasParseError()){
			cout<<"Error while parsing the json string while registeration of client"<<endl;
		}
		
		//--------------------code to register a client with the co-ordination server-----------------
		else if(strcmp(document["request_type"].GetString(),"register_client")==0){
			assert(document.IsObject());

			cout<<"Parsing of the document for client registeration is successful"<<endl;
			string mystring_here = client_acknowledge();
			cout<<"json string to acknowledge client registeration "<<mystring_here<<endl<<endl;	
			send(new_socket,mystring_here.c_str(),200,0);
			cout<<"acknowledge successfully sent to the client"<<endl;
			rc = pthread_create(&threads[i], NULL, ServiceToAny, (void *)&td[i]);
			if (rc){
				cout << "Error:unable to create thread," << rc << endl;
			}
			pthread_detach(threads[i]);
			i++;
		}
		//--------------------code to register a client with the co-ordination server-----------------

		//--------------------code to register a slave with the co-ordination server-------------------
		else if(strcmp(document["request_type"].GetString(),"register_slave")==0){
			assert(document.IsObject());
			assert(document.HasMember("slave_ip"));
			assert(document.HasMember("slave_port"));
			assert(document["slave_ip"].IsString());
			assert(document["slave_port"].IsString());

			cout<<"Parsing of the document for client registeration is successful"<<endl;

			//int registeration_id = slave_uid++;
			char slave_ipport[100];
			strcpy(slave_ipport,document["slave_ip"].GetString());
			strcat(slave_ipport,":");
			strcat(slave_ipport,document["slave_port"].GetString());
			string sl_ipport(slave_ipport);

			cout<<"This is slave ip:port: "<<sl_ipport<<endl;
			cout<<"slave registered but acknowledgement is left"<<endl;
			string slave_ack_string;

			//----------differentiate among already registered slave server-----------------
			if(ipport_to_uid.find(sl_ipport)!=ipport_to_uid.end()){
				//to ensure that a slave sever gets the same id if it was registered once with co-ordination server
				slave_ack_string=slave_acknowledge(ipport_to_uid[sl_ipport],sl_ipport);
			}
			else{
				int id = calculate_hash_value(sl_ipport,RING_CAPACITY);
				ipport_to_uid[sl_ipport]=id;
				slave_ack_string = slave_acknowledge(id,sl_ipport);
			}
			//----------differentiate among already registered slave server-----------------

			//string mystring_here = slave_acknowledge(registeration_id,sl_ipport);
			cout<<"json string to acknowledge slave registeration "<<slave_ack_string<<endl<<endl;	
			send(new_socket,slave_ack_string.c_str(),200,0);
			cout<<"acknowledge successfully sent to the slave"<<endl;
		}
		//--------------------code to register a slave with the co-ordination server-------------------
	} 		
	return 0; 
} 
