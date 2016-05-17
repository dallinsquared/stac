# stac
simple dialect of forth implemented in c

## Building
Run `make` to build the default target.  This will generate a system dependent include file, and compile the whole program.
For Debug, run `make debug`.  You can then attach to the process using `sudo gdb -p [proc id of stac]`

## Running
stac can be run by `cat core.stc - | ./stac`
If you have a stac program, you can run it by `cat core.stc [program.stc] | ./stac`
