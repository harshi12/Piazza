//  g++ -pthread slaveServer1.cpp -o SS
//  ./SS 127.0.0.1:8081 127.0.0.1:8080


#include <bits/stdc++.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include "common_functions.hpp"
#include <fstream>
#include <unordered_map>
#include <set>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <semaphore.h>
// #define CSPORT 8080
#define BEATPORT 15200
#define NUM_THREADS 5
using namespace std;
using namespace rapidjson;

Document document;

unordered_map<string, string> own;
unordered_map<string, string> previous;

int readcount = 0;
int put = 1;

mutex mtxlock;

string cordination_ip; //global variable to store ip of cordination server.

void print_own_table(){
	unordered_map<string, string>::iterator miter;
	cout<<"=============OWN================\n"<<endl;
	cout<<"KEY -> VALUE       \n"<<endl;
	for(miter = own.begin();miter != own.end(); ++miter){
		cout<<miter->first<<" -> "<< miter->second<<endl;
	}
	cout<<"================================\n"<<endl;

}

void print_previous_table(){
	unordered_map<string, string>::iterator miter;
	cout<<"============PREVIOUS===========\n"<<endl;
	cout<<"KEY -> VALUE       \n"<<endl;
	for(miter = previous.begin();miter != previous.end(); ++miter){
		cout<<miter->first<<" -> "<< miter->second<<endl;
	}
	cout<<"================================\n"<<endl;

}

string register_slaveserver(string slave_ip, string slave_port)
{
	string mystring = " {  \"request_type\" : \"register_slave\", \"slave_ip\" : \"" + slave_ip + "\", \"slave_port\" : \"" + slave_port + "\" } ";
	return mystring;
}

string slave_request_ack(int status)
{
	string mystring = " {  \"request_type\" : \"slave_ack\", \"request_status\" : \"" + to_string(status) + "\" } ";
	return mystring;
}

string get_reponse_fun(string value)
{
	string mystring = " {  \"request_type\" : \"getreq_response\", \"value\" : \"" + value + "\" } ";
	return mystring;
}

string dead_slave_pred_request()
{
	string mystring = " {  \"request_type\" : \"get_own_from_pred\" } ";
	return mystring;
}

string get_own_from_pred()
{
	string mystring = " {  \"request_type\" : \"get_own_from_pred\" } ";
	return mystring;
}

// string replicate_response_fun()
// {
// 	cout<<"inside final response for CS by suc server "<<endl;
// 	string repl_json = "{";
// 	unordered_map<string, string>::iterator mapitr;
// 	cout << "inside rep response" << endl;
// 	if (!own.empty())
// 	{
// 		for (mapitr = own.begin(); mapitr != own.end(); ++mapitr)
// 		{

// 			string repl_json = "{";
// 			unordered_map<string, string>::iterator mapitr;

// 			if (!own.empty())
// 			{
// 				for (mapitr = own.begin(); mapitr != own.end(); ++mapitr)
// 				{

// 					cout << "map elements: " << mapitr->first << " " << mapitr->second << endl;
// 					repl_json = repl_json + " \"" + mapitr->first + "\" : \"" + mapitr->second + "\", ";
// 				}
// 				repl_json[repl_json.length() - 2] = ' ';
// 				repl_json[repl_json.length() - 1] = '}';

// 				cout << " REPLICATE RESPONSE JSON : " << repl_json << endl;
// 			}
// 			else
// 			{
// 				repl_json = slave_request_ack(0); //status bit 0 represents that the operation has failed! try again.
// 			}
// 			// string mystring = " {  \"request_type\" : \"replicate_response\" } ";
// 			return repl_json;
// 		}
// 	}
// }

string get_keys_from_succ(int slave_id, int pre_id)
{
	string mystring = " {  \"request_type\" : \"get_keys_from_succ\", \"slave_id\" : " + to_string(slave_id) + ", \"pre_id\" : " + to_string(pre_id) + " } ";
	return mystring;
}

