{
  description = "Eunice embedded development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs =
    { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          # Zephyr build system
          cmake
          ninja
          python312
          python312Packages.pip

          # Native build utilities
          git
          gnumake
          pkg-config
          ccache

          # Devicetree / configuration tools
          dtc
          gperf

          # Useful utilities
          wget
          curl
          file
          which

          # ARM bare-metal compiler
          gcc-arm-embedded
        ];

        shellHook = ''
          echo "Eunice firmware environment"
          echo "ARM toolchain: $(arm-none-eabi-gcc --version | head -1)"
        '';
      };
    };
}
