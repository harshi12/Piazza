#include <bits/stdc++.h> 
#define SETS 4
using namespace std; 
  

class LRUCache 
{ 
    // store keys of cache 
    list<int> dq; 
  
    // store references of key in cache 
    unordered_map<int, list<int>::iterator> ma; 
    int csize; //maximum capacity of cache 
  
public: 
    LRUCache(int); 
    void refer(int); 
    void display(); 
}; 
  
LRUCache::LRUCache(int n) 
{ 
    csize = n; 
} 
  
/* Refers key x with in the LRU cache */
void LRUCache::refer(int x) 
{ 
    // not present in cache 
    if (ma.find(x) == ma.end()) 
    { 
        // cache is full 
        if (dq.size() == csize) 
        { 
            //delete least recently used element 
            int last = dq.back(); 
            dq.pop_back(); 
            ma.erase(last); 
        } 
    } 
  
    // present in cache 
    else
        dq.erase(ma[x]); 
  
    // update reference 
    dq.push_front(x); 
    ma[x] = dq.begin(); 
} 

// display contents of cache 
void LRUCache::display() 
{ 
    for (auto it = dq.begin(); it != dq.end(); 
                                        it++) 
        cout << (*it) << " "; 
  
    cout << endl; 
} 


// Driver program to test above functions 
int main() 
{ 
    vector<class LRUCache>cache(SETS);

    LRUCache * cac = new LRUCache(2);

    // LRUCache ca(2); 
   
  
    cac->refer(1); 
    cac->refer(3); 
    cac->refer(1); 
    cac->refer(2); 
    cac->refer(4); 
    cac->refer(5); 
    cache.push_back(*cac);
    
    for (auto it = cache.begin();it != cache.end();it++) 
            it -> display(); 
  
    return 0; 
} 
// This code is contributed by Satish Srinivas 