string replicate_response_fun()
{
	string repl_json = " { \"request_type\" : \"get_own_from_pred\", \"request_status\" : ";
	unordered_map<string, string>::iterator mapitr;
	cout << "inside rep response" << endl;
	if (!own.empty())
	{
		repl_json += to_string(1) + ", ";
		for (mapitr = own.begin(); mapitr != own.end(); ++mapitr)
		{
			cout << "map elements: " << mapitr->first << " " << mapitr->second << endl;
			repl_json = repl_json + " \"" + mapitr->first + "\" : \"" + mapitr->second + "\", ";
		}
		repl_json[repl_json.length() - 2] = ' ';
		repl_json[repl_json.length() - 1] = '}';

		cout << " REPLICATE RESPONSE JSON : " << repl_json << endl;
	}
	else
	{
		repl_json += "0 } "; //status bit 0 represents that the operation has failed! try again.
	}
	// string mystring = " {  \"request_type\" : \"replicate_response\" } ";
	return repl_json;
}

string json_generator_to_successor()
{
	cout << "inside final response for CS by suc server " << endl;
	string repl_json = "{ \"request_type\" : \"get_own_from_succ\",";
	unordered_map<string, string>::iterator mapitr;
	cout << "inside rep response" << endl;
	if (!own.empty())
	{
		for (mapitr = own.begin(); mapitr != own.end(); ++mapitr)
		{

			cout << "map elements: " << mapitr->first << " " << mapitr->second << endl;
			repl_json = repl_json + " \"" + mapitr->first + "\" : \"" + mapitr->second + "\", ";
		}
		repl_json[repl_json.length() - 2] = ' ';
		repl_json[repl_json.length() - 1] = '}';

		cout << " SUC OWN DATA FOR SUC OF SUC : " << repl_json << endl;
	}
	else
	{
		repl_json = slave_request_ack(0); //status bit 0 represents that the operation has failed! try again.
	}
	// string mystring = " {  \"request_type\" : \"replicate_response\" } ";
	return repl_json;
}

struct thread_data
{
	int thread_id, new_socket;
};

struct hb_thread
{
	int id;
	char *ip;
};

struct thread_connection_establish
{
	int thread_id;
	char response_string[300];
	Document doc_thread;
};

