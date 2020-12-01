Remote debug logger
Auther: Shichao Wang, Geoffrey Wu

This is our assignment 2 for UNX511.
We are using 127.0.0.1:1153 for communicating. Please make sure the address and port is not in use before run this program.

To compile Server, go ./server and run "make all"

This project contains 2 parts - Server and logger. 
The logger should be embedded in other program. 
The Server should be running already before executing the program contains logger.

The server will create a file called "logs" to record everything received from logger, it can be found in ./server directory.
