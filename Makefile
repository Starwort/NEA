INCLUDE_DIR=.
CC=gcc
CC_W32=i686-w64-mingw32-gcc
CC_W64=x86_64-w64-mingw32-gcc
CC_FLAGS=-I $(INCLUDE_DIR)
SOURCE_DIR=solver_c
OBJECT_DIR=lib
EXE_DIR=dist
COMMON_DEPS=${OBJECT_DIR}/common.h

%.o: %.c $(COMMON_DEPS)
	$(CC) -c -o $@ $< $(CC_FLAGS)

all: ${EXE_DIR}/solver

${OBJECT_DIR}/common.o: ${SOURCE_DIR}common.c
	$(CC) -c -o ${OBJECT_DIR}/common.o ${SOURCE_DIR}common.c $(CC_FLAGS)

${OBJECT_DIR}/solver.o: solver.c
	$(CC) -c -o ${OBJECT_DIR}/solver.o ${SOURCE_DIR}solver.c

${EXE_DIR}/solver: ${OBJECT_DIR}/solver.o ${OBJECT_DIR}/common.o
	$(CC) -o $@ $^ $(CC_FLAGS)

${EXE_DIR}/solver.32.exe: ${OBJECT_DIR}/solver.o ${OBJECT_DIR}/common.o
	$(CC_W32) -o $@ $^ $(CC_FLAGS)

${EXE_DIR}/solver.64.exe: ${OBJECT_DIR}/solver.o ${OBJECT_DIR}/common.o
	$(CC_W64) -o $@ $^ $(CC_FLAGS)

.PHONY: all clean

clean:
	rm -f ${OBJECT_DIR}/*.o *~ core $(INCLUDE_DIR)/*~