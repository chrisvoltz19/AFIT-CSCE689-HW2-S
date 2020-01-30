#Author: Christopher Voltz
# Purpose: This file will create misc files for the setup for HW2

# delete and then setup whitelist
test -f "src/whitelist" && { echo "Removing old whitelist"; rm -f src/whitelist ; }
test -f "src/whitelist" || { echo "Making src/whitelist"; touch src/whitelist ; }
echo "127.0.0.1" >> src/whitelist

# delete and then setup whitelist
test -f "whitelist" && { echo "Removing old whitelist"; rm -f whitelist ; }
test -f "whitelist" || { echo "Making whitelist"; touch whitelist ; }
echo "127.0.0.1" >> whitelist


# delete old login info and set up login.txt
test -f "src/passwd" && { echo "Removing old login info"; rm -f src/passwd ; }
test -f "src/passwd" || { echo "Making src/passwd"; touch src/passwd ; }

# delete old log and add new one 
test -f "src/server.log" && { echo "Removing old server log"; rm -f src/server.log ; }
test -f "src/server.log" || { echo "Making src/server.log" ; touch src/server.log ; }
echo "OFFICIAL TCP SERVER LOG" >> src/server.log
