{pkgs ? import <nixpkgs> {}}:
pkgs.stdenv.mkDerivation {
  pname = "fireplace";
  version = "1.0";

  src = ./.;

  buildInputs = [pkgs.ncurses];

  buildPhase = ''
    gcc -O2 -o fireplace main.c -lncursesw
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp fireplace $out/bin/
  '';
}
