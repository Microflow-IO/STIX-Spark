#!/bin/bash

while true; do 
  timeout 1 nc -u 127.0.0.1 15155 < test-stix.json
  sleep 1
done

