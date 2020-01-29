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

const int hashlen = 32;
const int saltlen = 16;

PasswdMgr::PasswdMgr(const char *pwd_file):_pwd_file(pwd_file) {

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

   //TODO: Insert your insane code here. This currently assumes that the format is <username>\n<password>\n<salt> Line 113
   int edit  = 0; // variable to determine when user is found and edit the next line
   std::vector<std::string> data;
   std::string user = name; // change username to string for c++ comparison

   // checks if the user is in the database and if it is, checks the password, (0 trust)
   if(!checkPasswd(name, passwd)) // user not found/invalide password
   {
	std::cout << "Invalid username/password to change" << std::endl;
	return false;
   }
   else
   {
	   // read in each line of the password file, adding each to string (file I/O is expensive)
	   std::ifstream oldFile("login.txt", std::ifstream::in);
	   oldFile.open("login.txt");
	   std::string aLine;
	   if(oldFile.is_open())
	   {
		while(std::getline(oldFile, aLine))
		{
			//compare the line to username 
			if(user.compare(aLine) == 0)
			{
				edit = 1; 
			}
			else if(edit == 1) // this is the line that needs to be overwritten
			{
				edit == 0; 
				aLine.assign(passwd); // overwrites the old line (does a '\n' needs to be added?)
			}
			// add the line to data to be written to the file later
			data.emplace_back(aLine); 
		}
		// close oldFile stream for cleaner code 
		oldFile.close();
	   }
	   else //where it goes if the file fails to open
	   {
		perror("Failed to open");
		return false; // return false because failed
	   }
	   // now to write back to file
	   std::ofstream updateFile("login.txt"); // opens up login.txt (all data is erased so need to rewrite everything)
	   if(updateFile.is_open())
	   {
		while(!data.empty())
		{
			updateFile << data.front(); // << std::endl; <- is the new line needed?
			data.erase(data.begin()); // removes the first element
		}
		// all information in data has been transferred. Close the stream
		updateFile.close();
	   }
	   else
	   {
		 perror("Failed to open for writing");
		 return false; // return false because failed
	   }
	   
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
   int counter = 0; 
   // is FileFD a string?
   // currently open a different file from the one passed in TODO: CHANGE TO HIS pwfile(?)
   std::ifstream readFile("login.txt", std::ifstream::in); // opens an ifstream to read in information from file 
   readFile.open("login.txt"); // opens said file 
   std::string aLine;
   if(readFile.is_open())
   {
	while(std::getline(readFile, aLine) && counter > 2)
	{
		//compare the line to username 
		if(name.compare(aLine) == 0)
		{
			// name doesn't need to be read in
			counter++; 
		}
		// read in the line for the hash 
		else if(counter == 1)
		{
			// conversion technique based on one from stackoverflow.com/questions/7664529/converting-a-string-to-uint-t-array-in-c
			//TODO: check it cast to correct type 
			std::vector<uint8_t> tempHVector(aLine.begin(), aLine.end());
			hash.emplace_back(tempHVector[0]);
			counter++;			
		}
		else if(counter == 2)
		{
			//salt.emplace_back(static_cast<uint8_t>(aLine));
			std::vector<uint8_t> tempSVector(aLine.begin(), aLine.end());
			hash.emplace_back(tempSVector[0]);
			counter++;
		}
	}
	// close oldFile stream for cleaner code 
	readFile.close();
   }
	else //where it goes if the file fails to open
	{
		perror("Failed to open");
		return false; // return false because failed
	}

   

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
   int results = 0;

   // Insert your wild code here!
   	   std::ofstream addFile("login.txt", std::ios::app); // opens up login.txt in an append mode 
	   if(addFile.is_open())
	   {
		// add the new information to the file 
		addFile << name << std::endl << hash[0] << std::endl << salt[0] << std::endl;
		// all information in data has been transferred. Close the stream
		addFile.close();
	   }
	   else
	   {
		 perror("Failed to open for writing");
		 return -1; // return -1 for error
	   }

   return sizeof(name) + sizeof(hash[0]) + sizeof(salt[0]); 
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

      if (!readUser(pwfile, uname, hash, salt)) {
         eof = true;
         continue;
      }

      if (!uname.compare(name)) {
         pwfile.closeFD();
         return true;
      }
   }

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
		salt[i] = ((rand() % 93) + 33); // generates ascii characters from dec 33 through 126 avoid other for readability
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
	
   }
   // if not create salt to be added 
   // open up file to append and put new user at the end
   //
}

