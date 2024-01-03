#!/bin/bash

function menu()
{
    echo Por favor, elige una opción introduciendo su número:
	echo 1 Mostrar código del programa
	echo 2 Compilar programa
	echo 3 Ejecutar programa
	echo 4 Salir
    echo
}

function ejecucion()
{
    if test -f PracticaIntermedia
    then
        echo ¿A cuántos asistentes quieres llamar?
        read asi
        while test $asi -lt 1
            do
                echo La entrada introducida no es correcta, por favor, introduce un número entero mayor que 1
                read asi
            done
        chmod +x ./PracticaIntermedia
        ./PracticaIntermedia $asi
        exit 0
    else
        echo Para poder ejecutar el programa debes compilar el archivo primero
    fi
}

while true
	do
        menu
        read input
        case $input in
            1)
            cat PracticaIntermedia.c
            echo
            echo ----------------------------------------------------------
            echo
            ;;

            2)
            gcc PracticaIntermedia.c -o PracticaIntermedia
            ;;

            3)
            ejecucion
            ;;

            4)
            echo La ejecución del script ha finalizado
            exit 0
            ;;

            *)  
            echo
            echo El número introducido no corresponde con ninguna opción
            echo
            ;;
            esac
    done

