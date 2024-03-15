# Notes regarding the second project

## Directory layout

Several files were moved to keep directories clean.
The directory tree should look like so:

```
|- 	bin
|	\-	...
|-	doc
|	|-	debugging.md
|	|-	enunciado.md
|	|-	guidelines.md
|	|-	informacoes.md
|	\-	README.md [this]
|-	public-tests
|	\-	...
\-	...
```

## Public tests

The `public-tests` folder contains:
- Public and private tests from project1
- Tests made by students for project1
- A custom test made by me to verify the behaviour of names in quotes
- The public tests for project2

## The makefile

I also wrote a custom makefile to help streamline the development of the project. It contains the following commands:
- `run`: Compiles the project (if needed) with all debugging macros and runs it.
- `build`: Compiles the project (if needed) with all debugging macros but does not run.
- `debug`: Compiles the project (if needed) with all debugging macros unrelated to memory and runs it with valgrind.
- `test`: Always compiles the project without any debugging macros and invokes the modified testing makefile. Deletes the binaries after testing.
- `clean`: Deletes all binaries.
- `clean-test`: Invokes the modified testing makefile to clean up the `.diff` files.

## Debugging macros

Several extra features were implemented for ease of testing or debugging, all of which enabled via the use of macros. In the first project these were enabled by uncommenting `#define` pre-processor calls in the code itself, but that was abandoned for this project and now those macros should be set via compilation arguments.

The available macros are:
- `CONTROL_MEMORY_ALLOCS`: Enables tracking of all memory allocs, frees and reallocs. **PERFORMANCE TAXING**
- `MAX_NAMES <count>`: Defines maximum number of operations supported per memory block. Default value is 4. Raise if using reallocs.
- `DEBUG`: Adds two extra commands: `m` for memory allocs/free information (if enabled) and `w` to test the `copyName` function (prints first read value). In project1 also added command `t`.
- `LAST_MEM_LIST`: Enables dumping of memory allocations that didn't get freed by the end of the program (if enabled). **SLOWS EXIT**
- `USE_SMARTER_ERROR_OUTPUT_STREAM`: Outputs errors to `stderr` instead of `stdout`.
- `PRINT_EMPTY_ROUTES`: Prints empty lines for each empty route in command `c`.
- `ALLOW_INVERT_CAPS`: Allows capitalization of letters in `inverso` when using command `c`.
