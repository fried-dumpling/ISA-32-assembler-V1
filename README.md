# Description
<div> __Custom assembler__ for **_DPI ISA-32_** architecture system of CPU, computer and OS. </div>
<div> The cpu is </div>

# How to use
Run this executable in cmd and pass in following arguement
```
"ISA-32_assembler.x64.exe" <input path> <output path> <-file, -token, -eval, -bin, -double>
```
## Arguement
- `<input path>` is path of an inputting text file
- `<output path>` is path of an outputing bin file ( though extention is .txt )
- `<-file>` dumps input text file
- `<-token>` dumps lexed token
- `<-bin>` dumps output bin in hex
- ~~eval~~ is not in use
- ~~double~~ is not in use

# Supporting feature
## Instruction
<div> Currently supports only __ALU__, __register__, __stack__, __ram__ and __flow__ instruction. </div>

<div> Instruction including __cache__, __MMU__, __interrupt__ or other __system__ and __kernel__ is **_not supported_**. </div>

### Supported instructions
```
add, addc, addi
sub, subc, subi
bxr, bxri
bor, bori
bnd, bndi
rol, roli, ror, rori
shiftl, shiftli, shiftr, shiftri
cmp, test
mov, set, setz, sets
pop, push
ld, st
jmp, ijmp
call
ret
nop
halt
```

## Suppported feature
- grammer
  - label
  - defining const expr
  - multi line comment
- assembly
  - register slicing
  - data assignment

# Assembly grammer

## Registers
__Syntax__
```
% <register name> <register mode>
```
### Names
```
gen[0], gen[1], gen[2]... gen[24]
sbp
zero, one, full
pc
stack
flag
```
- General purpose
  - `gen[0~24]`: 25 general purpose reigster.
  - `sbp`: Typically promised as stack bottom pointer, but doesn't matter if you really want.
