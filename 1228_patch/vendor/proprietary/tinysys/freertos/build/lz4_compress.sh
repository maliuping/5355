#!/bin/sh

IMAGE_KERNEL=$1
LZ4_TOOL=$2

${LZ4_TOOL} -l -c ${IMAGE_KERNEL} > ${IMAGE_KERNEL}.lz4

# append file size information
dec_size=0
fsize=$(stat -c "%s" ${IMAGE_KERNEL})
dec_size=$(expr $dec_size + $fsize)
printf "%08x\n" $dec_size |
        sed 's/\(..\)/\1 /g' | {
        read ch0 ch1 ch2 ch3;
        for ch in $ch3 $ch2 $ch1 $ch0; do
                printf `printf '%s%03o' '\\' 0x$ch` >> ${IMAGE_KERNEL}.lz4;
        done;
}

#mv ${IMAGE_KERNEL}.lz4 ${IMAGE_KERNEL}
