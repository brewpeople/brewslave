# brewslave

Brewslave is an Arduino slave application working with
[brewmeister](https://github.com/matze/brewmeister). It is still under heavy
development and comes with NO WARRANTY.


## Build

Initialize and update required library submodules with

    $ git submodule init
    $ git submodule update

Configure the source tree with

    $ ./configure [--with-ds18b20 <PIN>] <BOARD_TAG>[.<BOARD_SUB>]

and compile and flash with

    $ make && make upload


## Wiring

TBD
