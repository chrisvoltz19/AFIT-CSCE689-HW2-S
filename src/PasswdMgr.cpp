#include <argon2.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <list>
#include "PasswdMgr.h"
#include "FileDesc.h"
#include "strfuncts.h"
// My includes
#include <fstream>
#include <fcntl.h>

const int hashlen = 32;
const int saltlen = 16;

PasswdMgr::PasswdMgr(const char *pwd_file):_pwd_file(pwd_file) {
   // create file if not already there
   int myFile = open(pwd_file, O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
   close(myFile);
}


PasswdMgr::~PasswdMgr() {

}

/*******************************************************************************************
 * checkUser - Checks the password file to see if the given user is listed
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            reading
 *******************************************************************************************/

bool PasswdMgr::checkUser(const char *name) {
   std::vector<uint8_t> passwd, salt;

   bool result = findUser(name, passwd, salt);
  
   //std::cout << "Checking user" << std::endl; 

   return result;
     
}

/*******************************************************************************************
 * checkPasswd - Checks the password for a given user to see if it matches the password
 *               in the passwd file
 *
 *    Params:  name - username string to check (case insensitive)
 *             passwd - password string to hash and compare (case sensitive)
 *    
 *    Returns: true if correct password was given, false otherwise
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            reading
 *******************************************************************************************/

bool PasswdMgr::checkPasswd(const char *name, const char *passwd) {
   std::vector<uint8_t> userhash; // hash from the password file
   std::vector<uint8_t> passhash; // hash derived from the parameter passwd
   std::vector<uint8_t> salt;

   // Check if the user exists and get the passwd string
   if (!findUser(name, userhash, salt))
      return false;

   hashArgon2(passhash, salt, passwd, &salt);

   if (userhash == passhash)
      return true;

   return false;
}

/*******************************************************************************************
 * changePasswd - Changes the password for the given user to the password string given
 *
 *    Params:  name - username string to change (case insensitive)
 *             passwd - the new password (case sensitive)
 *
 *    Returns: true if successful, false if the user was not found
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            writing
 *
 *******************************************************************************************/

bool PasswdMgr::changePasswd(const char *name, const char *passwd) {

   //TODO: Insert your insane code here.
   bool changed = false;
   uint8_t hash[hashlen]; // to hold the hash
   uint8_t salt[saltlen]; // hold the salt
   

   // checks if the user is in the database and if it is, checks the password, (0 trust)
   if(!checkUser(name)) // user not found
   {
	std::cout << "Invalid username/password to change" << std::endl;
	return false;
   }
   else
   {
	// open password file to read in things
	FileFD pwfile(_pwd_file.c_str());
	if (!pwfile.openFile(FileFD::readfd))
	{
     		throw pwfile_error("Could not open passwd file for reading");
	}
	// open a temp value to write to
	int check = open("p", O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        close(check);
	FileFD wfile("tmppasswd");
	if (!wfile.openFile(FileFD::writefd))
	{
     		throw pwfile_error("Could not open passwd file for writing");
	}	
	bool eof = false;
        while (!eof) 
	{
      		std::string uname;

     		if (!readUser(pwfile, uname, hash, salt)) 
		{
         		eof = true;
         		continue;
      		}
      		if (uname.compare(name)) // found the entry to change
		{
			hashArgon2(hash, salt, passwd, &salt);
         		changed = true;
      		}
		// write information to temp file 
		writeUser(wfile, uname, hash, salt);
		
   	}
	// rename temp file to original file
	int re = rename("tmppasswd", _pwd_file.c_str());


	   
	   // when username is found, read in the next line (password line) and edit it to new password
	   // read through the rest of the file and add it to string
	   // put string back in file 
	   //
   }
   
   return true;
}

/*****************************************************************************************************
 * readUser - Taking in an opened File Descriptor of the password file, reads in a user entry and
 *            loads the passed in variables
 *
 *    Params:  pwfile - FileDesc of password file already opened for reading
 *             name - std string to store the name read in
 *             hash, salt - vectors to store the read-in hash and salt respectively
 *
 *    Returns: true if a new entry was read, false if eof reached 
 * 
 *    Throws: pwfile_error exception if the file appeared corrupted
 *
 *****************************************************************************************************/

bool PasswdMgr::readUser(FileFD &pwfile, std::string &name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt)
{
   // Insert your perfect code here!
   // take first line and read it into the &name
   ssize_t nSize = pwfile.readStr(name);
   if(nSize == 0)
   {
	return false;
   }
   // take the next line and read 32 bytes for the hash
   int hSize = pwfile.readBytes(hash, hashlen);
   // continue reading that line for the 16 bytes of salt
   int saSize = pwfile.readBytes(salt, saltlen);
   
   return true;	 

}

/*****************************************************************************************************
 * writeUser - Taking in an opened File Descriptor of the password file, writes a user entry to disk
 *
 *    Params:  pwfile - FileDesc of password file already opened for writing
 *             name - std string of the name 
 *             hash, salt - vectors of the hash and salt to write to disk
 *
 *    Returns: bytes written
 *
 *    Throws: pwfile_error exception if the writes fail
 *
 *****************************************************************************************************/

int PasswdMgr::writeUser(FileFD &pwfile, std::string &name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt)
{
   // Insert your wild code here!
   int nResult = pwfile.writeFD(name);
   nResult = pwfile.writeFD("\n"); // add new line after username
   if(nResult < 0)
   {
	 perror("Failed writing name");
	 return -1; // return -1 for error
   }
   int hResult = pwfile.writeBytes(hash); // write the hash
   if(hResult < 0)
   {
	 perror("Failed writing hash");
	 return -1; // return -1 for error
   }
   int sResult = pwfile.writeBytes(salt); // same line right next to hash
   if(sResult < 0)
   {
	 perror("Failed writing salt");
	 return -1; // return -1 for error
   }
   int fResult = pwfile.writeFD("\n"); // go to next line

   return sizeof(name) + sizeof(hash) + sizeof(salt) + 1; 
}

/*****************************************************************************************************
 * findUser - Reads in the password file, finding the user (if they exist) and populating the two
 *            passed in vectors with their hash and salt
 *
 *    Params:  name - the username to search for
 *             hash - vector to store the user's password hash
 *             salt - vector to store the user's salt string
 *
 *    Returns: true if found, false if not
 *
 *    Throws: pwfile_error exception if the pwfile could not be opened for reading
 *
 *****************************************************************************************************/

bool PasswdMgr::findUser(const char *name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt) {

   FileFD pwfile(_pwd_file.c_str());

   // You may need to change this code for your specific implementation

   if (!pwfile.openFile(FileFD::readfd))
      throw pwfile_error("Could not open passwd file for reading");

   // Password file should be in the format username\n{32 byte hash}{16 byte salt}\n
   bool eof = false;
   while (!eof) {
      std::string uname;

      //std::cout << "Finding user" << std::endl; 

      if (!readUser(pwfile, uname, hash, salt)) {
         eof = true;
         continue;
      }

      //std::cout << "User searched: " << uname << std::endl;

      if (!uname.compare(name)) {
         pwfile.closeFD();
         return true;
      }
   }

   //std::cout << "Found user: " << name << std::endl; 

   hash.clear();
   salt.clear();
   pwfile.closeFD();
   return false;
}


/*****************************************************************************************************
 * hashArgon2 - Performs a hash on the password using the Argon2 library. Implementation algorithm
 *              taken from the http://github.com/P-H-C/phc-winner-argon2 example. 
 *
 *    Params:  dest - the std string object to store the hash
 *             passwd - the password to be hashed
 *
 *    Throws: runtime_error if the salt passed in is not the right size
 *****************************************************************************************************/
void PasswdMgr::hashArgon2(std::vector<uint8_t> &ret_hash, std::vector<uint8_t> &ret_salt, 
                           const char *in_passwd, std::vector<uint8_t> *in_salt) {
   // TODO:Hash those passwords!!!!
   // create variables
   uint8_t hash[hashlen]; // to hold the hash
   uint8_t salt[saltlen]; // hold the salt
   srand(time(0)); // seed the time for randomness 

   // check to see if the user gave their own salt or if need to generate one 
   if(in_salt != NULL)
   {   	
	if(in_salt->size() != saltlen)
   	{
   		throw std::runtime_error("INVALID: The provided salt was not the correct length for a salt.\n");
	}
   	// populate salt with passed in salt
	for(int i = 0; i < saltlen; i++)
	{
		salt[i] = (*in_salt)[i];
	}
   }
   else // generate a salt if no salt is passed in
   {
 	for(int i = 0; i < saltlen; i++)
	{
		salt[i] = ((rand() % 93) + 33); // generates ascii characters from dec 33 through 126; avoids others for readability
	}
   }

   // referenced argon2 documentation for this part 
   uint32_t t_cost = 2;       // 1-pass computation
   uint32_t m_cost = (1<<16); // 64 mebibytes memory usage
   uint32_t parallelism = 1;  // number of threads and lanes 

   // hash using argon2 and put hash in ret_hash and salt in ret_salt
   argon2i_hash_raw(t_cost, m_cost, parallelism, in_passwd, strlen(in_passwd), salt, saltlen, hash, hashlen); 

   ret_hash.clear(); // clear out the return hash in case something was in there
   ret_salt.clear(); // clear out the return salt in case something was in there
   ret_hash.reserve(hashlen); // reserve the hashlen amount of space for the vector (ensure it is together)
   ret_salt.reserve(saltlen); // reserve the saltlen amount of space for the vector
   
   for(int i = 0; i < hashlen; i++)
   {
     	ret_hash.emplace_back(hash[i]); 
   }
   for(int i = 0; i < saltlen; i++)
   {
   	ret_salt.emplace_back(hash[i]); 
   }
}

/****************************************************************************************************
 * addUser - First, confirms the user doesn't exist. If not found, then adds the new user with a new
 *           password and salt
 *
 *    Throws: pwfile_error if issues editing the password file
 ****************************************************************************************************/

void PasswdMgr::addUser(const char *name, const char *passwd) {
   // Add those users!
   // check file and see if user is already there
   if (!checkUser(name))
   {
	// variables
   	FileFD pwfile(_pwd_file.c_str());

	if (!pwfile.openFile(FileFD::appendfd))
	{
     		throw pwfile_error("Could not open passwd file for writing");
	}
        std::string newName(name); 
   	std::vector<uint8_t> passhash; // hash derived from the parameter passwd
   	std::vector<uint8_t> salt;
        // populate hash and salt
   	hashArgon2(passhash, salt, passwd);
        // write new user to file
	writeUser(pwfile, newName, passhash, salt);
        
   }
   else
   {
     	throw pwfile_error("User already exists");
   }
   // if not create salt to be added 
   // open up file to append and put new user at the end
   //
}

