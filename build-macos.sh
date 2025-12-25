# For MacOS and OpenBSD/FreeBSD builds
cc -Wall -Wextra -std=c11 \
   main.c terminfo.c \
   -lncurses -o fireplace