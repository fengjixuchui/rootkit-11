CC=cc
RSHELL_SRC=rshell_target.c
RSHELL_OUT=rshell_target
KEYLOG_SRC=keylog_target.c
KEYLOG_OUT=keylog_target

rshell: ${RSHELL_SRC}
	${CC} -o ${RSHELL_OUT} ${RSHELL_SRC}
keylog: ${KEYLOG_SRC}
	${CC} -o ${KEYLOG_OUT} ${KEYLOG_SRC}
clean: 
	rm -f ${KEYLOG_OUT} ${RSHELL_OUT}
