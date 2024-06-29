#!/bin/bash

while true; do leaks --list webserv | grep 'total leaked bytes'; sleep 1; done
