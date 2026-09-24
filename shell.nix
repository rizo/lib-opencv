{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = [
    pkgs.clang
    pkgs.cmake
    pkgs.kraft
    pkgs.bison
    pkgs.flex
  ];
}
