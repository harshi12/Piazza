//./exe ip:port of itself in command line arguments
//ip:port of cordination server in command line arguments

#include <unistd.h>
#include <bits/stdc++.h>
#include <cstdlib>
#include <pthread.h>
#include<string.h>
#include <unistd.h>
#include <unordered_map>
#include <fstream>
#include <unordered_map>
#include <set>
#include "strtoken.hpp"
#include "common_functions.hpp"
#include "BST.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "cache/LRUset_cache.hpp"

#define UDP_PORT 15200
#define NUM_THREADS 5
using namespace rapidjson;
using namespace std;
int number_of_clients = 0;
Document document;
map<int, int> slaveid_socket; // mapping of socket with the salve id
map<int, int> dead_slave;	 // map to store dead slaves
map<int, int> timeout;		  //which threads have timeout
map<int, bool> islive;		  //true if timeout is non-zero, else false

u_int64_t slave_uid = 9223372036854775;
u_int64_t client_uid = 1234567890123456;

int number_of_slave_servers;	   //to keep track of total number of slave servers
int number_of_slave_servers_alive; //to keep track of all the slave servers that are alive. Max of 1 slave can be down at a time

unordered_map<u_int64_t, string> slaveuid_to_ipport;
// unordered_map <u_int64_t,string> clientuid_to_ipport;
unordered_map<string, int> ipport_to_uid; //to keep track of all the slave servers that are already registered, if new slave then add to this map as well
unordered_map<u_int64_t, int> uid_to_socket;

string client_acknowledge(string req_type, string message, int status)
{
	string mystring = " { \"request_type\" : \"" + req_type + "\", \"message\" : \"" + message + "\", \"status\" : \"" + to_string(status) + "\" } ";
	return mystring;
}

string slave_acknowledge(int id_slave, int id_succ, int id_pre, string ipport_slave, string ipport_succ, string ipport_pre)
{
	string mystring = " { \"request_type\" : \"acknowledge_slave_registeration\", \"id_slave\" : " + to_string(id_slave) + ", \"id_succ\" : " + to_string(id_succ) + ", \"id_pre\" : " + to_string(id_pre) + ", \"ipport_slave\" : \"" + ipport_slave + "\", \"ipport_succ\" : \"" + ipport_succ + "\", \"ipport_pre\" : \"" + ipport_pre + "\" } ";
	return mystring;
}

string put_request_slave(string key, string value, int main_ss)
{ //main_ss will say slave server to make changes in own or previous for 0 and 1 value respectively.
	string mystring = " {  \"request_type\" : \"put_request\", \"key\" : \"" + key + "\", \"value\" : \"" + value + "\", \"main_ss\" : " + to_string(main_ss) + " } ";
	return mystring;
}

string get_request_slave(string key, int main_ss)
{
	string mystring = " {  \"request_type\" : \"get_request\", \"key\" : \"" + key + "\", \"main_ss\" : " + to_string(main_ss) + " } ";
	return mystring;
}

string del_request_slave(string key, int main_ss)
{
	string mystring = " {  \"request_type\" : \"delete_request\", \"key\" : \"" + key + "\", \"main_ss\" : " + to_string(main_ss) + " } ";
	return mystring;
}

string slave_commit_func(int status)
{
	string mystring = " {  \"request_type\" : \"commit_operation\", \"commit_status\" : " + to_string(status) + " } ";
	return mystring;
}

string request_slave_replicate(string ipport_pred, string ipport_succ)
{

	string mystring = " {  \"request_type\" : \"replicate\", \"ipport_pred\" : \"" + ipport_pred + "\", \"ipport_succ\" : \"" + ipport_succ + "\" } ";
	return mystring;
}

string get_reponse_fun(string value)
{
	string mystring = " {  \"request_type\" : \"getreq_response\", \"value\" : \"" + value + "\" } ";
	return mystring;
}

Node *root = NULL;

struct thread_data
{
	int thread_id, new_socket;
	string request_string;
	// char* ip_port;
	// int slaveid;
};

