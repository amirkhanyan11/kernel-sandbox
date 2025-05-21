
NAME = test
${CC} = gcc
SRCS = $(wildcard ./*.c) 
OBJS = $(SRCS:.c=.o)

all: ${NAME}

${NAME} : ${OBJS}
	${CC} ${OBJS} -o ${NAME}

.c.o:
	${CC} -c $< -o $@

clean:
	rm -rf ${OBJS} ${NAME}

re: clean all

bre: clean
	$(bear -- make)
