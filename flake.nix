{
  description = "Flake shell";
  # inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.nixpkgs.url = "github:gen740/nixpkgs/cxx-import-std";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs =
    { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (
      system: with nixpkgs.legacyPackages.${system}; {
        devShells.default = mkShellNoCC {
          packages = [
            llvmPackages_19.clang-tools
            (llvmPackages_19.libcxxClang.overrideAttrs (oldAttrs: {
              postFixup =
                oldAttrs.postFixup
                + ''
                  ln -sf  ${oldAttrs.passthru.libcxx}/lib/libc++.modules.json $out/resource-root/libc++.modules.json
                  ln -sf  ${oldAttrs.passthru.libcxx}/share $out
                '';
            }))
            (cmake.overrideAttrs (oldAttrs: {
              version = "3.30.2";
              src = oldAttrs.src.overrideAttrs { outputHash = null; };
            }))
            ninja
          ];
        };

        defaultPackage = stdenv.mkDerivation {
          name = "import_std_example";
          src = ./.;
          nativeBuildInputs = [
            llvmPackages_19.clang-tools
            (llvmPackages_19.libcxxClang.overrideAttrs (oldAttrs: {
              postFixup =
                oldAttrs.postFixup
                + ''
                  ln -sf  ${oldAttrs.passthru.libcxx}/lib/libc++.modules.json $out/resource-root/libc++.modules.json
                  ln -sf  ${oldAttrs.passthru.libcxx}/share $out
                '';
            }))
            (cmake.overrideAttrs (oldAttrs: {
              version = "3.30.2";
              src = oldAttrs.src.overrideAttrs { outputHash = null; };
            }))
            ninja
          ];
          installPhase = ''
            mkdir -p $out/bin
            cp main $out/bin
          '';
        };
      }
    );
}
