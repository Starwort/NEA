# add some overridable macros for setting additional args
ADDITIONAL_FLAGS=
CFLAGS=
OPTIMISE_LEVEL=3
WARN_LEVEL=error
WARN_SETTINGS=
ifeq ($(WARN_LEVEL),all)
    WARN_SETTINGS=-Wall -Wextra -pedantic
else ifeq ($(WARN_LEVEL),error)
    WARN_SETTINGS=-Wall -Wextra -Werror -pedantic-errors
endif
DEBUG_METHOD=asan
DEBUG_LIB_FLAG=
ifeq ($(DEBUG_METHOD),asan)
    DEBUG_LIB_FLAG=-fsanitize=address
else ifeq ($(DEBUG_METHOD),efence)
    DEBUG_LIB_FLAG=-lefence
endif
ifdef DEBUG
    DEBUG_FLAGS=-g $(DEBUG_LIB_FLAG) -D DEBUG
    all: solver
else
    DEBUG_FLAGS=
    all: solver solver-win
endif

INCLUDE_DIR=.
CC=gcc
CC_W32=i686-w64-mingw32-gcc
CC_W64=x86_64-w64-mingw32-gcc
CC_FLAGS=-I $(INCLUDE_DIR) -O$(OPTIMISE_LEVEL) -flto $(ADDITIONAL_FLAGS) $(CFLAGS) $(WARN_SETTINGS) $(DEBUG_FLAGS)
SOURCE_DIR=solver_c
OBJECT_DIR=object
EXE_DIR=dist
COMMON=board_info common compress hash parse memory timer
ifdef DEBUG
    COMMON += debug
endif
COMMON_OBJECTS_LIN=$(addsuffix .o,$(addprefix ${OBJECT_DIR}/,${COMMON}))
COMMON_OBJECTS_WIN32=$(addsuffix .32.o,$(addprefix ${OBJECT_DIR}/,${COMMON}))
COMMON_OBJECTS_WIN64=$(addsuffix .64.o,$(addprefix ${OBJECT_DIR}/,${COMMON}))

%.o: %.c %.h solver_c/types.h
	$(CC) -c -o $@ $< $(CC_FLAGS)

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
	rm -f ${OBJECT_DIR}/*.o *~ core $(INCLUDE_DIR)/*~ ${EXE_DIR}/*