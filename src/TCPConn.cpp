#include <stdexcept>
#include <strings.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <iostream>
#include "TCPConn.h"
#include "strfuncts.h"
// added includes 
#include <time.h>
#include <ctime>
#include <chrono>
#include <fstream>
#include <fcntl.h>


// The filename/path of the password file
const char pwdfilename[] = "passwd";

TCPConn::TCPConn():_passwordMan(pwdfilename){ // LogMgr &server_log):_server_log(server_log) {

}


TCPConn::~TCPConn() {

}

/**********************************************************************************************
 * accept - simply calls the acceptFD FileDesc method to accept a connection on a server socket.
 *
 *    Params: server - an open/bound server file descriptor with an available connection
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

bool TCPConn::accept(SocketFD &server) {
   return _connfd.acceptFD(server);
}

/**********************************************************************************************
 * sendText - simply calls the sendText FileDesc method to send a string to this FD
 *
 *    Params:  msg - the string to be sent
 *             size - if we know how much data we should expect to send, this should be populated
 *
 *    Throws: runtime_error for unrecoverable errors
 **********************************************************************************************/

int TCPConn::sendText(const char *msg) {
   return sendText(msg, strlen(msg));
}

int TCPConn::sendText(const char *msg, int size) {
   if (_connfd.writeFD(msg, size) < 0) {
      return -1;  
   }
   return 0;
}

