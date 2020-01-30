#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <strings.h>
#include <vector>
#include <iostream>
#include <memory>
#include <sstream>
#include "TCPServer.h"
// added includes
#include <fstream>


TCPServer::TCPServer(){ // :_server_log("server.log", 0) {
}


TCPServer::~TCPServer() {

}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port) {

   struct sockaddr_in servaddr;

   // _server_log.writeLog("Server started.");
   _log_conn->logEvent("Server started", "127.0.0.1");

   // Set the socket to nonblocking
   _sockfd.setNonBlocking();

   // Load the socket information to prep for binding
   _sockfd.bindFD(ip_addr, port);
 
}

/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::listenSvr() {

   bool online = true;
   timespec sleeptime;
   sleeptime.tv_sec = 0;
   sleeptime.tv_nsec = 100000000;
   int num_read = 0;

   // Start the server socket listening
   _sockfd.listenFD(5);

    
   while (online) {
      struct sockaddr_in cliaddr;
      socklen_t len = sizeof(cliaddr);

      if (_sockfd.hasData()) {
         TCPConn *new_conn = new TCPConn();
         if (!new_conn->accept(_sockfd)) {
            // _server_log.strerrLog("Data received on socket but failed to accept.");
	    _log_conn->logEvent("Data received on socket but failed to accept.", "127.0.0.1");
            continue;
         }
         std::cout << "***Got a connection***\n";

         _connlist.push_back(std::unique_ptr<TCPConn>(new_conn));

         // Get their IP Address string to use in logging
         std::string ipaddr_str;
         new_conn->getIPAddrStr(ipaddr_str);
	 if(!checkWhitelist(ipaddr_str))
	 {
		new_conn->disconnect();
	 }
	 else
	 {
         	new_conn->sendText("Welcome to the CSCE 689 Server!\n");

         	// Change this later
         	new_conn->startAuthentication();
	 }
      }

      // Loop through our connections, handling them
      std::list<std::unique_ptr<TCPConn>>::iterator tptr = _connlist.begin();
      while (tptr != _connlist.end())
      {
         // If the user lost connection
         if (!(*tptr)->isConnected()) {
            // Log it
	    std::string IP;
	    (*tptr)->getIPAddrStr(IP);
	    _log_conn->logEvent("Client Disconnected", IP, (*tptr)->getUsernameStr());

            // Remove them from the connect list
            tptr = _connlist.erase(tptr);
            std::cout << "Connection disconnected.\n";
            continue;
         }

         // Process any user inputs
         (*tptr)->handleConnection();

         // Increment our iterator
         tptr++;
      }

      // So we're not chewing up CPU cycles unnecessarily
      nanosleep(&sleeptime, NULL);
   } 


   
}

/**********************************************************************************************
 * checkWhiteList - Check connected IP address to see if it is on whitelist.
 * 
 *	Returns: true if on the whitelist and false if not
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

bool TCPServer::checkWhitelist(std::string connIP) 
{
	// create the file if it does not exist 
   	int check = open("whitelist", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        close(check);
	// open an ifstream to read in
	std::ifstream wFile("whitelist", std::ifstream::in);
	wFile.open("whitelist");
	std::string IPChecker;
	if(wFile.is_open())
	{
		std::cout << connIP << std::endl;
		// read in line by line and compare
		while(std::getline(wFile, IPChecker))//TODO: error is something to do with this line 
		{
			std::cout << "Yay: " << IPChecker << std::endl;
			//std::cout << connIP.compare(IPChecker) << std::endl << std::endl;
			if(connIP.compare(IPChecker) == 0)
			{
				_log_conn->logEvent("Client connection on whitelist", connIP);
				return true;
			}
		}
		std::cout << IPChecker << std::endl;
		_log_conn->logEvent("Client connection not on whitelist", connIP);
		wFile.close();
		return true; //debugging
		//return false; // not on list	
   	}
}


/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {

   _sockfd.closeFD();
}


