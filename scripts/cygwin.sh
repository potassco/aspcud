#!/bin/bash

args=( )
for i in "${@}"
do
  case "$i" in
    */*)
      args+=( "$(cygpath -w "$i")" )
      ;;
    *)
      args+=( "$i" )
      ;;
  esac
done

exec "$(dirname $0)/$(basename $0 .exe)-exe.exe" "${args[@]}"
