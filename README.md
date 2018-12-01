# Piazza
Distributed key value store

Run the files in the following sequence:
    1. cordinationServer.cpp
    2. slaveServer1.cpp
    3. piazzaclient.cpp
Events occurrence sequence:
1. Registration of Slave Servers

2. PUT Request

3. GET request

4. DELETE request

5. Heartbeat Signal from slave to cordination server

6. When one slave is down :
   when will it get discovered that the slave is dead
   => inside the timer() function in cordinationServer.cpp when the slave dies, replicate() function is called.
   => Inside Replicate:
        1. the successor,predecessor and successor of successor is found out using the id of dead slave which is passed to this function, from the BST.
        2. the successor is supposed to copy the content of it's 'previous' hash table to it's 'own' hash table, as the 'previous' hash table is supposed to store now the value of the 'own' table of the predecessor. All the values lieng between predecessor and the successor of the dead slave, will now move to successor of dead slave (so we also copied the previous of dead slave to own of successor).
        3. The successor is then supposed to send the values of it's updated 'own' table to it's successor i.e successor of successor of the dead slave.
