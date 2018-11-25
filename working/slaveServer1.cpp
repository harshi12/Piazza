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
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#define CSPORT 8080 
#define BEATPORT 15200
#define NUM_THREADS 5
using namespace std;
using namespace rapidjson;

Document document;

unordered_map<string, string>own;
unordered_map<string, string>previous;

string register_slaveserver(string slave_ip, string slave_port){
	string mystring = " {  \"request_type\" : \"register_slave\", \"slave_ip\" : \""+slave_ip+"\", \"slave_port\" : \""+slave_port+"\" } ";
	return mystring;
}

string slave_request_ack(int status){
	string mystring = " {  \"request_type\" : \"slave_ack\", \"request_status\" : \""+to_string(status)+"\" } ";
	return mystring;
}

string get_reponse_fun(string value){
	string mystring = " {  \"request_type\" : \"getreq_response\", \"value\" : \""+value+"\" } ";
	return mystring;
}

string replicate_response_fun()
{
	string mystring = " {  \"request_type\" : \"replicate_response\" } ";
	return mystring;	
}

struct thread_data {
    int  thread_id,new_socket;
};

struct hb_thread{
	int id;
	char* ip;
};

void* Service(void* t){	
	struct thread_data *tid;
    tid = (struct thread_data *)t;
	cout<<"SERVICING request" <<endl;
	char Buffer[1024];
	string buffer(Buffer);
	int readval = read(tid->new_socket,Buffer,1024);
	cout <<"BUFFER: "<<Buffer<<endl;
	document.Parse(Buffer);
	cout<<"trying here: "<<Buffer<<endl;
	if (document.HasParseError()){
		cout<<"Error while parsing the json string while extracting request type from cs"<<endl;
	}
	else if(strcmp(document["request_type"].GetString(),"put_request")==0){
		assert(document.IsObject());
		assert(document.HasMember("key"));
		assert(document.HasMember("value"));
		assert(document["key"].IsString());
		assert(document["value"].IsString());

		cout<<"In PUT request"<<endl;

		string req_ack = slave_request_ack(1);
		cout<<"slave ack string: "<<req_ack<<endl;
		send(tid->new_socket,req_ack.c_str(),req_ack.length(),0);
		sleep(1);
		char response[100];
		recv(tid->new_socket,response,100,0);
		Document temp_doc;
		if (temp_doc.ParseInsitu(response).HasParseError()){
			cout<<"Error while parsing the json string while parsing commit message from cs in put"<<endl;
		}
		else if(temp_doc["commit_status"].GetInt() == 1){
			char char_key[100];
			strcpy(char_key,document["key"].GetString());
			string key(char_key);
			char char_value[100];
			strcpy(char_value,document["value"].GetString());
			string value(char_value);

			if(document["main_ss"].GetInt() == 0){
				//make changes in own hash table
				own[key] = value;
				cout<<"added key: "<<key<<" and value: "<<value<<" to own hash table"<<endl;
			}
			else if(document["main_ss"].GetInt() == 1){
				//make changes in prev hash table
				previous[key] = value;
				cout<<"added key: "<<key<<" and value: "<<value<<" to previous hash table"<<endl;
			}
		}
		else{
			cout<<"Put request received but cannot commit!"<<endl;
		}
	}

	else if(strcmp(document["request_type"].GetString(),"get_request")==0){
		assert(document.IsObject());
		assert(document.HasMember("key"));
		assert(document["key"].IsString());
		
		cout<<"In GET request"<<endl;

		char char_key[100];
		strcpy(char_key,document["key"].GetString());
		string key(char_key);
		string value;
		if(document["main_ss"].GetInt() == 0){
			cout<<"sending value from OWN hash table"<<endl;
			value = own[key];
		}
		else if(document["main_ss"].GetInt() == 1){
			cout<<"sending value from previous hash table"<<endl;
			value = previous[key];
		}

		string get_response = get_reponse_fun(value);
		send(tid->new_socket,get_response.c_str(),get_response.length(),0);
		cout<<"value of key: "<<key<<" successfully sent to co-ordination server"<<endl; 

	}
	else if(strcmp(document["request_type"].GetString(),"delete_request")==0){
		assert(document.IsObject());
		assert(document.HasMember("key"));
		assert(document["key"].IsString());

		cout<<"In DEL request"<<endl;

		string req_ack = slave_request_ack(1);
		send(tid->new_socket,req_ack.c_str(),req_ack.length(),0);
		char response[100];
		recv(tid->new_socket,response,100,0);
		Document temp_doc;
		if (temp_doc.ParseInsitu(response).HasParseError()){
			cout<<"Error while parsing the json string while parsing commit message from cs in delete"<<endl;
		}
		else if(temp_doc["commit_status"].GetInt() == 1){
			char char_key[100];
			strcpy(char_key,document["key"].GetString());
			string key(char_key);

			if(document["main_ss"].GetInt() == 0){
				//make changes in own hash table
				own.erase(key);
				// own[key] = value;
				cout<<"deleted key: "<<key<<" from own hash table"<<endl;
			}
			else if(document["main_ss"].GetInt() == 1){
				//make changes in prev hash table
				previous.erase(key);
				// previous[key] = value;
				cout<<"deleted key: "<<key<<" from previous hash table"<<endl;
			}
		}
		else{
			cout<<"DEL request received but cannot commit!"<<endl;
		}
	}
	else if(strcmp(document["request_type"].GetString(),"replicate")==0){

		
		cout<<"CMDBUFFER IS 1 : "<<Buffer<<endl;
		assert(document.IsObject());
		cout << "in replicate request"<<endl;
		cout<<"CMDBUFFER IS: "<<Buffer<<endl;
	
		string send_response = replicate_response_fun();
		cout<<"inside replicate\n";
	
		send(tid->new_socket,send_response.c_str() ,send_response.length() ,0);				
		cout <<"replicate response sent to CS"<<endl;
	}
}

