#!/bin/sh


pmFiles="mkafka.c proto.h table.c"
for fl in $pmFiles; do
    cp $fl /usr/src/minix/servers/pm
done

# Copy Makefiles
cp header_makefile /usr/src/minix/include/Makefile
cp pm_makefile /usr/src/minix/servers/pm/Makefile

cp callnr.h /usr/src/minix/include/minix
cp mkafkalib.h /usr/src/minix/include
