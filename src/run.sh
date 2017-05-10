#! /bin/bash

MAX=10000
for (( i = 0; i<MAX; i++)) ; {
    param="$(python3 random_search.py)"
    screen -d -m python3 main.py $param
}