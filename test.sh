#!/bin/bash

while true; do 
  timeout 1 nc -4u 127.0.0.1 12201 < test-stix.json
  sleep 1
done

