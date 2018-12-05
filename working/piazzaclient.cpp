// g++ -g piazzaclient.cpp -o pclient
// ./pclient
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include "common_functions.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <string>
#include <iostream>
#include "strtoken.hpp"

// #define PORT_CS 8080
using namespace std;
using namespace rapidjson;

Document document;

string register_with_coserver()
{
	string mystring = " {  \"request_type\" : \"register_client\" } ";
	return mystring;
}

string send_put_request(string key, string value)
{
	string mystring = " {  \"request_type\" : \"client_put_request\", \"key\" : \"" + key + "\", \"value\" : \"" + value + "\" } ";
	return mystring;
}

string send_get_request(string key)
{
	string mystring = " {  \"request_type\" : \"client_get_request\", \"key\" : \"" + key + "\" } ";
	return mystring;
}

string send_delete_request(string key)
{
	string mystring = " {  \"request_type\" : \"client_delete_request\", \"key\" : \"" + key + "\" } ";
	return mystring;
}

int main(int argc, char const *argv[])
{
	int sock_cs;
	struct sockaddr_in cs_serv_addr;
	string server_ip;
	int server_port;
	//char buffer[1024] = {0};
	if(argc < 1){
		cout<<"Please enter the ip:port of the co-ordination server"<<endl;
		exit(1);
	}
	else{
		string server_ipport(argv[1]);
		server_ip = get_ip(server_ipport);
		server_port = get_port(server_ipport);
	}
	cout<<"Connecting to co-ordination server with ip:port "<<server_ip<<":"<<server_port<<endl;
	//------------------establish connection with the co-ordination server with port number 8080---------------
	if ((sock_cs = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	memset(&cs_serv_addr, '0', sizeof(cs_serv_addr));
	cs_serv_addr.sin_family = AF_INET;
	cs_serv_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
	cs_serv_addr.sin_port = htons(server_port);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, server_ip.c_str(), &cs_serv_addr.sin_addr) <= 0)
	{
		printf("\npc: Invalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock_cs, (struct sockaddr *)&cs_serv_addr, sizeof(cs_serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	cout << "Connection successfully established with co-ordination server: ";
	//------------------establish connection with the co-ordination server with port number 8080---------------

	// ---------------register the client with co-ordination server-----------------
	string string_here = register_with_coserver();
	cout << string_here << endl;
	// cout<<string_here.length()<<endl;

	send(sock_cs, string_here.c_str(), 100, 0);

	char cs_ack[300];
	recv(sock_cs, cs_ack, 300, 0);
	if (document.ParseInsitu(cs_ack).HasParseError())
	{
		cout << "Error while parsing the json string while registeration of client" << endl;
	}
	else if (strcmp(document["request_type"].GetString(), "acknowledge_client_registeration") == 0)
	{
		char temp[200];
		strcpy(temp, document["message"].GetString());
		string output_msg(temp);
		cout << "acknowledge_client_registeration: ";
		cout << output_msg << endl;
	}

	//---------------register the client with co-ordination server-------------------

	while (1)
	{
		cout << "Please select any one choice: " << endl;
		cout << "1. PUT\n";
		cout << "2. GET\n";
		cout << "3. DELETE\n";
		cout << "4. Exit\n";

		int choice;
		string key, value, res_ip;
		vector<string> slave_ipport;
		cin >> choice;
		char response[300];
		memset(response, 0, sizeof(response));
		int flag = 0;
		string command1, command2;
		if (choice == 1)
		{
			cout << "Please enter the key value pair: ";
			cin >> key >> value;
			string put_request = send_put_request(key, value);
			send(sock_cs, put_request.c_str(), 100, 0);
			recv(sock_cs, response, 300, 0);
			flag = 1;
		}
		else if (choice == 2)
		{
			cout << "Please enter the key: ";
			cin >> key;
			string get_request = send_get_request(key);
			send(sock_cs, get_request.c_str(), 100, 0);
			recv(sock_cs, response, 300, 0);
			flag = 1;
		}
		else if (choice == 3)
		{
			cout << "Please enter the key: ";
			cin >> key;
			string del_request = send_delete_request(key);
			send(sock_cs, del_request.c_str(), 100, 0);
			recv(sock_cs, response, 300, 0);
			flag = 1;
		}
		else if (choice == 4)
		{
			cout << "Client will exit now!" << endl;
			exit(1);
		}
		else
		{
			cout << "Please select a valid option!" << endl;
		}

		if (flag)
		{
			cout << "Response from co-ordination server: " << response << endl;
			document.Parse(response);
			if (document.HasParseError())
			{
				cout << "Error while parsing the json response from co-ordination server" << endl;
			}
			else if (strcmp(document["request_type"].GetString(), "error") == 0)
			{
				char temp[200];
				strcpy(temp, document["message"].GetString());
				string output_msg(temp);
				cout << "Response from co-ordination server: ";
				cout << output_msg << endl;
			}
			else if (strcmp(document["request_type"].GetString(), "put_request_ack") == 0)
			{
				char temp[200];
				strcpy(temp, document["message"].GetString());
				string output_msg(temp);
				cout << "response from co-ordination server: ";
				cout << output_msg << endl;
			}
			else if (strcmp(document["request_type"].GetString(), "del_request_ack") == 0)
			{
				char temp[200];
				strcpy(temp, document["message"].GetString());
				string output_msg(temp);
				cout << "response from co-ordination server: ";
				cout << output_msg << endl;
			}
			else if (strcmp(document["request_type"].GetString(), "getreq_response") == 0)
			{
				char temp[200];
				strcpy(temp, document["value"].GetString());
				string output_msg(temp);
				if(output_msg == ""){
					cout<<"response from co-ordination server: VALUE NOT FOUND"<<endl;
				}
				else{
					cout << "response from co-ordination server: ";
					cout << output_msg << endl;
				}
			}
			// else if (strcmp(document["request_type"].GetString(), "req_ack") == 0)
			// {
			// 	char temp[200];
			// 	strcpy(temp, document["value"].GetString());
			// 	string output_msg(temp);
			// 	cout << "response from co-ordination server: ";
			// 	cout << output_msg << endl;
			// }
		}
	}

	return 0;
}
