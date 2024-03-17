# Unix Shell Clone

This project is a simple clone of a Unix shell, designed to demonstrate the basic functionalities of command interpretation, process spawning, and execution control. It is implemented in C and can be compiled using GCC or any standard C compiler.


## Commands

Once you have compiled and ran the shell, you can use the following example commands:
* A blank line.
* `cd <dir>` Note: You can specify a program by either an absolute path, a relative path, or base name only. Thus, `cd` is not the only way to locate programs.
* `exit`
* `/usr/bin/ls -a -l`
* `cat shell.c | grep main | less`
* `cat < input.txt`
* `cat > output.txt`
* `cat >> output.txt`
* `cat < input.txt > output.txt`
* `cat < input.txt >> output.txt`
* `cat > output.txt < input.txt`
* `cat >> output.txt < input.txt`
* `cat < input.txt | cat > output.txt`
* `cat < input.txt | cat | cat >> output.txt`
* etc.

Ctrl-C or Ctrl-Z signals do noot terminate or suspend the shell.

### Prereqs

Before running this program,  have the following installed:
- GCC (GNU Compiler Collection) or other C compiler
- GNU Make

### Installing

To compile the shell:

1. Clone this repo to your local machine using `git clone`, or download & extract ZIP file.

2. Navigate to the directory containing the project files.

3. Run the following command to build the shell:

    ```bash
    make
    ```

This will compile the source code and generate an executable named `nyush`.

### Running the Shell

After compilation, you can start the shell by executing:

```bash
./nyush
