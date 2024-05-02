#!/bin/bash

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

#Correr primer modulo
echo "Corriendo memoria"
cd memoria/bin
./memoria
if [ $? -ne 0 ]; then
	echo "Error al ejecutar memoria"
	exit 1
fi 
sleep 1
cd ../../

# Correr CPU
echo "Corriendo CPU"
cd cpu/bin
./cpu
if [ $? -ne 0 ]; then
	echo "Error al ejecutar cpu"
	exit 1
fi

sleep 1 
cd ../../

# Correr Kernel
echo "Corriendo Kernel"
cd kernel/bin
./kernel

if [ $? -ne 0 ]; then
	echo "Error al ejecutar Kernel"
	exit 1
fi

sleep 1


# Correr Entrada y Salida
echo "Corriendo Entrada y Salida"
cd entradasalida/bin
./entradasalida

if [ $? -ne 0 ]; then
	echo "Error al ejecutar Entrada y Salida"
	exit 1
fi
cd ../../


