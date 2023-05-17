VERSION -1.0
1) Make server with make SERVER command line
2) Press ./server 
3) Open new terminal and type: telnet localhost 7123
4) Server window will response - that is it for now.


Also there are files for custom messages and "connection" class description
Overall, it creates good architecture for server aplience.

VERSION UPDATE 0.0

1) Client added, here simple one is provided
 a) It connects to the server
 b) Send a request to download track
 c) It waits new incoming messages in stupid "while(true)" loop - (finish it with ˆc)
After execution it creates "New.mp3" file, that it received. 


2) Server and client now can communicate with net_messages in general. 

3) Build client with make CLIENT and server with make SERVER 