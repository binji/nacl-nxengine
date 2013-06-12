NXEngine
--------

NXEngine is a complete open-source clone/rewrite of the masterpiece
jump-and-run platformer Doukutsu Monogatari (also known as Cave Story).

More info and the source for NXEngine here: http://nxengine.sourceforge.net/

More info about Cave Story here: http://www.cavestory.org

NXEngine was written by Caitlin Shaw.
Cave Story was written by Daisuke "Pixel" Amaya.


Setting up the Repo
-------------------

You'll probably need linux (or something linux-like).

    # Find a good place to put nacl-nxengine
    $ git clone git://github.com/binji/nacl-nxengine
    $ cd nacl-nxengine
    $ git submodule init
    $ git submodule update


Building
--------

    $ make

The output is put in $PWD/out. The data needed for the package is in
$PWD/out/package.


Running
-------

    $ export CHROME_PATH=/path/to/chrome
    $ make run-package
