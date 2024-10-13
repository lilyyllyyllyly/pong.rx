# pong.rx
This is a simple recreation of the game Pong in the [REXX programming language](https://en.wikipedia.org/wiki/Rexx). <br>
I just randomly felt like making something in this language when I first heard of it a few hours ago, and this is the result!

The left paddle moves with W/S while the right paddle moves with I/K.

This is absolutely terrible because I just wanted to be done with it as fast as possible, but it works mostly i think?

## Building & Running
This project depends on [raylib](https://github.com/raysan5/raylib) for graphics and [regina](https://regina-rexx.sourceforge.io/) as the REXX interpreter.
Make sure you have those installed before proceeding. You should have a `regina` binary in your PATH. It's very unlikely this would work on other REXX interpreters.

First things first, clone the repo and enter the directory:
```bash
$ git clone https://github.com/lilyyllyyllyly/pong.rx
$ cd pong.rx
```
Then build the C shared library (raylib bindings for REXX) with make:
```bash
$ make
```
Finally, you can run it:
```bash
$ LD_LIBRARY_PATH="$LD_LIBRARY_PATH":"./" regina pong.rx
```
You need to set the LD_LIBRARY_PATH so the linker will be able to find the shared library that provides the raylib bindings.

### NixOS
If you are on NixOS like I am, there's a few extra things. First of all, this repo includes a very basic shell.nix with raylib and regina
that you can use by running `nix-shell`. I'd probably need to do a lot more to do things the proper "nixy" way, but I have no idea what I'm
doing when it comes to nix still so this is the best you'll get for now :P

**More importantly,** if you try running the program in the manner described in the section above, raylib might complain about failing to load libwayland-client.
I was able to run it under NixOS by first running the following command:
```bash
$ nix-build '<nixpkgs>' -A wayland
```
And then running the (very simple one liner) provided run.sh script:
```bash
$ ./run.sh
```
This just adds `result/lib` created by the nix-build command to LD_LIBRARY_PATH along with the current directory before running.
I don't really understand what sort of black magic nix-build is doing, I just got this from [this thread](https://discourse.nixos.org/t/raylib-games-failed-to-load-libwayland-client/45722)
and it works, maybe it will help you too in case you run into the same problem.
