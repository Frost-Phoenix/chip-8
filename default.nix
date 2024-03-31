# default.nix

{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [ 
    pkgs.SDL2
  ];

  shellHook = ''
    hyprctl keyword windowrulev2 "float,class:^(chip-8)$"
    export SDL_VIDEODRIVER=wayland
  '';
}
