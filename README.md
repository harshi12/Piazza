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


** How to find a slave server is down?

-> Slave servers periodically send a message to the cordination server (CS) to inform it that it is alive. When the CS senses that message has not arrived from the slave server side, it gets to know that the particular slave server is down.
We have maintained one thread on the slave server side which makes a UDP connection with the CS. This thread sends heartbeat message to CS for every 5 seconds.
Also, there is a heart beat listener thread on the CS side to receive heartbeat message from different slave servers. 
It maintains two maps:
 - timeout: to maintain count of every message received from a particular slave server. initiallized with 0 for all slave servers and incremented each time a message is received.
 - islive: to maintain whether a particular slave server is alive or not. initiallized with false for all slave servers and made true once a heartbeat message is received.
 There is a second thread in the CS which sleeps and checks if thread is alive or not. It resets the timeout map for all  slave servers to 0. Then it checks whether a slave server with 0 timeout and true alive status exists or not. If any such entry is found, that particular slave server is declared down.
