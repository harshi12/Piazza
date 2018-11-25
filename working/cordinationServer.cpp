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

string client_acknowledge(string req_type, string message, int status){
	string mystring = " { \"request_type\" : \""+req_type+"\", \"message\" : \""+message+"\", \"status\" : \""+to_string(status)+"\" } ";
	return mystring;
}

string slave_acknowledge(u_int64_t id, string ipport){
	string mystring = " { \"request_type\" : \"acknowledge_slave_registeration\", \"slave_uid\" : "+to_string(id)+", \"slave_ipport\" : \""+ipport+"\" } ";
	return mystring;
}

string put_request_slave(string key, string value, int main_ss){ //main_ss will say slave server to make changes in own or previous for 0 and 1 value respectively.
	string mystring = " {  \"request_type\" : \"put_request\", \"key\" : \""+key+"\", \"value\" : \""+value+"\", \"main_ss\" : "+to_string(main_ss)+" } ";
	return mystring;
}

string get_request_slave(string key, int main_ss){
	string mystring = " {  \"request_type\" : \"get_request\", \"key\" : \""+key+"\", \"main_ss\" : "+to_string(main_ss)+" } ";
	return mystring;
}

string del_request_slave(string key, int main_ss){
	string mystring = " {  \"request_type\" : \"delete_request\", \"key\" : \""+key+"\", \"main_ss\" : "+to_string(main_ss)+" } ";
	return mystring;
}

string slave_commit_func(int status){
	string mystring = " {  \"request_type\" : \"commit_operation\", \"commit_status\" : "+to_string(status)+" } ";
	return mystring;
}