/**********************************************************************************************
 * startAuthentication - Sets the status to request username
 *
 *    Throws: runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPConn::startAuthentication() {

   // Skipping this for now
   _status = s_username;

   _connfd.writeFD("Username: "); 
}

/**********************************************************************************************
 * handleConnection - performs a check of the connection, looking for data on the socket and
 *                    handling it based on the _status, or stage, of the connection
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

void TCPConn::handleConnection() {

   timespec sleeptime;
   sleeptime.tv_sec = 0;
   sleeptime.tv_nsec = 100000000;

   try {
      switch (_status) {
         case s_username:
            getUsername();
            break;

         case s_passwd:
            getPasswd();
            break;
   
         case s_changepwd:
         case s_confirmpwd:
            changePassword();
            break;

         case s_menu:
            getMenuChoice();

            break;

         default:
            throw std::runtime_error("Invalid connection status!");
            break;
      }
   } catch (socket_error &e) {
      std::cout << "Socket error, disconnecting.";
      disconnect();
      return;
   }

   nanosleep(&sleeptime, NULL);
}

/**********************************************************************************************
 * getUsername - called from handleConnection when status is s_username--if it finds user data,
 *               it expects a username and compares it against the password database
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

void TCPConn::getUsername() {
   // Insert your mind-blowing code here
   std::string ipaddr_str;
   getIPAddrStr(ipaddr_str);
   // get user input from client 
   if(getUserInput(_username))
   {
	// compare username input against the data
        if(_passwordMan.checkUser(_username.c_str()))
	{ 
		logEvent("Valid username connecting", ipaddr_str, getUsernameStr());
		// set status to get password b/c valid username
		_status = s_passwd;
	}
	else
	{
		// disconnect due to invalid username
		_connfd.writeFD("Disconnecting due to invalid username\n");
		logEvent("Invalid username attempt", ipaddr_str, getUsernameStr());
      		disconnect();
	}

   }


}

/**********************************************************************************************
 * getPasswd - called from handleConnection when status is s_passwd--if it finds user data,
 *             it assumes it's a password and hashes it, comparing to the database hash. Users
 *             get two tries before they are disconnected
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

void TCPConn::getPasswd() {
   // Insert your astounding code here  
   // variables
   std::string password;
   int myCounter = 0;
   std::string ipaddr_str;
   getIPAddrStr(ipaddr_str);
   // get user input from client for password
   _connfd.writeFD("Password: "); 
   //sleep(10); // sleep for 10 seconds for password
   if(getUserInput(password))
   {
   	while(_pwd_attempts < 2)
   	{
		if(_passwordMan.checkPasswd(_username.c_str(), password.c_str())) // valid username/password combo
		{ 
			// set status to get password b/c valid username
			_status = s_menu;
			// clear password 
			password.clear();
			// escape loop
			break;
		}
		else
		{
			if(_pwd_attempts == 0)	
			{		
				// need to get new password
				_connfd.writeFD("Incorrect Password. Please try again. \nPassword: ");
				while(!getUserInput(password) && myCounter <10)
				{
					// sleep and wait for response
					//sleep(5); //sleep for 5 seconds 
					myCounter++;
				}
			}
		}
		// iterate attempts
		_pwd_attempts++;
	}
	if(_pwd_attempts == 2)
	{
		logEvent("2 Incorrect Password attempts", ipaddr_str, getUsernameStr());
        	_connfd.writeFD("2 incorrect passwords. Now disconnecting\n");
		disconnect();
	}
	else
	{
		logEvent("Valid Login", ipaddr_str, getUsernameStr());
	}		
   }
   // user did not enter password
   else
   {
	logEvent("No password entered", ipaddr_str, getUsernameStr());
	_connfd.writeFD("You did not enter a password. Now disconnecting\n");
	disconnect();

   }
   
}

/**********************************************************************************************
 * changePassword - called from handleConnection when status is s_changepwd or s_confirmpwd--
 *                  if it finds user data, with status s_changepwd, it saves the user-entered
 *                  password. If s_confirmpwd, it checks to ensure the saved password from
 *                  the s_changepwd phase is equal, then saves the new pwd to the database
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

void TCPConn::changePassword() {
   // Insert your amazing code here
   if(_status == s_changepwd)
   {
	// get user input from client for desired new password
	_connfd.writeFD("Please enter desired new password: "); 
	sleep(12); // sleep for 12 seconds for password
	if(!getUserInput(_newpwd))
   	{
		_connfd.writeFD("You did not enter a password. Returning to menu\n");	
		_status = s_menu;
	}
	else
	{
		_status = s_confirmpwd;
	}
   }
   else
   {
	// confirm new password
	std::string confirmer;
	_connfd.writeFD("Please re-type desired new password: "); 
	sleep(12); // sleep for 12 seconds for password
	if(!getUserInput(confirmer))
   	{
		_connfd.writeFD("You did not enter a password. Returning to menu\n");	
	}
	else if(confirmer.compare(_newpwd) == 0)
	{
		// actually write new password
		_passwordMan.changePasswd(_username.c_str(), _newpwd.c_str());
		_connfd.writeFD("Successfully created new password\n");	
	}
	else
	{
		_connfd.writeFD("Passwords did not match. No change made to password.\n");	
	}
	// take user back to menu
	_status = s_menu;
   }
   
}


/**********************************************************************************************
 * getUserInput - Gets user data and includes a buffer to look for a carriage return before it is
 *                considered a complete user input. Performs some post-processing on it, removing
 *                the newlines
 *
 *    Params: cmd - the buffer to store commands - contents left alone if no command found
 *
 *    Returns: true if a carriage return was found and cmd was populated, false otherwise.
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

bool TCPConn::getUserInput(std::string &cmd) {
   std::string readbuf;

   // read the data on the socket
   _connfd.readFD(readbuf);

   // concat the data onto anything we've read before
   _inputbuf += readbuf;

   // If it doesn't have a carriage return, then it's not a command
   int crpos;
   if ((crpos = _inputbuf.find("\n")) == std::string::npos)
      return false;

   cmd = _inputbuf.substr(0, crpos);
   _inputbuf.erase(0, crpos+1);

   // Remove \r if it is there
   clrNewlines(cmd);

   return true;
}

/**********************************************************************************************
 * getMenuChoice - Gets the user's command and interprets it, calling the appropriate function
 *                 if required.
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

void TCPConn::getMenuChoice() {
   if (!_connfd.hasData())
      return;
   std::string cmd;
   if (!getUserInput(cmd))
      return;
   lower(cmd);      

   // Don't be lazy and use my outputs--make your own!
   std::string msg;
   if (cmd.compare("hello") == 0) {
      _connfd.writeFD("Hello there!\n");
   } else if (cmd.compare("menu") == 0) {
      sendMenu();
   } else if (cmd.compare("exit") == 0) {
      _connfd.writeFD("Disconnecting...thank you for coming!\n");
      disconnect();
   } else if (cmd.compare("passwd") == 0) {
      _connfd.writeFD("New Password: ");
      _status = s_changepwd;
   } else if (cmd.compare("1") == 0) {
      msg += "Study Hard and Don't Procrastinate.\n";
      _connfd.writeFD(msg);
   } else if (cmd.compare("2") == 0) {
      _connfd.writeFD("The longest wedding veil was the same length as 63.5 football fields\n"); // source bestlifeonline.com/random-fun-facts
   } else if (cmd.compare("3") == 0) {
      _connfd.writeFD("Have a positive impact on what is in front of you\n");
   } else if (cmd.compare("4") == 0) {
      _connfd.writeFD("How do you make friends with a squirrel\n");
   } else if (cmd.compare("5") == 0) {
      _connfd.writeFD("Climb up a tree and act like a nut\n");
   } else {
      msg = "Unrecognized command: ";
      msg += cmd;
      msg += "\n";
      _connfd.writeFD(msg);
   }

}

/**********************************************************************************************
 * sendMenu - sends the menu to the user via their socket
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/
void TCPConn::sendMenu() {
   std::string menustr;

   // Make this your own!
   menustr += "Available choices: \n";
   menustr += "  1). AFIT Study Tips\n";
   menustr += "  2). Learn a fact \n";
   menustr += "  3). Save the world advice\n";
   menustr += "  4). Get joke\n";
   menustr += "  5). Get joke answer\n";
   menustr += "Other commands: \n";
   menustr += "  Hello - self-explanatory\n";
   menustr += "  Passwd - change your password\n";
   menustr += "  Menu - display this menu\n";
   menustr += "  Exit - disconnect.\n\n";

   _connfd.writeFD(menustr);
}


/**********************************************************************************************
 * disconnect - cleans up the socket as required and closes the FD
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/
void TCPConn::disconnect() {
   _connfd.closeFD();
}


/**********************************************************************************************
 * isConnected - performs a simple check on the socket to see if it is still open 
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/
bool TCPConn::isConnected() {
   return _connfd.isOpen();
}

/**********************************************************************************************
 * getIPAddrStr - gets a string format of the IP address and loads it in buf
 *
 **********************************************************************************************/
