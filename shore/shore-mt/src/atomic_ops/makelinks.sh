#!/bin/sh
if test -d ia32; then   # originally test -h
   echo ./asm_linkage/usr/src/uts/intel/ia32 exists
else
   echo ln -s ./asm_linkage/usr/src/uts/intel/ia32
   ln -s ./asm_linkage/usr/src/uts/intel/ia32
fi

if test -d intel; then   # originally test -h
    echo ./asm_linkage/usr/src/uts/intel exists
else
    echo ln -s ./asm_linkage/usr/src/uts/intel
    ln -s ./asm_linkage/usr/src/uts/intel
fi

if test -d sparc; then   # originally test -h
    echo ./asm_linkage/usr/src/uts/sparc exists
else
    echo ln -s ./asm_linkage/usr/src/uts/sparc
    ln -s ./asm_linkage/usr/src/uts/sparc
fi

