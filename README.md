# brewslave

![workflow status](https://github.com/brewpeople/brewslave/actions/workflows/main.yml/badge.svg)

Brewslave is an Arduino slave application working with
[brewmeister](https://github.com/brewpeople/brewmeister-rs). It is still under
heavy development and comes with NO WARRANTY.


## Build

Initialize and update required library submodules with

    $ git submodule init
    $ git submodule update

Make a copy of the `config.ini.template` and adapt the values accordingly. Then
pass the filename to the `configure` script:

    $ ./configure <config.ini>

and compile and flash with

    $ make && make upload


## Wiring

TBD