void TCPConn::getIPAddrStr(std::string &buf) {
   return _connfd.getIPAddrStr(buf);
}


/*******************************************************************************************
 * eventLogger - Logs specified events in server.log
 *
 *    Params:  event     - the event that triggered the logging
 *             ipAddress - address of whatever caused event
 *             time      - date and timestamp down to the second of the event 
 *
 *    Returns: true if successful, false if not 
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            writing
 *
 *******************************************************************************************/
bool TCPConn::logEvent(std::string event, std::string ipAddress, std::string userName)
{
	// time code based on code found at stackoverflow.com/questions997946/how-to-get-current-time-and-date-in-c
	// get the time 
	auto currTime = std::chrono::system_clock::now();
	std::time_t eventTime = std::chrono::system_clock::to_time_t(currTime);
        // craft string
        std::string entry;
        entry.assign("Event:");
	entry.append(event);
	if(!userName.empty())
	{
		entry.append(" Username: ");
		entry.append(userName); 
	}
	entry.append(" IP Address:");
        entry.append(ipAddress);
	entry.append(" ");
        entry.append(std::ctime(&eventTime));
	entry.append("\n");
    	const char *cEntry = entry.c_str();
        
        // open a file descriptor to write to
	int logger = open("server.log", O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	int amount = write(logger, cEntry, entry.length());
	//int amount2 = write(logger, "\n", 1);
	//std::cout << amount << strerror(errno) << std::endl;
        close(logger);
        return true;

}
