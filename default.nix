# default.nix

{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [ 
    pkgs.SDL2
  ];

  shellHook = ''
    hyprctl keyword windowrulev2 "float,class:^(app)$"
    export SDL_VIDEODRIVER=wayland
  '';
}
