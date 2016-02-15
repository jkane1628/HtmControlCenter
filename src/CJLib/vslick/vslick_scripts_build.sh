#!/bin/bash          



function build {
   echo --- BUILD: TARGET=$1 ---
   make $1
   if [ "$?" -ne "0" ]; then
        echo --- !!ERROR!! ---BUILD COMPLETED WITH ERRORS--- !!ERROR!! ---
        exit 1
   else
        echo --- BUILD COMPLETED SUCCESSFULLY ---
   fi
}

function rebuild {
   clean 
   build $1 $2
}

function clean {
   echo --- CLEAN: PRODUCT=$1 ---
   make PRODUCT=$1 clean
   if [ "$?" -ne "0" ]; then
        echo --- !!ERROR!! ---CLEAN COMPLETED WITH ERRORS--- !!ERROR!! ---
        exit 1
   else
        echo --- CLEAN COMPLETED SUCCESSFULLY ---
   fi
}

function execute {
    echo --- EXECUTE: PRODUCT=$1 ---
    ../fw_prototype/buildout/DP2_PROTO/sim/link/simplesim
    if [ "$?" -ne "0" ]; then
        echo --- !!ERROR!! ---EXECUTE COMPLETED WITH ERRORS--- !!ERROR!! ---
        exit 1
    else
        echo --- EXECUTE COMPLETED SUCCESSFULLY ---
    fi
}

if [ "$1" = "build" ]; then
   build $2 $3
fi
if [ "$1" = "rebuild" ]; then
   rebuild $2 $3
fi
if [ "$1" = "execute" ]; then
   execute $2 $3
fi



