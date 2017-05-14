#! /bin/bash

MAX=10000
for (( i = 0; i<MAX; i++)) ; {
    python3 main.py parameters.ini
}