- Read only (doesn't change if you store in it)
  - `zero`: Constant 0
  - `one`: Constant 1
  - `full`: Constant __unsigned int32__ 0xFFFFFFFF or __signed int32__ -1
- Special
  - `PC`: Program counter, points to the address of the next instruction. Read only.
  - `stack`: Points to stack's top, can be read and written. However, `push` and `pop` changes its value.
  - `flag`: State of current cpu and result of ALU operations
### Modes
```
.32B
.16L
.8L
.8H
.S16H
.S16L
.S8L
.S8H
```
- Signed
  - `.32B`: signed and unsigned 32 bits of the register. (because full width is 32bit, it doesn't care if it's signed or not)
  - `.S16H`: signed high 16bits of the register.
  - `.S16L`: signed low 16bits of the register.
  - `.S8L`: signed low 8bits of the register.
  - `.S8H`: signed high 8bits of the register.
- Unsigned
  - `.32B`: signed and unsigned 32 bits of the register. (because full width is 32bit, it doesn't care if it's signed or not)
  - `.16L`: unsigned low 16bits of the register.
  - `.8L`: unsigned low 8bits of the register.
  - `.8H`: unsigned high 8bits of the register.
<div> __Register slicing diagram__ </div> 

```
|-------------------------|
| 32B                     |
|-------------------------|
| 16H        | 16L        |
|-------------------------|
|            | 8H  | 8L   |
|-------------------------|
```

__Example Syntax__
```
%gen[0].32B
%pc.8H
```

## Flags
Flag keywords are used as jmp instruction's condition __arguement__.
### Names
```
zero
neg
pos
carry
carry4
overflow
one
gen
```
- Read only
  - `zero`: Whether current ALU operaton resulted in zero.
  - `neg`: Whether current ALU operaton resulted in negative number, **_excluding zero_**.
  - `pos`: Whether current ALU operaton resulted in positive number, **_excluding zero_**.
  - `carry`: Whether current ALU operaton generated carry. It is also used to __inform carry signal__ for some __ALU instruction__
  - `carry4`: Whether current ALU operaton generated carry in 4th bit. Used in binary to decimal convertion.
  - `overflow`: Whether current ALU operation resulted overflow.
  - `one`: Constant 1 or `true`
- General purpose
  - `gen`: General purposed condition flag.
- Used by instruction
  - `carry`: Whether current ALU operaton generated carry. It is also used to __inform carry signal__ for some __ALU instruction__

__example__
```
jmp.zero %pc.32B, 0
ijmp.carry %pc.32B, 1
jmp.one %pc.32B, 2
```
<div> __Flag slicing diagram__ </div> 

```
|-------------------------|
| 32B                     |
|-------------------------|
| GP (24bit)       | flag |
|-------------------------|

flag:
|----------------------------------------------------------------------------------------|
| gen      | one      | overflow | carry4   | carry    | pos      | neg      | zero      |
|----------------------------------------------------------------------------------------|
```

## Instruction syntax

- `add <dest reg> <src reg>`:
  - `<dest reg>` = `<dest reg>` + `<src reg>`
- `addc <dest reg> <src reg>`:
  - `<dest reg>` = `<dest reg>` + `<src reg>` with carry in
- `addi <dest reg> <immidate>`:
  - `<dest reg>` = `<dest reg>` + `<immidate>`
- `sub <dest reg> <src reg>`:
  - `<dest reg>` = `<dest reg>` - `<src reg>`
- `subc <dest reg> <src reg>`:
  - `<dest reg>` = `<dest reg>` - `<src reg>` with carry in
- `subi <dest reg> <immidate>`:
  - `<dest reg>` = `<dest reg>` - `<immidate>`
- `bxr <dest reg> <src reg>`:
  - `<dest reg>` = `<dest reg>` bitwise xor `<src reg>`
- `bxri <dest reg> <immidate>`:
  - `<dest reg>` = `<dest reg>` bitwise xor `<immidate>`
- `bor <dest reg> <src reg>`:
  - `<dest reg>` = `<dest reg>` bitwise or `<src reg>`
- `bori <dest reg> <immidate>`:
  - `<dest reg>` = `<dest reg>` bitwise or `<immidate>`
- `bnd <dest reg> <src reg>`:
  - `<dest reg>` = `<dest reg>` bitwise and `<src reg>`
- `bndi <dest reg> <immidate>`:
  - `<dest reg>` = `<dest reg>` bitwise and `<immidate>`
- `rol <dest reg> <src reg>`:
  - `<dest reg>` = `<dest reg>` roll left by `<src reg>`
- `roli <dest reg> <immidate>`:
  - `<dest reg>` = `<dest reg>` roll left by `<immidate>`
- `ror <dest reg> <src reg>`:
  - `<dest reg>` = `<dest reg>` roll right by `<src reg>`
- `rori <dest reg> <immidate>`:
  - `<dest reg>` = `<dest reg>` roll right by `<immidate>`
- `shiftl <dest reg> <src reg>`:
  - `<dest reg>` = `<dest reg>` shift left by `<src reg>`
- `shiftli <dest reg> <immidate>`:
  - `<dest reg>` = `<dest reg>` shift left by `<immidate>`
- `shiftr <dest reg> <src reg>`:
  - `<dest reg>` = `<dest reg>` shift right by `<src reg>`
- `shiftri <dest reg> <immidate>`:
  - `<dest reg>` = `<dest reg>` shift right by `<immidate>`
- `cmp <dest reg> <src reg>`:
  - result is only stored in flag
  - `<dest reg>` - `<src reg>`
- `test <dest reg> <src reg>`:
  - result is only stored in flag
  - `<dest reg>` = `<dest reg>` bitwise and `<src reg>`

```
mov
set
setz
sets
pop
push
ld
st
jmp
ijmp
call
ret
nop
halt
```


## Feature
<div> Supports __define__, __data section allocation__, __cpu instruction__,  __label__, and __c-style multi line comment__ </div>
<div> Everything else including __bss__, __macro__, etc.. is **_not supported_** </div>

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