string get_reponse_fun(string value){
	string mystring = " {  \"request_type\" : \"getreq_response\", \"value\" : \""+value+"\" } ";
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

int to_connect(string ip, int port, int cs_sock){
	int sock_here;
	struct sockaddr_in serv_addr;
	int addrlen = sizeof(serv_addr); 
	if ((sock_here = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 
	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(port); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock_here, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}
	return sock_here;
}

string get_ip(string ipport){
	string ip = ipport.substr(0,ipport.find(':'));
	return ip;
}

int get_port(string ipport){
	string temp = ipport.substr(ipport.find(':')+1);
	int port = stoi(temp);
	return port;
}

void* ServiceToAny(void * t)
{
    struct thread_data *tid=(struct thread_data *)t;
    tid = (struct thread_data *)t;
	cout<<"SERVICING request" <<endl;
	while(1){
		char Buffer[1024];
		int readval = read(tid->new_socket,Buffer,1024);
		string buffer(Buffer);
		int BufferSize = strlen(Buffer);
		if(BufferSize != 0)
		{
		if (document.ParseInsitu(Buffer).HasParseError()){
			cout<<"Error while parsing the json string while extracting request type"<<endl;
			string client_ack = client_acknowledge("error","Request incomplete. Try Again!",0);
			send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
		}
		//-------------------put request-------------------------------------------------
		else if(strcmp(document["request_type"].GetString(),"client_put_request")==0){
			assert(document.IsObject());
			assert(document.HasMember("key"));
			assert(document.HasMember("value"));
			assert(document["key"].IsString());
			assert(document["value"].IsString());

			char char_key[100];
			strcpy(char_key,document["key"].GetString());
			string key(char_key);
			char char_value[100];
			strcpy(char_value,document["value"].GetString());
			string value(char_value);

			unsigned long slave_id = calculate_hash_value(key,RING_CAPACITY);
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

			string slave_ip = get_ip(slave_node->ipport);
			int slave_port = get_port(slave_node->ipport);
			string suc_ip = get_ip(suc_of_slave->ipport);
			int suc_port = get_port(suc_of_slave->ipport);

			int sock_slave, sock_suc;
			sock_slave = to_connect(slave_ip, slave_port,tid->new_socket);
			sock_suc = to_connect(suc_ip, suc_port, tid->new_socket);

			if(sock_slave != -1 && sock_suc != -1){
				string putreq_slave = put_request_slave(key,value,0);
				send(sock_slave,putreq_slave.c_str(),putreq_slave.length(),0);
				sleep(1);
				string putreq_suc = put_request_slave(key,value,1);
				send(sock_suc,putreq_suc.c_str(),putreq_suc.length(),0);

				char response_slave[200],response_suc[200];
				recv(sock_slave,response_slave,200,0);
				// sleep(1);
				recv(sock_suc,response_suc,200,0);
				cout<<"response of slave:"<<response_slave<<endl;
				cout<<"response of succ: "<<response_suc<<endl;

				char response_slave_copy[200],response_suc_copy[200];
				strcpy(response_slave_copy,response_slave);
				strcpy(response_suc_copy,response_suc);				

				Document response1, response2;
				response1.Parse(response_slave);
				response2.Parse(response_suc);
				if (response1.HasParseError()){
					cout<<"DocumentParsing error for put request -- slave"<<endl;
					string client_ack = client_acknowledge("put_request_ack","Request incomplete. Try Again!",0);
					send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
				}	
				if(response2.HasParseError()){
					cout<<"DocumentParsing error for put request --  succ"<<endl;
					string client_ack = client_acknowledge("put_request_ack","Request incomplete. Try Again!",0);
					send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
				}
				else{
					// response1.Parse(response_slave_copy);
					// response2.Parse(response_slave_copy);
					cout<<"Parsing successful"<<endl;
					cout<<"response of slave:"<<response_slave<<endl;
					cout<<"response of succ: "<<response_suc<<endl;
					cout<<"here i am comijng"<<endl;

					cout<<1<<endl;
					assert(response1.IsObject());
					cout<<2<<endl;
					assert(response1.HasMember("request_status"));
					cout<<3<<endl;
					// assert(document["key"].IsString());
					assert(response1["request_status"].IsString());
					cout<<4<<endl;
					assert(response2.IsObject());
					cout<<5<<endl;
					assert(response2.HasMember("request_status"));
					cout<<6<<endl;
					assert(response2["request_status"].IsString());
					cout<<7<<endl;					
					
					cout<<"request type slave "<<response1["request_status"].GetString()<<endl;
					cout<<"request type succ "<<response2["request_status"].GetString()<<endl;
					if(strcmp(response1["request_status"].GetString(),"1") == 0 && strcmp(response2["request_status"].GetString(),"1") == 0){
						cout<<"I entered here"<<endl;
						string commit_slave = slave_commit_func(1);
						string commit_succ = slave_commit_func(1);
						cout<<"commit message for slave: "<<commit_slave<<endl;
						cout<<"commit message for succ: "<<commit_succ<<endl;
						send(sock_slave,commit_slave.c_str(),commit_slave.length(),0);
						sleep(1);
						send(sock_suc,commit_succ.c_str(),commit_succ.length(),0);
						cout<<"commit message successfully sent to slave and its successor successfully"<<endl;
						string client_ack = client_acknowledge("put_request_ack","Request Completed!",1);						
						send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
					}
					else{
						cout<<"Cannot commit to both the nodes. Please try again"<<endl;
						string client_ack = client_acknowledge("put_request_ack","Request incomplete. Try Again!",0);
						send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
					}
				}
				close(sock_slave);
				close(sock_suc);
			}
			else{
				cout<<"either of slave or its successor is down. Please try again!"<<endl;
			}
		}
		//-------------------------get request----------------------------------------
		else if(strcmp(document["request_type"].GetString(),"client_get_request")==0){
			assert(document.IsObject());
			assert(document.HasMember("key"));
			assert(document["key"].IsString());

			char char_key[100];
			strcpy(char_key,document["key"].GetString());
			string key(char_key);

			unsigned long slave_id = calculate_hash_value(key,RING_CAPACITY);
			int suc=slave_id;
			Node *slave_node = findPreSuc(root,suc);
			if(slave_node == NULL)
				slave_node = minValue(root);
			cout<<"slave node is : "<<slave_node->ipport<<"of id "<<slave_id<<endl;
			cout<<"successor is : =============="<<slave_node->key<<endl;					

			string slave_ip = get_ip(slave_node->ipport);
			int slave_port = get_port(slave_node->ipport);
			int sock_slave;
			sock_slave = to_connect(slave_ip, slave_port,tid->new_socket);

			if(sock_slave != -1){
				string get_request = get_request_slave(key,0);
				send(sock_slave,get_request.c_str(),get_request.length(),0);
				cout<<"value of the key: "<<key<<" requested from slave server"<<endl;
				char char_val[200];
				memset(char_val,0,sizeof(char_val));
				recv(sock_slave,char_val,200,0);
				cout<<"slave's response for get: "<<char_val<<endl;
				Document response;
				response.Parse(char_val);
				if (response.HasParseError()){
					cout<<"Error while parsing the json response of slave server for get request"<<endl;
					string client_ack = client_acknowledge("error","Request incomplete. Try Again!",0);
					send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
				}
				else{
					cout<<"Parsing successful"<<endl;
					cout<<"slave server's response: "<<char_val<<endl;
					char temp[100];
					strcpy(temp,response["value"].GetString());
					string value(temp);

					string get_response = get_reponse_fun(value);
					send(tid->new_socket,get_response.c_str(),get_response.length(),0);
					cout<<"value successfully sent to client"<<endl;
				}
				close(sock_slave);
			}
			else{
				Node *suc_of_slave = findPreSuc(root,slave_node->key+1);
				if(suc_of_slave == NULL)
					suc_of_slave = minValue(root);

				cout<<"slave node is : "<<suc_of_slave->ipport<<endl;				
				cout<<"successor of slave_node is : =============="<<suc_of_slave->key<<endl;					
				
				string suc_ip = get_ip(suc_of_slave->ipport);
				int suc_port = get_port(suc_of_slave->ipport);

				int sock_suc;
				sock_suc = to_connect(suc_ip, suc_port, tid->new_socket);

				string get_request = get_request_slave(key,1);
				send(sock_slave,get_request.c_str(),get_request.length(),0);
				cout<<"value of the key: "<<key<<" requested from slave server"<<endl;
				char char_val[200];
				recv(sock_slave,char_val,200,0);
				Document response;
				if (response.ParseInsitu(Buffer).HasParseError()){
					cout<<"Error while parsing the json response of successor server for get request"<<endl;
					string client_ack = client_acknowledge("error","Request incomplete. Try Again!",0);
					send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
				}
				else{
					cout<<"Parsing successful"<<endl;
					cout<<"successor server's response: "<<char_val<<endl;
					char temp[100];
					strcpy(temp,response["value"].GetString());
					string value(temp);

					string get_response = get_reponse_fun(value);
					send(tid->new_socket,get_response.c_str(),get_response.length(),0);
					cout<<"value successfully sent to client"<<endl;
				}
				close(sock_suc);
			}
		}
		//-----------------------------delete request-----------------------------------
		else if(strcmp(document["request_type"].GetString(),"client_delete_request")==0){
			assert(document.IsObject());
			assert(document.HasMember("key"));
			assert(document["key"].IsString());
			cout<<"In DEL request"<<endl;

			char char_key[100];
			strcpy(char_key,document["key"].GetString());
			string key(char_key);

			unsigned long slave_id = calculate_hash_value(key,RING_CAPACITY);
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

			string slave_ip = get_ip(slave_node->ipport);
			int slave_port = get_port(slave_node->ipport);
			string suc_ip = get_ip(suc_of_slave->ipport);
			int suc_port = get_port(suc_of_slave->ipport);

			int sock_slave, sock_suc;
			sock_slave = to_connect(slave_ip, slave_port,tid->new_socket);
			sock_suc = to_connect(suc_ip, suc_port, tid->new_socket);

			if(sock_slave != -1 && sock_suc != -1){
				string delreq_slave = del_request_slave(key,0);
				send(sock_slave,delreq_slave.c_str(),delreq_slave.length(),0);
				sleep(1);
				string delreq_suc = del_request_slave(key,1);
				send(sock_suc,delreq_suc.c_str(),delreq_suc.length(),0);

				char response_slave[200],response_suc[200];
				recv(sock_slave,response_slave,200,0);
				sleep(1);
				recv(sock_suc,response_suc,200,0);

				Document response1, response2;
				response1.Parse(response_slave);
				response2.Parse(response_suc);
				if (response1.HasParseError()){
					cout<<"DocumentParsing error for delete request"<<endl;
					string client_ack = client_acknowledge("error","Request incomplete. Try Again!",0);
					send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
				}	
				if(response2.HasParseError()){
					cout<<"DocumentParsing error for delete request"<<endl;
					string client_ack = client_acknowledge("error","Request incomplete. Try Again!",0);
					send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
				}
				else{
					cout<<"Parsing successful"<<endl;
					cout<<"response of slave:"<<response_slave<<endl;
					cout<<"response of succ: "<<response_suc<<endl;

					if(strcmp(response1["request_status"].GetString(),"1") == 0 && strcmp(response2["request_status"].GetString(),"1") == 0){
						string commit_slave = slave_commit_func(1);
						string commit_succ = slave_commit_func(1);
						send(sock_slave,commit_slave.c_str(),commit_slave.length(),0);
						sleep(1);
						send(sock_suc,commit_succ.c_str(),commit_succ.length(),0);
						cout<<"commit message successfully sent to slave and its successor successfully"<<endl;
						string client_ack = client_acknowledge("del_request_ack","Request completed!",0);
						send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
					}
					else{
						cout<<"Cannot commit to both the nodes. Please try again"<<endl;
						string client_ack = client_acknowledge("del_request_ack","Request incomplete. Try Again!",0);
						send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
					}
				}
				close(sock_slave);
				close(sock_suc);
			}
			else{
				cout<<"either of slave or its successor is down. Please try again!"<<endl;
				string client_ack = client_acknowledge("del_request_ack","Request incomplete. Try Again!",0);
				send(tid->new_socket,client_ack.c_str(),client_ack.length(),0);
			}
		}
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
			string mystring_here = client_acknowledge("acknowledge_client_registeration","registeration successful",1);
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
				root = insert(root,id,sl_ipport); //to insert the newly registered slave server to BST
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
