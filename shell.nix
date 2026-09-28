{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = [
    # pkgs.clang
    pkgs.gcc
    pkgs.cmake
    pkgs.kraft
    pkgs.bison
    pkgs.flex
  ];
}
