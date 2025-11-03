echo -e "\033[31m-------------------------\033[0m"
curl -X GET http://localhost:8079
echo -e "\033[33mSuccessfully loaded index\033[0m"
echo -e "\033[31m-------------------------\033[0m"
read -p "Press Enter to continue..."
curl -X GET http://localhost:8079/monkey
echo -e "\033[33m\nNon-existent URI\033[0m"
echo -e "\033[31m-------------------------\033[0m"
read -p "Press Enter to continue..."
curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit" 127.0.0.1:8079/uploads
echo -e "\033[33m\nUploaded POST\033[0m"
echo -e "\033[31m-------------------------\033[0m"
curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit" 127.0.0.1:8078/uploads
echo -e "\033[33m\nUploaded POST with too big of a body\033[0m"
echo -e "\033[31m-------------------------\033[0m"
touch ./www/uploads/sad.html
curl -X DELETE localhost:8079/uploads/sad.html
echo -e "\033[33m\nDeleted successfully with permission\033[0m"
echo -e "\033[31m-------------------------\033[0m"
touch ./www/uploads/happy.html
curl -X DELETE localhost:8078/uploads/happy.html
echo -e "\033[33m\nTried to delete without permission\033[0m"
echo -e "\033[31m-------------------------\033[0m"
read -p "Press Enter to send UNKNOWN request..."
curl -X UNKNOWN localhost:8079
echo -e "\033[33m\nYay! basic Curl tests done\033[0m"
echo -e "\033[31m-------------------------\033[0m"