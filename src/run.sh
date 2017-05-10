#! /bin/bash

MAX=3
for (( i = 0; i<MAX; i++)) ; {
    param="$(python3 random_search.py)"
    echo "python3 main.py ${param}"
}