void *Service(void *t)
{
	struct thread_data *tid;
	tid = (struct thread_data *)t;

	char Buffer[1024];
	string buffer(Buffer);
	memset(Buffer, 0, sizeof(Buffer));
	int readval = read(tid->new_socket, Buffer, 1024);
	cout << "BUFFER: " << Buffer << endl;
	document.Parse(Buffer);
	if (document.HasParseError())
	{
		cout << "Error while parsing the json string while extracting request type from cs" << endl;
	}
	else if (strcmp(document["request_type"].GetString(), "get_own_from_pred") == 0)
	{
		cout << "Sending my own table to my successor!" << endl;
		string response_string = replicate_response_fun();
		// 		// char* ipport = tid->ip;
		// 		cout<<"TEMP: "<<temp<<endl;
		// 		send(sock,temp.c_str() ,temp.length() ,0);
		// }

		// //------------------------------------------tring = replicate_response_fun();
		cout << "This is the json string of my OWN table: " << response_string;
		send(tid->new_socket, response_string.c_str(), response_string.length(), 0);
		cout << "Response successfully sent!" << endl;
	}
	else if (strcmp(document["request_type"].GetString(), "get_keys_from_succ") == 0)
	{
		assert(document.IsObject());
		assert(document.HasMember("slave_id"));
		assert(document.HasMember("pre_id"));
		// cout << 1 << endl;
		assert(document["slave_id"].IsInt());
		// cout << 2 << endl;
		assert(document["pre_id"].IsInt());

		int slave_id = document["slave_id"].GetInt();
		int pre_id = document["pre_id"].GetInt();

		cout << "Will filter and send keys mapped between " << pre_id << " and " << slave_id << " to the requesting node" << endl;

		string repl_json = "{ \"request_type\" : \"filtered_keys_from_succ\", \"map_status\" : ";
		unordered_map<string, string>::iterator mapitr;
		cout << "before starting to filter out the values" << endl;
		if (!own.empty())
		{
			repl_json += "1, ";
			for (mapitr = own.begin(); mapitr != own.end(); ++mapitr)
			{
				int hashed_key = calculate_hash_value(mapitr->first, RING_CAPACITY);
				if (hashed_key > pre_id && hashed_key <= slave_id)
				{
					cout << "successfully filtered map elements: " << mapitr->first << " " << mapitr->second << endl;
					repl_json = repl_json + " \"" + mapitr->first + "\" : \"" + mapitr->second + "\", ";
					own.erase(mapitr);
				}
			}
			repl_json[repl_json.length() - 2] = ' ';
			repl_json[repl_json.length() - 1] = '}';

			cout << " Final response with all the selected key,value pairs : " << repl_json << endl;
		}
		else
		{
			repl_json += "0 } "; //status bit 0 represents that the operation has failed! try again.
		}
		send(tid->new_socket, repl_json.c_str(), repl_json.length(), 0);
		cout << "Response successfully sent!" << endl;
	}
	else if (strcmp(document["request_type"].GetString(), "put_request") == 0)
	{
		assert(document.IsObject());
		assert(document.HasMember("key"));
		assert(document.HasMember("value"));
		assert(document["key"].IsString());
		assert(document["value"].IsString());

		cout << "In PUT request" << endl;

		string key = document["key"].GetString();
		string value = document["value"].GetString();
		int main_ss = document["main_ss"].GetInt();
		cout << "key: " << key << " value: " << value << " main_ss: " << main_ss << endl;
		string req_ack = slave_request_ack(1);
		cout << "slave ack string: " << req_ack << endl;
		send(tid->new_socket, req_ack.c_str(), req_ack.length(), 0);
		sleep(1);
		char response[100];
		recv(tid->new_socket, response, 100, 0);
		Document temp_doc;
		if (temp_doc.ParseInsitu(response).HasParseError())
		{
			cout << "Error while parsing the json string while parsing commit message from cs in put" << endl;
		}
		else if (temp_doc["commit_status"].GetInt() == 1)
		{
			// entry section
			mtxlock.lock();
			while (readcount != 0)
				;
			put--;
			// critical section
			if (main_ss == 0)
			{
				//make changes in own hash table
				own[key] = value;
				cout << "added key: " << key << " and value: " << value << " to own hash table" << endl;
				print_own_table();
			}
			else if (main_ss == 1)
			{
				//make changes in prev hash table
				previous[key] = value;
				cout << "added key: " << key << " and value: " << value << " to previous hash table" << endl;
				print_previous_table();
			}
			
			
			// exit section
			put++;
			mtxlock.unlock();
			
			// ./client 192.168.43.252:6000 192.168.43.174:4578
		}
		else
		{
			cout << "Put request received but cannot commit!" << endl;
		}
	}

	else if (strcmp(document["request_type"].GetString(), "get_own_from_pred") == 0)
	{

		cout << "inside dead slave's predecessor " << endl;
		assert(document.IsObject());
		string json = replicate_response_fun();
		cout << "JSON DATA OF DEAD SLAVE PRED OWN TABLE :" << json << endl;
		send(tid->new_socket, json.c_str(), json.length(), 0);
	}
	else if (strcmp(document["request_type"].GetString(), "get_own_from_succ") == 0)
	{

		cout << "inside dead slave's successor " << endl;
		assert(document.IsObject());
		//string json = replicate_response_fun();
		cout << "JSON DATA OF DEAD SLAVE SUC OF SUC OWN TABLE :" << Buffer << endl;

		cout << "previous of suc_suc_own before updation" << endl;
		unordered_map<string, string>::iterator it;
		for (it = previous.begin(); it != previous.end(); ++it)
		{
			cout << it->first << "->" << it->second << endl;
		}

		cout << " size of previous before updating suc_suc_own table ***************************" << previous.size();
		for (Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr)
		{
			string name1 = itr->name.GetString();
			//string key = "\""+name1+"\"";
			Value::ConstMemberIterator itr1 = document.FindMember(itr->name);

			if (name1 != "request_type")
			{
				cout << "NAME  " << name1 << "========= VALUE " << itr1->value.GetString() << endl;
				previous[name1] = itr1->value.GetString();
			}
		}
		cout << " size of previous after updating suc_suc_own table ****************************" << previous.size();

		print_previous_table();
		
		string msg = " { \"status\" : \"own_updation_done\" } ";
		send(tid->new_socket, msg.c_str(), msg.length(), 0);
	}

	else if (strcmp(document["request_type"].GetString(), "get_request") == 0)
	{
		assert(document.IsObject());
		assert(document.HasMember("key"));
		assert(document["key"].IsString());

		cout << "In GET request" << endl;

		char char_key[100];
		strcpy(char_key, document["key"].GetString());
		string key(char_key);
		string value;
		//entry section
		int flag = 0;
		if (put == 0)
		{
			mtxlock.lock();
			flag = 1;
		}
		readcount++;

		//critical section
		if (document["main_ss"].GetInt() == 0)
		{
			cout << "sending value from OWN hash table" << endl;
			value = own[key];
		}
		else if (document["main_ss"].GetInt() == 1)
		{
			cout << "sending value from previous hash table" << endl;
			value = previous[key];
		}
		string get_response = get_reponse_fun(value);
		send(tid->new_socket, get_response.c_str(), get_response.length(), 0);
		cout << "value of key: " << key << " successfully sent to co-ordination server" << endl;
		//exit section
		readcount--;
		if (flag == 1)
			mtxlock.unlock();
	}
	else if (strcmp(document["request_type"].GetString(), "delete_request") == 0)
	{
		assert(document.IsObject());
		assert(document.HasMember("key"));
		assert(document["key"].IsString());

		cout << "In DEL request" << endl;
		//-----retrieve main_ss and key from the json string BEGIN--------------
		int main_ss = document["main_ss"].GetInt();
		char char_key[100];
		strcpy(char_key, document["key"].GetString());
		string key(char_key);
		//-----retrieve main_ss and key from the json string END--------------

		string req_ack = slave_request_ack(1);
		send(tid->new_socket, req_ack.c_str(), req_ack.length(), 0);
		char response[100];
		recv(tid->new_socket, response, 100, 0);
		Document temp_doc;
		if (temp_doc.ParseInsitu(response).HasParseError())
		{
			cout << "Error while parsing the json string while parsing commit message from cs in delete" << endl;
		}
		else if (temp_doc["commit_status"].GetInt() == 1)
		{
			// entry section
			mtxlock.lock();
			while (readcount != 0)
				;
			put--;
			// critical section
			if (main_ss == 0)
			{
				//make changes in own hash table
				own.erase(key);
				// own[key] = value;
				cout << "deleted key: " << key << " from own hash table" << endl;
			}
			else if (main_ss == 1)
			{
				//make changes in prev hash table
				previous.erase(key);
				// previous[key] = value;
				cout << "deleted key: " << key << " from previous hash table" << endl;
			}
			// exit section
			put++;
			mtxlock.unlock();
		}
		else
		{
			cout << "DEL request received but cannot commit!" << endl;
		}
	}
	else if (strcmp(document["request_type"].GetString(), "replicate") == 0)
	{

		cout << "CMDBUFFER IS 1 : " << Buffer << endl;
		cout << "in replicate request" << endl;
		// cout<<"CMDBUFFER IS: "<<Buffer<<endl;

		// 		string send_response = replicate_response_fun();
		// 		cout<<"response string generated: "<<send_response<<endl;
		// 		cout<<"inside replicate\n";

		// 		send(tid->new_socket,send_response.c_str() ,send_response.length() ,0);
		// 		cout <<"replicate response sent to CS"<<endl;
		// 	}
		// 	memset(Buffer,0,sizeof(Buffer));
		// 	close(tid->new_socket);
		// }
		cout << "CMDBUFFER IS 2: " << Buffer << endl;

		assert(document.IsObject());
		assert(document.HasMember("ipport_succ")); //ipport of dead node's successor's successor
		assert(document.HasMember("ipport_pred")); //ipport of dead node's predecessor
		assert(document["ipport_succ"].IsString());
		assert(document["ipport_pred"].IsString());

		string ipport_succ = document["ipport_succ"].GetString();
		string ipport_pred = document["ipport_pred"].GetString();

		string ip_of_succ = get_ip(ipport_succ);
		int port_of_succ = get_port(ipport_succ);
		string ip_of_pred = get_ip(ipport_pred);
		int port_of_pred = get_port(ipport_pred);

		//copying it's own 'previous' in it's own table---------------
		unordered_map<string, string>::iterator ownitr;
		unordered_map<string, string>::iterator previtr;
		//cout << "SIZE1 OF PREVIOUS BEFORE ERASING: " << previous.size();
		for (previtr = previous.begin(); previtr != previous.end(); ++previtr)
		{
			//cout << "copying " << previtr->first << " to first and " << previtr->second << " to second of own table" << endl;
			own[previtr->first] = previtr->second;
			//previous.erase(previtr);
		}
		previous.clear();

		//cout << "SIZE OF PREVIOUS TABLE AFTER DELETION : " << previous.size() << endl;
		print_previous_table();
		print_own_table();

		//establishing connection with dead slave's predecessor to get it's own 'content=============
		int sock_cs;
		struct sockaddr_in cs_serv_addr;
		//char buffer[1024] = {0};

		if ((sock_cs = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("\n Socket creation error \n");
		}

		memset(&cs_serv_addr, '0', sizeof(cs_serv_addr));
		cs_serv_addr.sin_family = AF_INET;
		cs_serv_addr.sin_port = htons(port_of_pred);

		// Convert IPv4 and IPv6 addresses from text to binary form
		if (inet_pton(AF_INET, ip_of_pred.c_str(), &cs_serv_addr.sin_addr) <= 0)
		{
			printf("\nreplicate: Invalid address/ Address not supported \n");
		}

		if (connect(sock_cs, (struct sockaddr *)&cs_serv_addr, sizeof(cs_serv_addr)) < 0)
		{
			printf("\nConnection Failed \n");
		}
		cout << "Connection successfully established with pred of slave server" << endl;
		char buffer[1024];
		memset(buffer, 0, sizeof(buffer));
		string message = dead_slave_pred_request();

		send(sock_cs, message.c_str(), message.length(), 0);
		int valread = read(sock_cs, buffer, 1024);
		//cout << " data received from predecessor of the dead slave " << buffer << endl;

		//adding the data from dead slave's pred hash table that is just received----------
		//cout << "SIZE3 OF PREVIOUS: " << previous.size();
		Document doc;
		doc.Parse(buffer);
		//cout << "parsing here: " << buffer << endl;
		if (doc.HasParseError())
		{
			cout << "Error while parsing the json string while extracting request type from cs" << endl;
		}
		else
		{
			for (Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
			{
				string name1 = itr->name.GetString();
				string key = "\"" + name1 + "\"";
				Value::ConstMemberIterator itr1 = doc.FindMember(itr->name);
				if (strcmp(name1.c_str(), "request_type") == 0 || strcmp(name1.c_str(), "request_status") == 0)
				{
					continue;
				}
				else
				{
					cout << "NAME  " << name1 << "========= VALUE " << itr1->value.GetString() << endl;
					previous[name1] = itr1->value.GetString();
				}
			}
			print_previous_table();
		}
		//cout << "SIZE OF PREVIOUS OWN OF PRED OF DEAD SLAVE : " << previous.size() << endl;
		close(sock_cs);
		//=================================================================================================

		//new connction with the successor of successor of dead slave-----
		int sock_suc;
		//char buffer[1024] = {0};
		struct sockaddr_in serv_addr;

		if ((sock_suc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("\n Socket creation error \n");
		}

		memset(&serv_addr, '0', sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(port_of_succ);

		// Convert IPv4 and IPv6 addresses from text to binary form
		if (inet_pton(AF_INET, ip_of_succ.c_str(), &serv_addr.sin_addr) <= 0)
		{
			printf("\nreplicate2: Invalid address/ Address not supported \n");
		}

		if (connect(sock_suc, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			printf("\nConnection Failed \n");
		}
		cout << "Connection successfully established with pred of slave server" << endl;
		char buffer1[1024];
		message = json_generator_to_successor();

		cout << " data sending to successor of successor of the dead slave " << message << endl;
		send(sock_suc, message.c_str(), message.length(), 0);

		int vr = read(sock_suc, buffer1, 1024);
		cout << "acknowldgement from successor of successor of dead slave " << buffer1 << endl;
		close(sock_suc);

		string done = " { \"status\" : \"replication_done\" } ";
		send(tid->new_socket, done.c_str(), 1024, 0);
		//==================================================================================================
	}
	memset(Buffer, 0, sizeof(Buffer));
	close(tid->new_socket);
}

void *heartbeat(void *t)
{
	
	struct hb_thread *tid;
	tid = (struct hb_thread *)t;
	struct sockaddr_in serv_addr;
	int sock = 0;
	cout<<"ip of cordination server: "<<cordination_ip<<endl;	

	char *message = tid->ip; //get the slave id
	int sid = calculate_hash_value(tid->ip,RING_CAPACITY);
	while (1)
	{
		if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			printf("\n Socket creation error \n");
		}
		memset(&serv_addr, '0', sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(cordination_ip.c_str());
		serv_addr.sin_port = htons(BEATPORT);
		if (inet_pton(AF_INET, cordination_ip.c_str(), &serv_addr.sin_addr) <= 0)
		{

			printf("\nheartbeat: Invalid address/ Address not supported \n");

		}

		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			printf("\nConnection Failed in heartbeat \n");
		}

		send(sock, message, strlen(message), 0);

		cout << "sent>> " << message <<" slave id : "<<sid<< endl;

		sleep(5);
	}
}

void *get_data(void *t)
{
	// cout << "in thread" << endl;
	thread_connection_establish *tid = (thread_connection_establish *)t;
	document.Parse(tid->response_string);
	assert(document.IsObject());
	assert(document.HasMember("id_succ"));
	assert(document.HasMember("id_slave"));
	assert(document.HasMember("id_pre"));
	assert(document.HasMember("ipport_succ"));
	assert(document.HasMember("ipport_pre"));
	assert(document["id_succ"].IsInt());
	assert(document["id_slave"].IsInt());
	assert(document["id_pre"].IsInt());
	assert(document["ipport_succ"].IsString());
	assert(document["ipport_pre"].IsString());

	string succ_ipport = document["ipport_succ"].GetString();
	string succ_ip = succ_ipport.substr(0, succ_ipport.find(':'));
	string succ_port = succ_ipport.substr(succ_ipport.find(':') + 1);
	int succ_port_int = stoi(succ_port);

	string pre_ipport = document["ipport_pre"].GetString();
	string pre_ip = pre_ipport.substr(0, pre_ipport.find(':'));
	string pre_port = pre_ipport.substr(pre_ipport.find(':') + 1);
	int pre_port_int = stoi(pre_port);

	int slave_id = document["id_slave"].GetInt();
	int succ_id = document["id_succ"].GetInt();
	int pre_id = document["id_pre"].GetInt();
	cout << "ip, port and id of all the nodes extracted!" << endl;
	sleep(5);
	int sock_succ = to_connect(succ_ip, succ_port_int);
	if (sock_succ != -1)
	{
		cout << "Connection successfully established with successor node. Conecting to get key,value pairs!" << endl;
		string request_keys = get_keys_from_succ(slave_id, pre_id);
		send(sock_succ, request_keys.c_str(), request_keys.length(), 0);
		cout << "Request to get key,value pairs successfully sent" << endl;
		char succ_response[1024];
		memset(succ_response, 0, sizeof(succ_response));
		recv(sock_succ, succ_response, 1024, 0);
		document.Parse(succ_response);
		if (document.HasParseError())
		{
			cout << "Error while parsing response of successor at the time of connection establishment. Please restart the server!" << endl;
			exit(1);
		}
		else
		{
			assert(document.IsObject());
			assert(document.HasMember("map_status"));
			assert(document["map_status"].IsInt());
			cout << "response from successor is parsed successfully!" << endl;
			cout << "Updating OWN hash table of the node:" << endl;
			if (document["map_status"].GetInt() == 1)
			{
				for (Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr)
				{
					string name1 = itr->name.GetString();
					Value::ConstMemberIterator itr1 = document.FindMember(itr->name);
					if (strcmp(name1.c_str(), "request_type") == 0 || strcmp(name1.c_str(), "map_status") == 0)
					{
						continue;
					}
					else
					{
						cout << "NAME  " << name1 << "========= VALUE " << itr1->value.GetString() << endl;
						own[name1] = itr1->value.GetString();
					}
				}
				cout << "Values successfully added to OWN hashtable!" << endl;
			}
			else
			{
				cout << "Map in successor node was empty! Nothing to add to OWN table." << endl;
			}
		}
	}
	else
	{
		cout << "Unable to establish connection with successor to obtain the key,value pair. Restart the server" << endl;
		exit(1);
	}
	cout << "Closing connection with successor node!" << endl;
	close(sock_succ);

	cout << "Values successfully received and stored from successor!" << endl;
	int sock_pre = to_connect(pre_ip, pre_port_int);
	if (sock_pre != -1)
	{
		cout << "Connection successfully established with predecessor node. Conecting to get key,value pairs!" << endl;
		string request_keys = get_own_from_pred();
		send(sock_pre, request_keys.c_str(), request_keys.length(), 0);
		cout << "Request to get key,value pairs successfully sent" << endl;
		char pre_response[1024];
		memset(pre_response, 0, sizeof(pre_response));
		recv(sock_pre, pre_response, 1024, 0);
		cout << "response of predecessor: " << pre_response << endl;
		Document doc;
		doc.Parse(pre_response);
		if (doc.HasParseError())
		{
			cout << "Error while parsing response of predecessor at the time of connection establishment. Please restart the server!" << endl;
			exit(1);
		}
		else
		{
			assert(doc.IsObject());
			assert(doc.HasMember("request_status"));
			assert(doc["request_status"].IsInt());

			cout << "Predecessor's response string is successfully parsed!" << endl;
			if (doc["request_status"].GetInt() == 1)
			{
				cout << "Will make changes in PREVIOUS table!" << endl;
				for (Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
				{
					string name1 = itr->name.GetString();
					Value::ConstMemberIterator itr1 = doc.FindMember(itr->name);
					string request_type = "request_type", request_status = "request_status";
					// cout<<"printing the iterator: "<<itr1<<endl;
					if (strcmp(name1.c_str(), request_type.c_str()) == 0 || strcmp(name1.c_str(), request_status.c_str()) == 0)
					{
						continue;
					}
					else
					{
						cout << "name inside the function: " << name1 << endl;
						cout << "NAME  " << name1 << "========= VALUE " << itr1->value.GetString() << endl;
						previous[name1] = itr1->value.GetString();
					}
				}
				cout << "Values successfully added to PREVIOUS hashtable!" << endl;
			}
			else
			{
				cout << "OWN table of predecessor is empty! Nothing to append." << endl;
			}
		}
		cout << "Operation successful! Closing the connection." << endl;
		close(sock_pre);
	}
	else
	{
		cout << "Unable to establish connection with successor to obtain the key,value pair. Restart the server" << endl;
		exit(1);
	}
}

int main(int argc, char const *argv[])
{
	int count = 100;
	string temp(argv[1]);
	string slave_ip = temp.substr(0, temp.find(':'));
	string slave_port = temp.substr(temp.find(':') + 1);
	cout << "this is slave ip:port " << slave_ip << ":" << slave_port << endl;


	int cordination_port;

	if (argc < 2)
	{
		cout << "Please enter ip:port of both slave_server and co-ordination server in the same order!";
		exit(1);
	}
	else
	{
		string cordination_ipport(argv[2]);
		cordination_ip = get_ip(cordination_ipport);
		cordination_port = get_port(cordination_ipport);
	}
	cout << "Connecting to co-ordination server on " << cordination_ip << ":" << cordination_port << endl;

	int sock = to_connect(cordination_ip, cordination_port);

	//--------------------------------Registering slave with co-ordination server--------------------
	string reg_slave = register_slaveserver(slave_ip, slave_port);
	cout << "slave registeration> " << reg_slave << endl;
	send(sock, reg_slave.c_str(), 100, 0);
	cout << "Registeration request successfully sent to co-ordination server" << endl;

	char cs_ack[300];
	recv(sock, cs_ack, 300, 0);
	string ackstring(cs_ack);

	//--------------------------------Registering slave with co-ordination server--------------------
	close(sock); //closing the socket sock

	document.Parse(cs_ack);
	if (document.HasParseError())
	{
		cout << "Error while parsing registeration ack from co-ordination server. Restart the server!" << endl;
		exit(1);
	}
	else
	{
		cout << "Slave successfully registered with the server: " << ackstring << endl;
		pthread_t threads;
		int thread_ret;
		struct thread_connection_establish thread_for_connection;
		// cout << 1 << endl;
		thread_for_connection.thread_id = count++;
		// cout << 2 << endl;
		strcpy(thread_for_connection.response_string, cs_ack);
		// cout << 3 << endl;
		// thread_for_connection.doc_thread = document;

		thread_ret = pthread_create(&threads, NULL, get_data, (void *)&thread_for_connection);
		if (thread_ret)
		{
			cout << "Error:unable to create thread," << thread_ret << endl;
		}
		pthread_detach(threads);
	}

	pthread_attr_t attr;
	int server_fd, new_socket;
	struct sockaddr_in slaveAddress;
	int addrlen = sizeof(slaveAddress);
	int rc;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	slaveAddress.sin_family = AF_INET;
	slaveAddress.sin_addr.s_addr = inet_addr(slave_ip.c_str());
	slaveAddress.sin_port = htons(stoi(slave_port));

	if (bind(server_fd, (struct sockaddr *)&slaveAddress, sizeof(slaveAddress)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	int port = BEATPORT;
	pthread_t thread_heartbeat;
	struct hb_thread hb;
	hb.id = -1;
	char *temp1 = new char[256];
	strcpy(temp1, argv[1]);
	hb.ip = temp1;

	if (pthread_create(&thread_heartbeat, NULL, heartbeat, (void *)&hb) < 0)
	{
		perror("Thread error");
	}
	sleep(5);

	int i = 1;

	while (1)
	{

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
		if (listen(server_fd, 3) < 0)
		{
			cout << "inside listen" << endl;
			perror("listen");
			exit(EXIT_FAILURE);
		}

		if ((new_socket = accept(server_fd, (struct sockaddr *)&(slaveAddress), (socklen_t *)&(addrlen))) < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

		td[i].thread_id = i;
		td[i].new_socket = new_socket;

		rc = pthread_create(&threads[i], NULL, Service, (void *)&td[i]);
		if (rc)
		{
			cout << "Error:unable to create thread," << rc << endl;
		}
		pthread_detach(threads[i]);
		i++;
	}
	return 0;
}
