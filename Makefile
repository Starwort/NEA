INCLUDE_DIR=.
CC=gcc
CC_W32=i686-w64-mingw32-gcc
CC_W64=x86_64-w64-mingw32-gcc
CC_FLAGS=-I $(INCLUDE_DIR)
SOURCE_DIR=solver_c
OBJECT_DIR=object
EXE_DIR=dist
COMMON=board_info common compress hash parse
COMMON_OBJECTS=$(addsuffix .o,$(addprefix ${OBJECT_DIR}/,${COMMON}))

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CC_FLAGS)

all: solver # solver-win
solver: ${EXE_DIR}/solver
solver-win: ${EXE_DIR}/solver.32.exe ${EXE_DIR}/solver.64.exe
test: ${EXE_DIR}/test

${OBJECT_DIR}/%.o: ${SOURCE_DIR}/%.c ${SOURCE_DIR}/%.h
	$(CC) -c -o $@ $< $(CC_FLAGS)

${EXE_DIR}/test: ${OBJECT_DIR}/test.o ${COMMON_OBJECTS}
	$(CC) -o $@ $^ $(CC_FLAGS)

${EXE_DIR}/solver: ${OBJECT_DIR}/solver.o ${COMMON_OBJECTS}
	$(CC) -o $@ $^ $(CC_FLAGS)

${EXE_DIR}/solver.32.exe: ${OBJECT_DIR}/solver.o ${COMMON_OBJECTS}
	$(CC_W32) -o $@ $^ $(CC_FLAGS)

${EXE_DIR}/solver.64.exe: ${OBJECT_DIR}/solver.o ${COMMON_OBJECTS}
	$(CC_W64) -o $@ $^ $(CC_FLAGS)

.PHONY: all clean

clean:
	rm -f ${OBJECT_DIR}/*.o *~ core $(INCLUDE_DIR)/*~