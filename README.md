# SICXE Disassembler Project

## Description
-This is a class project for my Systems Programming course. It is a simple SICXE Disassembler that disassembles hexadecimal object code into assembly code. 
-The first part of this program gets each instruction in the text record of the object code, extracting the mnemonic instruction,
the instruction's format, the operand addressing type, the target address addressing mode, and the object code. 
-The second part of this program first parses the object code file for the header, text, and modification records. 
-It then parses a second file, test.sym, for the symbol and literal tables and getting, if they exist, the symbol label information and
the constants and literals information. 
-Finally, after parsing all of this information it then outputs the assembly code into the output file, out.lst. 

## Requirements
-C++11 compatible compiler
-`make` utility installed on your system
-the input files: test.obj and test.sym

## Setup
### Cloning the repository
-To start this project, clone the repository from Github by running the following command: 
```bash
git clone https://github.come/username/disassembler-project.git
```

### Running the code
-Once you have the repository cloned and have all the code files, you simply just have to run the Makefile. 
```bash
make
```
