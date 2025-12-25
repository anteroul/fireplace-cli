# Requires MinGW / MSYS2
gcc -std=c11 -Wall -Wextra \
    main.c terminfo.c \
    -lpdcurses \
    -o fireplace.exe
