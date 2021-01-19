INCLUDE_DIR=.
CC=gcc
CC_W32=i686-w64-mingw32-gcc
CC_W64=x86_64-w64-mingw32-gcc
CC_FLAGS=-I $(INCLUDE_DIR)
SOURCE_DIR=solver_c
OBJECT_DIR=object
EXE_DIR=dist
COMMON=board_info common compress hash parse memory # debug
COMMON_OBJECTS_LIN=$(addsuffix .o,$(addprefix ${OBJECT_DIR}/,${COMMON}))
COMMON_OBJECTS_WIN32=$(addsuffix .32.o,$(addprefix ${OBJECT_DIR}/,${COMMON}))
COMMON_OBJECTS_WIN64=$(addsuffix .64.o,$(addprefix ${OBJECT_DIR}/,${COMMON}))

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CC_FLAGS)

all: solver solver-win
solver: ${EXE_DIR}/solver
solver-win: ${EXE_DIR}/solver.32.exe ${EXE_DIR}/solver.64.exe
test: ${EXE_DIR}/test

${OBJECT_DIR}/%.32.o: ${SOURCE_DIR}/%.c ${SOURCE_DIR}/%.h
	$(CC_W32) -c -o $@ $< $(CC_FLAGS)
${OBJECT_DIR}/%.64.o: ${SOURCE_DIR}/%.c ${SOURCE_DIR}/%.h
	$(CC_W64) -c -o $@ $< $(CC_FLAGS)
${OBJECT_DIR}/%.o: ${SOURCE_DIR}/%.c ${SOURCE_DIR}/%.h
	$(CC) -c -o $@ $< $(CC_FLAGS)

${EXE_DIR}/test: ${OBJECT_DIR}/test.o ${COMMON_OBJECTS_LIN}
	$(CC) -o $@ $^ $(CC_FLAGS)

${EXE_DIR}/solver: ${OBJECT_DIR}/solver.o ${COMMON_OBJECTS_LIN}
	$(CC) -o $@ $^ $(CC_FLAGS)

${EXE_DIR}/solver.32.exe: ${OBJECT_DIR}/solver.32.o ${COMMON_OBJECTS_WIN32}
	$(CC_W32) -o $@ $^ $(CC_FLAGS)

${EXE_DIR}/solver.64.exe: ${OBJECT_DIR}/solver.64.o ${COMMON_OBJECTS_WIN64}
	$(CC_W64) -o $@ $^ $(CC_FLAGS)

.PHONY: all clean

clean:
	rm -f ${OBJECT_DIR}/*.o *~ core $(INCLUDE_DIR)/*~