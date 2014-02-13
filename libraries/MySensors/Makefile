#############################################################################
#
# Compiles Mysensors Gateway for Raspberry Pi. 
#
# make
#
# The valiable RF24H below should point to the directory for the RF24 header
# files
#
# License: GPL (General Public License)
#
#

CC=g++
CCFLAGS=-Wall -Ofast -mfpu=vfp -DDEBUG -DRPI -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s

# define all programs
PROGRAMS = Gateway Relay Sensor
SOURCES = ${PROGRAMS:=.cpp}
OBJS = ${PROGRAMS:=.o}
DEPS = ${PROGRAMS:=.h}
RF24H=../../Raspberry/librf24

all: ${OBJS} 

%.o: %.cpp ${DEPS}
	${CC} -c -g -o $@ $< ${CCFLAGS} -I${RF24H}

clean:
	rm -rf $(PROGRAMS)

