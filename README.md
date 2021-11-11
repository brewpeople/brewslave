# brewslave

Brewslave is an Arduino slave application working with
[brewmeister](https://github.com/matze/brewmeister). It is still under heavy
development and comes with NO WARRANTY.


## Build

Initialize and update required library submodules with

    $ git submodule init
    $ git submodule update

Configure the source tree with

    $ ./configure [--with-mock-controller] [uno|nano]

and compile and flash with

    $ make && make upload


## Wiring

TBD
