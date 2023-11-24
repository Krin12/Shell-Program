#!/bin/bash
echo "This is SHP"

trap 'echo "Ctrl-Z를 눌러 프로그램을 종료할 수 있습니다."' INT
trap QUIT

stty -echoctl

while true; do
   
    read -p "명령어를 입력하세요 (끝내려면 'exit' 입력): " userInput

   
    if [ "$userInput" == "exit" ]; then
        echo "프로그램을 종료합니다."
        break
    else
        
        echo "입력된 명령어: $userInput"

        
        if [[ "$userInput" == *"&"* ]]; then
            $userInput &
        else
       
            eval "$userInput"
        fi
    fi
done
