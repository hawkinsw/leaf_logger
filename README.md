## README

### Building:

If you have the GCC plugin developer tools available, you should be
able to build by simply running 

```
$ make
```

To get the GCC plugin developer tools on Ubuntu, load the following
packages using `apt`:

```
apt-get install gcc-x.y-plugin-dev
```

where `x.y` corresponds to the version of GCC installed on your computer.

### Testing:

$ make test

should print

```
leafy, leaf
unleafy, not leaf
main, not leaf
```

if everything worked correctly!

### Running:

When compiling with gcc, use these extra command line flags to use the
plugin:

```
$ gcc test.c -o a.out -fplugin=<path to leaf_logger.so>/leaf_logger.so 
```

You can use an optional second parameter to name the output file:

```
$ gcc test.c -o a.out -fplugin=<path to leaf_logger.so>/leaf_logger.so -fplugin-arg-leaf_logger-output=<output file>
```

If you do not specify the output file, the result will be stored in a file automatically. That output file will be named the same as the main input file with the addition of a .leafs extension. If the input file contains a path (and /s), the path delimiters will be replaced with `_`s.

That should be it!
