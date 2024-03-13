# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

{
  description = "Basic C development flake";

  outputs = { self, nixpkgs }:
    let
      eachSystem = f: builtins.mapAttrs f nixpkgs.legacyPackages;
    in
  {
    devShells = eachSystem (system: pkgs: {
      default = pkgs.mkShell {
        packages = [
          pkgs.stdenv
          pkgs.clang-tools
          pkgs.clang-tools.clang
          pkgs.cmake
          pkgs.gdb
          pkgs.valgrind
        ];
      };
    });

    packages = eachSystem (system: pkgs: rec {
      libsensible = pkgs.stdenv.mkDerivation {
        name = "libsensible";
        version = "0.1";

        src = ./.;

        nativeBuildInputs = [
          pkgs.cmake
        ];

        meta = with pkgs.lib; {
          homepage = "https://github.com/414owen/libsensible";
          description = "A collection of libraries for making C programming enjoyable";
          licencse = licenses.bsd3;
          platforms = with platforms; linux ++ darwin;
          maintainers = [ maintainers."414owen" ];    
        };
      };

      default = libsensible;
    });
  };
}