//function to listen to heart beat signals
//udp connection!

//sleeps and checks if thread is alive or not

void *heartbeatListener(void *arg)
{

	//convert void* to int
	int port_addr = *((int *)arg);

	//make connection using udp;
	int server_fd, new_socket, valread;
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

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("udp bind failed");
		exit(EXIT_FAILURE);
	}

	while (1)
	{

		char buffer[1024] = {0};

		// read(server_fd,buffer,1024);
		recv(server_fd, buffer, 1024, 0);

		int index = calculate_hash_value(buffer, RING_CAPACITY);
	//	cout << "slave uid " << index << " is alive\n";
		islive[index] = true;
		timeout[index]++;
	}
}

//function to replicate the slave sever in case on slave server is down-------------
void replicate(int slave_key)
{

	Node *pre = NULL, *succ = NULL;
	findPreSuc(root, pre, succ, slave_key);
	Node *pred = pre;
	if (pred == NULL)
	{
		pred = maxValue(root);
	}
	cout << "predecessor of dead slave : " << pred->ipport << endl;

	Node *suc = succ;
	if (suc == NULL)
	{
		suc = minValue(root);
	}
	cout << "successor of dead slave : " << suc->ipport << endl;
	//successor of successor-------------------------------------------
	pre = NULL;
	succ = NULL;
	findPreSuc(root, pre, succ, suc->key);
	Node *suc_of_suc = succ;
	if (suc_of_suc == NULL)
	{
		suc_of_suc = minValue(root);
	}
	cout << "successor of successor of dead slave : " << suc_of_suc->ipport << endl;

	// vector<string>port;
	// const char delimiter = ':';
	// tokenize(suc->ipport,delimiter,port);
	int port = get_port(suc->ipport);
	string ip = get_ip(suc->ipport);
	cout << port << "--------------" << endl;

	int rep_socket = 0;
	rep_socket = to_connect(ip, port);

	cout << "pred->ipport : " << pred->ipport << endl;
	cout << "suc_of_suc->ipport : " << suc_of_suc->ipport << endl;
	string replicate_msg = request_slave_replicate(pred->ipport, suc_of_suc->ipport);
	cout << "--------------" << endl;
	char buff[1024];
	send(rep_socket, replicate_msg.c_str(), replicate_msg.length(), 0); //tid->newsocket
	cout << " sending to successor of dead_slave the message : " << replicate_msg << endl;

	int valread = read(rep_socket, buff, 1024);
	cout << "RECEIVED message from suc after completed replication of dead slave_node" << buff << endl;
}

//function to listen to heart beat signals
//udp connection!
//sleeps and checks if thread is alive or not
void *timer(void *arg)
{

	sleep(30);
	while (1)
	{
		for (int i = 0; i < RING_CAPACITY; i++)
		{
			// cout<<"i: "<<i<<"\n";
			if (timeout[i] == 0 && islive[i] == true)
			{
				islive[i] = false;
				cout << "slave uid " << i << " died\n";
				dead_slave[i] = 0;
				//replicate--------
				replicate(i);
				root = deleteNode(root, i);
				cout << "inorder........:  "<<endl;
				inorder(root);
				cout<<endl;
				cout << "inorder...done " << endl;

			}
			timeout[i] = 0;
		}
		sleep(20);
	}
}

