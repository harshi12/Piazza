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
#define ll long long

ll sizeoftable;
// ll n = RING_CAPACITY;
bool is_prime(ll x)
{   for(ll i=2;i*i<=x;i++)
        if(x%i==0)
            return 0;
    return 1;
}


void prev_prime(ll n)
{   for(;!is_prime(n);n--);
    sizeoftable = n;
    // n = RING_CAPACITY;
    
}
template <class datatype1,class datatype2>
ll calculate_hash_value(datatype1 key,datatype2 temp = RING_CAPACITY)
{   
    string s = key;
    prev_prime(temp);
    int i,l=s.length();
    unsigned long long enc=0;
    for(i=0;i<l;i++)
    {   enc+=s[i]*99999989;
        enc%=sizeoftable;

    }
    return enc;
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
		printf("\nto connect: Invalid address/ Address not supported \n"); 
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
