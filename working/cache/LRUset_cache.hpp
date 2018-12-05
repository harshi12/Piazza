#include <bits/stdc++.h>
#define SETS 8
#define WAYS 2
using namespace std;

class LRUCache
{
    // store keys of cache
    list<pair<string, string>> dq;

    // store references of key in cache
    unordered_map<string, list<pair<string, string>>::iterator> ma;

  public:
    void refer(string, string);
    void display();
    string getlist(string key);
    void dellist(string key);
};

/* Refers key x with in the LRU cache */
void LRUCache::refer(string key, string value = "none")
{

    auto it = ma.find(key);
    if (it == ma.end())
    {
        // cache is full
        if (dq.size() == WAYS)
        {
            //delete least recently used element
            auto last = dq.back();
            dq.pop_back();
            ma.erase(last.first);
        }
    }

    // present in cache
    else
    {
        auto del = ma[key];
        dq.erase(del);
    }
    // update reference
    dq.push_front(make_pair(key, value));
    ma[key] = dq.begin();
}

// display contents of cache
void LRUCache::display()
{
    for (auto it = dq.begin(); it != dq.end();
         it++)
        cout << "key: " << (it->first) << " value: " << (it->second) << " ";

    cout << endl;
}

string LRUCache::getlist(string key)
{
    string val;
    auto it = ma.find(key);
    if (it != ma.end())
    {
        auto valptr = ma[key];
        val = valptr->second;
    }
    else
        val = "none";
    return val;
}

void LRUCache::dellist(string key)
{
    if (ma.find(key) != ma.end())
    {
        auto del = ma[key];
        ma.erase(del->first);
        dq.erase(del);
    }
}

vector<LRUCache> cache(SETS);

void initialise()
{
    for (int i = 0; i < SETS; i++)
    {
        LRUCache *cac = new LRUCache;
        cache[i] = *cac;
    }
}

void putInSet(string key, string value)
{
    int index = calculate_hash_value(key, SETS);
    cache[index].refer(key, value);
}

string getValue(string key)
{
    int index = calculate_hash_value(key, SETS);
    return cache[index].getlist(key);
}

void deleteKey(string key)
{
    int index = calculate_hash_value(key, SETS);
    cache[index].dellist(key);
}

// // Driver program to test above functions
// int main()
// {

//     initialise();
//     putInSet("17","55");
//     putInSet("14","342");
//     putInSet("8","2");
//     putInSet("15","2432");
//     putInSet("34","25");
//     putInSet("27","23455");
//     putInSet("44","2342342");
//     putInSet("85","42");
//     putInSet("25","132");
//     putInSet("16","2321");
//     putInSet("24","2");

//     cout<<"VALUE: "<<getValue("5")<<endl;
//     deleteKey("17");
//     cout<<"VALUE: "<<getValue("17")<<endl;

//     for (auto it = cache.begin();it != cache.end();it++)
//             it -> display();

//     return 0;
// }