void *ServiceToAny(void *t)
{
	struct thread_data *tid = (struct thread_data *)t;
	tid = (struct thread_data *)t;

	while (1)
	{
		char Buffer[1024];
		int readval = read(tid->new_socket, Buffer, 1024);
		string buffer(Buffer);
		int BufferSize = strlen(Buffer);
		if (BufferSize != 0)
		{
			if (document.ParseInsitu(Buffer).HasParseError())
			{
				cout << "Error while parsing the json string while extracting request type" << endl;
				string client_ack = client_acknowledge("error", "Request incomplete. Try Again!", 0);
				send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
			}
			//-------------------put request-------------------------------------------------
			else if (strcmp(document["request_type"].GetString(), "client_put_request") == 0)
			{
				assert(document.IsObject());
				assert(document.HasMember("key"));
				assert(document.HasMember("value"));
				assert(document["key"].IsString());
				assert(document["value"].IsString());

				char char_key[100];
				strcpy(char_key, document["key"].GetString());
				string key(char_key);
				char char_value[100];
				strcpy(char_value, document["value"].GetString());
				string value(char_value);

				unsigned long slave_id = calculate_hash_value(key, RING_CAPACITY);
				int suc = slave_id;
				Node *pre = NULL, *succ = NULL;
				findPreSuc(root, pre, succ, suc - 1);
				Node *slave_node = succ;
				if (slave_node == NULL)
					slave_node = minValue(root);
				Node *pre1 = NULL, *succ1 = NULL;
				findPreSuc(root, pre1, succ1, slave_node->key);
				Node *suc_of_slave = succ1;
				if (suc_of_slave == NULL)
					suc_of_slave = minValue(root);

				cout << "successor is : ==============" << slave_node->key << endl;
				cout << "slave node is : " << slave_node->ipport << "of id " << slave_id << endl;
				cout << "successor of slave_node is : ==============" << suc_of_slave->key << endl;
				cout << "slave node is : " << suc_of_slave->ipport << endl;

				string slave_ip = get_ip(slave_node->ipport);
				int slave_port = get_port(slave_node->ipport);
				string suc_ip = get_ip(suc_of_slave->ipport);
				int suc_port = get_port(suc_of_slave->ipport);

				int sock_slave, sock_suc;
				sock_slave = to_connect(slave_ip, slave_port);
				sock_suc = to_connect(suc_ip, suc_port);

				if (sock_slave != -1 && sock_suc != -1)
				{
					string putreq_slave = put_request_slave(key, value, 0);
					cout << "Put request for slave: " << putreq_slave << endl;
					send(sock_slave, putreq_slave.c_str(), putreq_slave.length(), 0);
					sleep(1);
					string putreq_suc = put_request_slave(key, value, 1);
					cout << "Put request for successor: " << putreq_suc << endl;
					send(sock_suc, putreq_suc.c_str(), putreq_suc.length(), 0);

					char response_slave[200], response_suc[200];
					recv(sock_slave, response_slave, 200, 0);
					// sleep(1);
					recv(sock_suc, response_suc, 200, 0);
					cout << "response of slave:" << response_slave << endl;
					cout << "response of succ: " << response_suc << endl;

					Document response1, response2;
					response1.Parse(response_slave);
					response2.Parse(response_suc);
					if (response1.HasParseError())
					{
						cout << "DocumentParsing error for put request -- slave" << endl;
						string client_ack = client_acknowledge("put_request_ack", "Request incomplete. Try Again!", 0);
						send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
					}
					if (response2.HasParseError())
					{
						cout << "DocumentParsing error for put request --  succ" << endl;
						string client_ack = client_acknowledge("put_request_ack", "Request incomplete. Try Again!", 0);
						send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
					}
					else
					{
						// response1.Parse(response_slave_copy);
						// response2.Parse(response_slave_copy);
						cout << "Parsing successful" << endl;
						cout << "response of slave:" << response_slave << endl;
						cout << "response of succ: " << response_suc << endl;

						assert(response1.IsObject());
						assert(response1.HasMember("request_status"));
						assert(document["key"].IsString());
						assert(response1["request_status"].IsString());
						assert(response2.IsObject());
						assert(response2.HasMember("request_status"));
						assert(response2["request_status"].IsString());

						cout << "request type slave " << response1["request_status"].GetString() << endl;
						cout << "request type succ " << response2["request_status"].GetString() << endl;
						if (strcmp(response1["request_status"].GetString(), "1") == 0 && strcmp(response2["request_status"].GetString(), "1") == 0)
						{
							string commit_slave = slave_commit_func(1);
							string commit_succ = slave_commit_func(1);
							cout << "commit message for slave: " << commit_slave << endl;
							cout << "commit message for succ: " << commit_succ << endl;
							send(sock_slave, commit_slave.c_str(), commit_slave.length(), 0);
							sleep(1);
							send(sock_suc, commit_succ.c_str(), commit_succ.length(), 0);
							cout << "commit message successfully sent to slave and its successor successfully" << endl;
							string client_ack = client_acknowledge("put_request_ack", "Request Completed!", 1);
							send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
						
						//Delete key from cache
							deleteKey(key);
							cout << "KEY: ( " << key << " )"
								 << "DELETED FROM CACHE" << endl;
						}
						else
						{
							cout << "Cannot commit to both the nodes. Please try again" << endl;
							string client_ack = client_acknowledge("put_request_ack", "Request incomplete. Try Again!", 0);
							send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
						}
					}
				}
				else
				{
					cout << "either of slave or its successor is down. Please try again!" << endl;
					string client_ack = client_acknowledge("put_request_ack", "Request incomplete. Try Again!", 0);
					send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
				}
				close(sock_slave);
				close(sock_suc);
			}
			//-------------------------get request----------------------------------------
			else if (strcmp(document["request_type"].GetString(), "client_get_request") == 0)
			{
				assert(document.IsObject());
				assert(document.HasMember("key"));
				assert(document["key"].IsString());

				char char_key[100];
				strcpy(char_key, document["key"].GetString());
				string key(char_key);

				// get key, value if exits in cache
				string valuefromcache = getValue(key);
				if (valuefromcache != "none")
				{
					string get_response = get_reponse_fun(valuefromcache);
					send(tid->new_socket, get_response.c_str(), get_response.length(), 0);
					cout << "VALUE: " << valuefromcache << " SENT TO CLIENT FROM CACHE" << endl;
				}
				else
				{
					unsigned long slave_id = calculate_hash_value(key, RING_CAPACITY);
					int suc = slave_id;
					Node *pre = NULL, *succ = NULL;
					findPreSuc(root, pre, succ, suc - 1);
					Node *slave_node = succ;
					if (slave_node == NULL)
						slave_node = minValue(root);
					cout << "slave node is : " << slave_node->ipport << "of id " << slave_id << endl;
					cout << "successor is : ==============" << slave_node->key << endl;

					string slave_ip = get_ip(slave_node->ipport);
					int slave_port = get_port(slave_node->ipport);
					int sock_slave;
					sock_slave = to_connect(slave_ip, slave_port);
					string value;

					if (sock_slave != -1)
					{
						string get_request = get_request_slave(key, 0);
						send(sock_slave, get_request.c_str(), get_request.length(), 0);
						cout << "value of the key: " << key << " requested from slave server" << endl;
						char char_val[200];
						memset(char_val, 0, sizeof(char_val));
						recv(sock_slave, char_val, 200, 0);
						cout << "slave's response for get: " << char_val << endl;
						Document response;
						response.Parse(char_val);
						if (response.HasParseError())
						{
							cout << "Error while parsing the json response of slave server for get request" << endl;
							string client_ack = client_acknowledge("error", "Request incomplete. Try Again!", 0);
							send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
						}
						else
						{
							cout << "Parsing successful" << endl;
							cout << "slave server's response: " << char_val << endl;
							char temp[100];
							strcpy(temp, response["value"].GetString());
							value = temp;

							string get_response = get_reponse_fun(value);
							send(tid->new_socket, get_response.c_str(), get_response.length(), 0);
							cout << "value successfully sent to client" << endl;
						}
						close(sock_slave);
					}
					else
					{
						Node *pre = NULL, *succ = NULL;
						findPreSuc(root, pre, succ, slave_node->key);
						Node *suc_of_slave = succ;
						// Node *suc_of_slave = findPreSuc(root,slave_node->key+1);
						if (suc_of_slave == NULL)
							suc_of_slave = minValue(root);

						cout << "slave node is : " << suc_of_slave->ipport << endl;
						cout << "successor of slave_node is : ==============" << suc_of_slave->key << endl;

						string suc_ip = get_ip(suc_of_slave->ipport);
						int suc_port = get_port(suc_of_slave->ipport);

						int sock_suc;
						sock_suc = to_connect(suc_ip, suc_port);

						string get_request = get_request_slave(key, 1);
						send(sock_slave, get_request.c_str(), get_request.length(), 0);
						cout << "value of the key: " << key << " requested from slave server" << endl;
						char char_val[200];
						memset(char_val, 0, sizeof(char_val));
						recv(sock_slave, char_val, 200, 0);
						Document response;
						if (response.ParseInsitu(char_val).HasParseError())
						{
							cout << "Error while parsing the json response of successor server for get request" << endl;
							string client_ack = client_acknowledge("error", "Request incomplete. Try Again!", 0);
							send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
						}
						else
						{
							cout << "Parsing successful" << endl;
							cout << "slave server's response: " << char_val << endl;
							char temp[100];
							strcpy(temp, response["value"].GetString());
							value = temp;

							string get_response = get_reponse_fun(value);
							send(tid->new_socket, get_response.c_str(), get_response.length(), 0);
							cout << "value successfully sent to client" << endl;
						}
						close(sock_suc);
					}
					//put recieved value to cache if not empty
					if (value != "")
					{
						putInSet(key, value);
						cout << "KEY, VALUE: ( " << key << " , " << value << " ) "
							 << "PUT TO CACHE" << endl;
					}
				}
			}
			//-----------------------------delete request-----------------------------------
			else if (strcmp(document["request_type"].GetString(), "client_delete_request") == 0)
			{
				assert(document.IsObject());
				assert(document.HasMember("key"));
				assert(document["key"].IsString());
				cout << "In DEL request" << endl;

				char char_key[100];
				strcpy(char_key, document["key"].GetString());
				string key(char_key);

				unsigned long slave_id = calculate_hash_value(key, RING_CAPACITY);
				int suc = slave_id;

				Node *pre = NULL, *succ = NULL;
				findPreSuc(root, pre, succ, suc - 1);
				Node *slave_node = succ;

				if (slave_node == NULL)
					slave_node = minValue(root);
				Node *pre1 = NULL, *succ1 = NULL;
				findPreSuc(root, pre1, succ1, slave_node->key);
				Node *suc_of_slave = succ1;
				if (suc_of_slave == NULL)
					suc_of_slave = minValue(root);
				cout << "successor is : ==============" << slave_node->key << endl;
				cout << "slave node is : " << slave_node->ipport << "of id " << slave_id << endl;
				cout << "successor of slave_node is : ==============" << suc_of_slave->key << endl;
				cout << "slave node is : " << suc_of_slave->ipport << endl;

				string slave_ip = get_ip(slave_node->ipport);
				int slave_port = get_port(slave_node->ipport);
				string suc_ip = get_ip(suc_of_slave->ipport);
				int suc_port = get_port(suc_of_slave->ipport);

				int sock_slave, sock_suc;
				sock_slave = to_connect(slave_ip, slave_port);
				sock_suc = to_connect(suc_ip, suc_port);

				if (sock_slave != -1 && sock_suc != -1)
				{
					string delreq_slave = del_request_slave(key, 0);
					send(sock_slave, delreq_slave.c_str(), delreq_slave.length(), 0);
					sleep(1);
					string delreq_suc = del_request_slave(key, 1);
					send(sock_suc, delreq_suc.c_str(), delreq_suc.length(), 0);

					char response_slave[200], response_suc[200];
					recv(sock_slave, response_slave, 200, 0);
					sleep(1);
					recv(sock_suc, response_suc, 200, 0);

					Document response1, response2;
					response1.Parse(response_slave);
					response2.Parse(response_suc);
					if (response1.HasParseError())
					{
						cout << "DocumentParsing error for delete request" << endl;
						string client_ack = client_acknowledge("error", "Request incomplete. Try Again!", 0);
						send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
					}
					if (response2.HasParseError())
					{
						cout << "DocumentParsing error for delete request" << endl;
						string client_ack = client_acknowledge("error", "Request incomplete. Try Again!", 0);
						send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
					}
					else
					{
						cout << "Parsing successful" << endl;
						cout << "response of slave:" << response_slave << endl;
						cout << "response of succ: " << response_suc << endl;

						if (strcmp(response1["request_status"].GetString(), "1") == 0 && strcmp(response2["request_status"].GetString(), "1") == 0)
						{
							string commit_slave = slave_commit_func(1);
							string commit_succ = slave_commit_func(1);
							send(sock_slave, commit_slave.c_str(), commit_slave.length(), 0);
							sleep(1);
							send(sock_suc, commit_succ.c_str(), commit_succ.length(), 0);
							cout << "commit message successfully sent to slave and its successor successfully" << endl;
							string client_ack = client_acknowledge("del_request_ack", "Request completed!", 0);
							send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);

							//Delete key from cache
							deleteKey(key);
							cout << "KEY: ( " << key << " )"
								 << "DELETED FROM CACHE" << endl;
						}
						else
						{
							cout << "Cannot commit to both the nodes. Please try again" << endl;
							string client_ack = client_acknowledge("del_request_ack", "Request incomplete. Try Again!", 0);
							send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
						}
					}
					close(sock_slave);
					close(sock_suc);
				}
				else
				{
					cout << "either of slave or its successor is down. Please try again!" << endl;
					string client_ack = client_acknowledge("del_request_ack", "Request incomplete. Try Again!", 0);
					send(tid->new_socket, client_ack.c_str(), client_ack.length(), 0);
				}
			}
		}
		memset(Buffer, 0, sizeof(Buffer));
	}
}

