#Author: Christopher Voltz
# Purpose: This file will create misc files for the setup for HW2

# delete and then setup whitelist
test -f "src/whitelist.txt" && { echo "Removing old whitelist"; rm -f src/whitelist.txt ; }
test -f "src/whitelist.txt" || { echo "Making src/whitelist.txt"; touch src/whitelist.txt ; }
echo "127.0.0.1" >> src/whitelist.txt

# delete old login info and set up login.txt
test -f "src/login.txt" && { echo "Removing old login info"; rm -f src/login.txt ; }
test -f "src/login.txt" || { echo "Making src/login.txt"; touch src/login.txt ; }
echo "admin" >> src/login.txt
echo "<enter hashed password here later" >> src/login.txt
echo "<enter a salt here>" >> src/login.txt

