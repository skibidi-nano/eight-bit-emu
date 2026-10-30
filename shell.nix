{ pkgs ? import <nixpkgs> {} }:

let
  ncursesStatic = pkgs.ncurses.override { enableStatic = true; };
in
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    clang
    gnumake
    file
  ];

  buildInputs = [
    pkgs.glibc.static
    ncursesStatic
    ncursesStatic.dev 
  ];
}