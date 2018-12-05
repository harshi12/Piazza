#include<string>
#include<iostream>
#include<sys/socket.h>
#include<stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <stdlib.h> 
#include<string.h>

#define RING_CAPACITY 16
using namespace std;

// unsigned long calculate_hash_value(int str1,int size) {
// 	//cout<<"string "<<str<<endl;
// 	std::string str;
// 	str=std::to_string(str1);
//     unsigned long hash_value = 5381;
//     int chr;
//     int  i=0;
//     while (chr = str[i++])
//         hash_value = (((hash_value << 5) + hash_value)+chr); 
    	
//     //cout<<"hash before return "<<hash% size<<endl;
//     /* hash * 33 + c */
//     return hash_value % size;

// }

// unsigned long calculate_hash_value(std::string str,int size) {
// 	//cout<<"string "<<str<<endl;
// 	unsigned long hash_value = 5381;
// 	int chr;
// 	int  i=0;
// 	while (chr = str[i++])
// 		hash_value = (((hash_value << 5) + hash_value)+chr); 
		
// 	//cout<<"hash before return "<<hash% size<<endl;
// 	/* hash * 33 + c */
// 	return hash_value % size;

// }

int calculate_hash_value(string key, int temp)
{
    int modval=key.length()%4;
    int n=key.length()-modval;
    string hashedString;
    for(int i=0;i<n;i+=4){
        int num1=(int)key[i];
        int num2=(int)key[i+1];
        int num3=(int)key[i+2];
        int num4=(int)key[i+3];
        int num=(num1+num2+num3+num4)%2;
        hashedString+=to_string(num);
    }
    int num=0;
    for(int i=n;i<key.length();i++){
        num+=key[i];
    }
    if(modval!=0){
        num=num%RING_CAPACITY;
        hashedString+=to_string(num);
    }
    return stoi(hashedString, nullptr, 2);
}

int calculate_hash_value(int str, int temp)
{

	std::string key;
	key=std::to_string(str);
    int modval=key.length()%4;
    int n=key.length()-modval;
    string hashedString;
    for(int i=0;i<n;i+=4){
        int num1=(int)key[i];
        int num2=(int)key[i+1];
        int num3=(int)key[i+2];
        int num4=(int)key[i+3];
        int num=(num1+num2+num3+num4)%2;
        hashedString+=to_string(num);
    }
    int num=0;
    for(int i=n;i<key.length();i++){
        num+=key[i];
    }
    if(modval!=0){
        num=num%RING_CAPACITY;
        hashedString+=to_string(num);
    }
    return stoi(hashedString, nullptr, 2);
}


int to_connect(std::string ip, int port){
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

std::string get_ip(std::string ipport){
	std::string ip = ipport.substr(0,ipport.find(':'));
	return ip;
}

int get_port(std::string ipport){
	std::string temp = ipport.substr(ipport.find(':')+1);
	int port = stoi(temp);
	return port;
}
