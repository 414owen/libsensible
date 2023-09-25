# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

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
  };
}
