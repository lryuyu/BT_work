{ pkgs, config, inputs, ... }:

{
    env.GREET = "devenv";
    packages = [
        pkgs.git
        pkgs.openssl_3
    ];
    languages.rust.enable = true;
    languages.c.enable = true;
}
