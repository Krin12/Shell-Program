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

	if [[ "$userInput" == "ls" ]]; then
		ls

	elif [[ "$userInput" == "pwd" ]]; then
		pwd

        elif [[ "$userInput" == *"&"* ]]; then

            $userInput &

        else

            inputRedirectRegex="\<(.+)"

            outputRedirectRegex="\>(.+)"

            pipeRegex="(.+)\|(.+)"

            if [[ $userInput =~ $inputRedirectRegex ]]; then

                inputFilePath="${BASH_REMATCH[1]}"

                command="${userInput%%<*$inputFilePath*}"

                $command < "$inputFilePath"

            elif [[ $userInput =~ $outputRedirectRegex ]]; then

                outputFilePath="${BASH_REMATCH[1]}"

                command="${userInput%%>*$outputFilePath*}"

                $command > "$outputFilePath"

            elif [[ $userInput =~ $pipeRegex ]]; then

                firstCommand="${BASH_REMATCH[1]}"

                secondCommand="${BASH_REMATCH[2]}"

                $firstCommand | $secondCommand

            else

                eval "$userInput"
            fi
        fi
    fi
done