int main(int argc, char const *argv[])
{
	pthread_attr_t attr;
	void *status;
	int server_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	string server_ip;
	int server_port;
	//initialise cache
	initialise();
	if(argc < 1){
		cout<<"Please enter the ip:port of the co-ordination server"<<endl;
		exit(1);
	}
	else{
		string server_ipport(argv[1]);
		server_ip = get_ip(server_ipport);
		server_port = get_port(server_ipport);
	}

	for (int i = 0; i < RING_CAPACITY; i++)
	{
		timeout[i] = 0;
	}

	for (int i = 0; i < RING_CAPACITY; i++)
	{
		islive[i] = false;
	}

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int rc;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(server_ip.c_str());
	address.sin_port = htons(server_port);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	int i = 1;
	cout << "SERVER is online" << endl;

	pthread_t hb_thread;
	int port = UDP_PORT;

	if (pthread_create(&hb_thread, NULL, heartbeatListener, (void *)&port) < 0)
	{
		perror("Error! ");
	}

	//a thread which checks live status every 5 secs
	pthread_t timer_thread;

	if (pthread_create(&timer_thread, NULL, timer, (void *)&port) < 0)
	{
		perror("Error!");
	}

	while (1)
	{
		pthread_t threads[10];
		struct thread_data td[10];
		char Buffer[1024] = {0};

		if (listen(server_fd, 3) < 0)
		{
			perror("listen");
			exit(EXIT_FAILURE);
		}
		if ((new_socket = accept(server_fd, (struct sockaddr *)&(address), (socklen_t *)&(addrlen))) < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		td[i].thread_id = i;
		td[i].new_socket = new_socket;

		int readval = read(new_socket, Buffer, 100);

		//------------------parsing json document-----------------------------------
		string buffer(Buffer);
		cout << "--------------\n";
		cout << "register request> " << buffer << endl;
		if (document.ParseInsitu(Buffer).HasParseError())
		{
			cout << "Error while parsing the json string while registeration of client" << endl;
		}

		//--------------------code to register a client with the co-ordination server-----------------
		else if (strcmp(document["request_type"].GetString(), "register_client") == 0)
		{
			assert(document.IsObject());

			cout << "Parsing of the document for client registeration is successful" << endl;
			string mystring_here = client_acknowledge("acknowledge_client_registeration", "registeration successful", 1);
			cout << "client_acknowledge> " << mystring_here << endl;
			send(new_socket, mystring_here.c_str(), 200, 0);
			cout << "Acknowledgement successfully sent to the client" << endl;
			rc = pthread_create(&threads[i], NULL, ServiceToAny, (void *)&td[i]);
			if (rc)
			{
				cout << "Error:unable to create thread," << rc << endl;
			}
			pthread_detach(threads[i]);
			i++;
			cout << "--------------\n";
		}
		//--------------------code to register a client with the co-ordination server-----------------

		//--------------------code to register a slave with the co-ordination server-------------------
		else if (strcmp(document["request_type"].GetString(), "register_slave") == 0)
		{
			assert(document.IsObject());
			assert(document.HasMember("slave_ip"));
			assert(document.HasMember("slave_port"));
			assert(document["slave_ip"].IsString());
			assert(document["slave_port"].IsString());

			cout << "Parsing of the document for client registeration is successful" << endl;

			//int registeration_id = slave_uid++;
			char slave_ipport[100];
			strcpy(slave_ipport, document["slave_ip"].GetString());
			strcat(slave_ipport, ":");
			strcat(slave_ipport, document["slave_port"].GetString());
			string sl_ipport(slave_ipport);

			cout << "This is slave ip:port " << sl_ipport << endl;
			cout << "Slave registered but acknowledgement is left" << endl;
			string slave_ack_string;
			int id; //slave server's hashed id

			//----------differentiate among already registered slave server BEGIN-----------------
			if (ipport_to_uid.find(sl_ipport) != ipport_to_uid.end())
			{
				//to ensure that a slave sever gets the same id if it was registered once with co-ordination server
				id = ipport_to_uid[sl_ipport];
			}
			else
			{
				int id_temp = calculate_hash_value(sl_ipport, RING_CAPACITY);
				root = insert(root, id_temp, sl_ipport); //to insert the newly registered slave server to BST
				ipport_to_uid[sl_ipport] = id_temp;
				id = id_temp;
			}
			//----------differentiate among already registered slave server END-----------------
			slaveid_socket[id] = new_socket; //store socket of this connection
			cout << "Hashed slave id is: " << id << endl;
			cout << "ADDING SOCKET: " << new_socket << endl;
			//-------------------get predecessor and successor of the slave node----------------------
			int suc = id;
			Node *pre = NULL, *succ = NULL;
			findPreSuc(root, pre, succ, suc);
			Node *suc_of_slave = succ;
			if (suc_of_slave == NULL)
				suc_of_slave = minValue(root);
			if (pre == NULL)
				pre = maxValue(root);
			cout << "INORDER=========================\n";
			inorder(root);
			cout << "INORDER=========================\n";
			cout << "successor of slave_node is : ==============" << suc_of_slave->key << endl;
			cout << "slave node is : " << suc_of_slave->ipport << endl;
			cout << "predecessor of slave_node is : ==============" << pre->key << endl;
			cout << "predecessor node is : " << pre->ipport << endl;

			int succ_id = ipport_to_uid[suc_of_slave->ipport];
			int pre_id = ipport_to_uid[pre->ipport];
			//-------------------get predecessor and successor of the slave node----------------------
			slave_ack_string = slave_acknowledge(id, succ_id, pre_id, sl_ipport, suc_of_slave->ipport, pre->ipport);
			//string mystring_here = slave_acknowledge(registeration_id,sl_ipport);
			cout << "json string to acknowledge slave registeration " << slave_ack_string << endl
				 << endl;
			send(new_socket, slave_ack_string.c_str(), 300, 0);
			cout << "acknowledge successfully sent to the slave" << endl;
		}
		//--------------------code to register a slave with the co-ordination server-------------------
	}
	return 0;
}