void* heartbeat(void* t){
	//t not being used
	struct hb_thread *tid;
    tid = (struct hb_thread *)t;
	struct sockaddr_in serv_addr;
	int sock = 0;
	cout<<"inside heartbeat\n";
	char* message=tid->ip; //get the slave id
	cout<<"id: "<<tid->id<<" ip"<<tid->ip<<"\n";
	while(1){
		if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
		{ 
			printf("\n Socket creation error \n"); 
		} 
		memset(&serv_addr, '0', sizeof(serv_addr)); 
		serv_addr.sin_family = AF_INET; 
		serv_addr.sin_port = htons(BEATPORT); 

		if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
		{ 
			printf("\nInvalid address/ Address not supported \n"); 
		
		} 	

		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
		{ 
			printf("\nConnection Failed \n"); 
		
		}  
		cout<<"before send\n";
		send(sock , message , strlen(message) , 0 );
        cout<<"sent"<<message<<endl;
		sleep(5);
	}

}
int main(int argc, char const *argv[]) 
{ 				
	struct sockaddr_in serv_addr;
	int sock = 0;

	string temp(argv[1]);
	string slave_ip = temp.substr(0,temp.find(':'));
	string slave_port = temp.substr(temp.find(':')+1);
	cout<<"this is slave ip:port "<<slave_ip<<":"<<slave_port<<endl;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 
	memset(&serv_addr, '0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(CSPORT); 
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0){ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}  

	//--------------------------------Registering slave with co-ordination server--------------------
	string reg_slave = register_slaveserver(slave_ip,slave_port);
	cout<<"this is json string sent to cs for slave registeration: "<<reg_slave<<endl;
	send(sock,reg_slave.c_str(),100,0);
	cout<<"registeration request successfully sent to co-ordination server"<<endl;

	char cs_ack[200];
	recv(sock, cs_ack, 200, 0);
    string ackstring(cs_ack);
	cout<<"Slave successfully registered with the server: "<<ackstring<<endl;
	//--------------------------------Registering slave with co-ordination server--------------------
	close(sock); //closing the socket sock

	pthread_attr_t attr;	
	int server_fd,new_socket; 
	struct sockaddr_in slaveAddress; 
	int addrlen = sizeof(slaveAddress);
	int rc;	

	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   	
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	slaveAddress.sin_family = AF_INET; 
	slaveAddress.sin_addr.s_addr = inet_addr(slave_ip.c_str()); 
	slaveAddress.sin_port = htons(stoi(slave_port)); 
	
	if (bind(server_fd, (struct sockaddr *)&slaveAddress,sizeof(slaveAddress))<0){ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 

	int port=BEATPORT;
	pthread_t thread_heartbeat;
	struct hb_thread hb;
	hb.id=-1;
	char *temp1=new char[256];
	strcpy(temp1,argv[1]); 
	hb.ip=temp1;
    
    if(pthread_create(&thread_heartbeat,NULL,heartbeat,(void*)&hb)<0){
	     perror("Thread error");
	}
	sleep(5);
	
	int i=1;

	while(1){


		// //----------------------------------------connecting to CS
		// char cmdBuffer[100];
		// int rval = read(sock,cmdBuffer,9);
		// cout<<"CMDBUFFER IS: "<<cmdBuffer<<endl;
		// string rp(cmdBuffer); 
		// if(rp == "replicate"){

		// 		cout<<"inside replicate\n";
		// 		// char* ipport = tid->ip;
		// 		cout<<"TEMP: "<<temp<<endl;
		// 		send(sock,temp.c_str() ,temp.length() ,0);				
		// }

		// //------------------------------------------

		pthread_t threads[10];
		struct thread_data td[10];
		if (listen(server_fd, 3) < 0){ 
			cout <<"inside listen" <<endl;
			perror("listen"); 
			exit(EXIT_FAILURE); 
		} 

		cout << "before accept" << endl;
		if ((new_socket = accept(server_fd, (struct sockaddr *)&(slaveAddress),(socklen_t*)&(addrlen)))<0){ 	
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}
		cout << "before read" << endl;
		
		td[i].thread_id = i;
      	td[i].new_socket=new_socket;

		rc = pthread_create(&threads[i], NULL, Service, (void *)&td[i]);
        if (rc){
     		cout << "Error:unable to create thread," << rc << endl;
 		}		
	    pthread_detach(threads[i]);
	  	i++;	
	} 
	return 0; 
} 
