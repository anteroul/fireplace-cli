{pkgs ? import <nixpkgs> {}}:
pkgs.stdenv.mkDerivation {
  pname = "fireplace";
  version = "1.0";

  src = ./.;

  buildInputs = [pkgs.ncurses];

  buildPhase = ''
    gcc -Wall -Wextra -std=c11 \
    main.c terminfo.c \
    -lncursesw -o fireplace
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp fireplace $out/bin/
  '';
}
