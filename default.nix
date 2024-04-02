# default.nix

{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [ 
    pkgs.gnumake
    pkgs.gcc
    pkgs.raylib
  ];
}
