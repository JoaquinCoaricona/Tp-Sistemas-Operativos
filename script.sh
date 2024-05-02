#!/bin/bash

# Compilar Utils
echo "Compilando Utils"
cd utils
make
if [ $? -ne 0 ]; then 
	echo "Error al compilar Utils"
	exit 1
fi
cd ..

# Compilar el primer módulo
echo "Compilando memoria..."
cd memoria 
make
if [ $? -ne 0 ]; then
    echo "Error: No se pudo compilar Memoria"
    exit 1
fi
cd ..

# Compilar el segundo módulo
echo "Compilando cpu..."
cd cpu 
make
if [ $? -ne 0 ]; then
    echo "Error: No se pudo compilar CPU"
    exit 1
fi
cd ..

# Compilar el tercer módulo
echo "Compilando kernel..."
cd kernel
make
if [ $? -ne 0 ]; then
    echo "Error: No se pudo compilar el Kernel..."
    exit 1
fi
cd ..

# Compilar el cuarto módulo
echo "Compilando  Entrada y salida..."
cd entradasalida
make
if [ $? -ne 0 ]; then
    echo "Error: No se pudo compilar el Entrada y Salida"
    exit 1
fi
cd ..

echo "Todos los módulos se han compilado correctamente."


run_module() {
	x-terminal-emulator -e "$1"
}

run_module "memoria/bin/memoria"
run_module "cpu/bin/cpu"
run_module "kernel/bin/kernel/"
run_module "entradasalida/bin/entradasalida"

if [ $? -ne 0 ]; then
    echo "Error: Hubo un problema con la ejecución de los módulos."
    exit 1
fi

echo "Todos los módulos se ejecutaron correctamente."


