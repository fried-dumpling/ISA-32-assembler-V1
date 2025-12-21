# Description
__Custom assembler__ for **_DPI ISA-32_** architecture system of CPU, computer and OS.

# How to use
run this executable in cmd and pass in arguement
```
assembler.exe <input path> <output path> <-file, -token, -eval, -bin, -double>
```
## Arguement
`<input path>` is path of an inputting text file
`<output path>` is path of an outputing bin file ( though extention is .txt )
`<-file>` dumps input text file
`<-token>` dumps lexed token
`<-bin>` dumps output bin in hex
~~eval~~ is not in use
~~double~~ is not in use

# Current grammer
## Instruction
Currently supports only __ALU__, __register__, __stack__, __ram__ and __flow__ instruction.
Instruction including __cache__, __MMU__, __interrupt__ or other __system__ and __kernel__ is **_not supported_**.

## Supported instructions
  ### ALU
```
add, addc, addi,
sub, subc, subi,
bxr, bxri,
bor, bori,
bnd, bndi,
rol, roli, ror, rori,
shiftl, shiftli, shiftr, shiftri,
cmp, test
```
### Register
```
mov, set
```
### Stack
```
pop, push
```
### RAM
```
ld, st
```
### Flow
```
jmp, ijmp
call,
ret
nop, 
halt
```

## Registers
```
% <reigster name> <register mode>
```
example
```
%gen[0].32B
%pc.8H
```
### Names
```
gen[0], gen[1], gen[2]... gen[24]
sbp,
zero, one, full,
pc,
stack,
flag
```
### Modes
```
.32b
.16L
.8L
.8H
.S16H
.S16L
.S8L
.S8H
```

## Flags
example
```
jmp.zero %pc.32B, 0
ijmp.carry %pc.32B, 1
jmp.one %pc.32B, 2
```
### Names
```
zero,
neg,
pos,
carry,
carry4,
overflow,
one,
gen
```

## Function
Supports __define__, __data section allocation__, __cpu instruction__,  __label__, and __c-style multi line comment__
Everything else including __bss__, __macro__, etc.. is **_not supported_**

## Supported function
### Define
```
#define identifier expression
```
### Section
```
.text 
 this is text section
.data
 this is data section
```
### Label
```
identifier:
```
### Comment
```
;this is comment

/* this is
multi line
comment */
```


