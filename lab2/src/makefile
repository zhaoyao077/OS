OBJS	=	main.o	my_print.o
main	:	${OBJS}
		gcc -o main ${OBJS}
main.o	:	main.c
		gcc -o main.o -c main.c
my_print.o	:	my_print.asm
		nasm -f elf32 my_print.asm -o my_print.o
clean	:
		-rm main ${OBJS}