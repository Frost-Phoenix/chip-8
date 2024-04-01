# default.nix

{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [ 
    pkgs.raylib
  ];
}
