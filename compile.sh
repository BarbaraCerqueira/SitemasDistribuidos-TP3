#!/bin/bash

# Compilation
gcc -pthread -Wall -Wextra -g -o cliente cliente.c
gcc -pthread -Wall -Wextra -g -o coordenador coordenador.c

rm -f log.txt resultado.txt
