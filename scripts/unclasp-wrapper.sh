#!/bin/bash

# NOTE: This script is a hack to be able to run unclasp.
#       The next clasp release will contain support for
#       unsatisfiable core based optimization. At this 
#       time, this script will be removed.

unclasp=unclasp

args=( )

for arg in "$@"
do
    case "$arg" in
        "-q1,2") ;;
        "--opt-hierarch="*) ;;
        "--opt-heu="*) ;;
        "--del-max="*) ;;
        "--restarts="*) ;;
        "--sat-prepro") ;;
        "--local-restarts="*) ;;
        *) 
            args[${#args[@]}]="$arg" 
            ;;

    esac
done

exec "$unclasp" "${args[@]}"
