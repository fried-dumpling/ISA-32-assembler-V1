# Description
__Custom assembler__ for **_DPI ISA-32_** architecture system of CPU, computer and OS. <br>
The cpu is

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
 Currently supports only __ALU__, __register__, __stack__, __ram__ and __flow__ instruction. <br>
 Instruction including __cache__, __MMU__, __interrupt__ or other __system__ and __kernel__ is **_not supported_**. <br>

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
  - `.32B`: __signed or unsigned__ 32 bits of the register. (because full width is 32bit, it doesn't care if it's signed or not)
  - `.S16H`: signed high 16bits of the register.
  - `.S16L`: signed low 16bits of the register.
  - `.S8L`: signed low 8bits of the register.
  - `.S8H`: signed high 8bits of the register.
- Unsigned
  - `.32B`: __signed or unsigned__ 32 bits of the register. (because full width is 32bit, it doesn't care if it's signed or not)
  - `.16L`: unsigned low 16bits of the register.
  - `.8L`: unsigned low 8bits of the register.
  - `.8H`: unsigned high 8bits of the register.

**_Note_**: <br> 
Sign mode of a register doesn't matter when it's being __written__. <br> 
The sign mode is only used to tell the CPU how to expand the register slice into __32bit__ (which is CPU's word size). <br> 
Because of this, `.32B`(full width register mode) __doesn't have a different sign mode__. <br> 

 __Register slicing diagram__
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
%flag.8L
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

### Flag register explained
 __Flag slicing diagram__
 __full 32bit flag register__
```
|-------------------------|
| 32B                     |
|-------------------------|
| GP (16b)  | sec2 | sec1 |
|-------------------------|
```

__sec1(aka. cond):__
```
|----------------------------------------------------------------------------------------|
| gen      | one      | overflow | carry4   | carry    | pos      | neg      | zero      |
|----------------------------------------------------------------------------------------|
```

__sec2(aka. ctrl):__
```
|----------------------------------------------------------------------------------------|
| trap     | int proc | GP (4bit)                                 | sc.ov    | pc.ov     |
|----------------------------------------------------------------------------------------|
```

*GP stands for __general purpose__ <br>
*sc.ov is stack overflow
*pc.ov is program counter overflow

## Instruction syntax

### add
increment `<dest reg>` by `<src reg>` <br>
__syntax:__ <br>
`'add' <dest reg> ',' <src reg>` <br>
__example:__ <br>
`add %gen[0].32B, %gen[1].32B`

### addc
increment `<dest reg>` by `<src reg>` with carry in <br>
__syntax:__ <br>
`'addc' <dest reg> ',' <src reg>` <br>
__example:__ <br>
`addc %gen[0].32B, %gen[1].32B`

### addi
increment `<dest reg>` by `<immidate>` <br>
`<immidate>` is unsigned 16bit value <br>
__syntax:__ <br>
`'addi' <dest reg> ',' <immidate>` <br>
__example:__ <br>
`addi %gen[0].32B, 0x1234`

### sub
decrement `<dest reg>` by `<src reg>` <br>
__syntax:__ <br>
`'sub' <dest reg> ',' <src reg>` <br>
__example:__ <br>
`sub %gen[0].32B, %gen[1].32B`

### subc
decrement `<dest reg>` by `<src reg>` with carry in <br>
__syntax:__ <br>
`'subc' <dest reg> ',' <src reg>` <br>
__example:__ <br>
`subc %gen[0].32B, %gen[1].32B`

### subi
decrement `<dest reg>` by `<immidate>` <br>
`<immidate>` is unsigned 16bit value <br>
__syntax:__ <br>
`'subi' <dest reg> ',' <immidate>` <br>
__example:__ <br>
`subi %gen[0].32B, 0x1234`

 ### bxr
 set `<dest reg>` to bitwise xor between `<dest reg>` `<src reg>` <br>
 __syntax:__ <br>
`'bxr' <dest reg> ',' <src reg>`
__example:__
`bxr %ger[0].32B, %gen[1].32B`

 ### bxri
 set `<dest reg>` to bitwise xor between `<dest reg>` `<immidate>` <br>
 `<immidate>` is unsigned 16bit value <br>
 __syntax:__ <br>
`'bxr' <dest reg> ',' <immidate>`
__example:__
`bxr %ger[0].32B, 0xFFFF`

 ### bor
 set `<dest reg>` to bitwise or between `<dest reg>` `<src reg>` <br>
 __syntax:__ <br>
`'bor' <dest reg> ',' <src reg>`
__example:__
`bor %ger[0].32B, %gen[1].32B`

 ### bori
 set `<dest reg>` to bitwise or between `<dest reg>` `<immidate>` <br>
 `<immidate>` is unsigned 16bit value <br>
 __syntax:__ <br>
`'bor' <dest reg> ',' <immidate>`
__example:__
`bor %ger[0].32B, 0xFFFF`

 ### bnd
 set `<dest reg>` to bitwise and between `<dest reg>` `<src reg>` <br>
 __syntax:__ <br>
`'bnd' <dest reg> ',' <src reg>`
__example:__
`bnd %ger[0].32B, %gen[1].32B`

 ### bndi
 set `<dest reg>` to bitwise and between `<dest reg>` `<immidate>` <br>
 `<immidate>` is unsigned 16bit value <br>
 __syntax:__ <br>
`'bnd' <dest reg> ',' <immidate>`
__example:__
`bnd %ger[0].32B, 0xFFFF`

### rol
left roll `<dest reg>`  by `<src reg>` <br>
__syntax:__ <br>
`'rol' <dest reg> ',' <src reg>` <br>
__example:__
`rol %gen[0].32B, %gen[1].32B`

### roli
left roll `<dest reg>` by `<immidate>` <br>
 `<immidate>` is unsigned 16bit value <br>
__syntax:__ <br>
`'roli' <dest reg> ',' <immidate>` <br>
__example:__ <br>
`roli %gen[0].32B, 0x0001`

### ror
right roll `<dest reg>` by `<src reg>` <br>
__syntax:__ <br>
`'ror' <dest reg> ',' <src reg>` <br>
__example:__
`ror %gen[0].32B, %gen[1].32B`

### rori
right roll `<dest reg>` by `<immidate>` <br>
 `<immidate>` is unsigned 16bit value <br>
__syntax:__ <br>
`'rori' <dest reg> ',' <immidate>` <br>
__example:__ <br>
`rori %gen[0].32B, 0x0001`

### shiftl
left shift `<dest reg>` by `<src reg>` <br>
__syntax:__ <br>
`'shiftl' <dest reg> ',' <src reg>` <br>
__example:__ <br>
`shiftl %gen[0].32B, %gen[1].32B`

### shiftli
left shift `<dest reg>` by `<immidate>` <br>
`<immidate>` is unsigned 16bit value <br>
__syntax:__ <br>
`'shiftli' <dest reg> ',' <immidate>` <br>
__example:__ <br>
`shiftli %gen [0].32B, %gen[1].32B`

### shiftr
right shift `<dest reg>` by `<immidate>` <br>
__syntax:__ <br>right shift `<dest reg>` by `<src reg>` <br>
`'shiftr' <dest reg> ',' <src reg>` <br>
__example:__ <br>
`shiftr %gen [0].32B, %gen[1].32B`

### shiftri
right shift `<dest reg>` by `<immidate>` <br>
`<immidate>` is unsigned 16bit value <br>
__syntax:__ <br>
`'shiftri' <dest reg> ',' <immidate>` <br>
__example:__ <br>
`shiftri %gen [0].32B, %gen[1].32B`

### cmp
cmp `<dest reg>` with `<immidate>` <br>
__syntax:__ <br>right shift `<dest reg>` by `<src reg>` <br>
`'shiftr' <dest reg> ',' <src reg>` <br>
__example:__ <br>
`shiftr %gen [0].32B, %gen[1].32B`
- `cmp <dest reg> <src reg>`:
  - result is only stored in flag
  - `<dest reg>` - `<src reg>`  <br> <br>
- `test <dest reg> <src reg>`:
  - result is only stored in flag
  - `<dest reg>` = `<dest reg>` bitwise and `<src reg>`  <br> <br>
- `mov <dest reg> <src reg>`
  - `<dest reg>` = `<dest reg>` <br> <br>
- `set <dest reg> <immidate>`
  - `<immidate>` is unsigned 16 bit
- `setz <dest reg> <immidate>` <br> <br>
  - `<immidate>` is unsigned 16 bit
  - `<dest reg>` = expanded `<immidate>` <br> <br>
- `sets <dest reg> <immidate>`
  - `<immidate>` is signed 16 bit
  - `<dest reg>` = expanded `<immidate>` <br> <br>
- `pop <dest reg>`
  - `<dest reg>` = stack's top data
  - stack counter decrement <br> <br>
- `push <src reg>`
  - stack's top data = `<src reg>`
  - stack counter increment <br> <br>
- `ld . <base reg> <dest reg> <immidate>`
  - `<immidate>` is signed 16 bit
  - `<dest reg>` = data at address `<base reg>` + `<immidate>` <br> <br>
- `st . <base reg> <dest reg> <immidate>`
  - `<immidate>` is signed 16 bit
  - data at address `<base reg>` + `<immidate>` = `<dest reg>` <br> <br>
```
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
 Supports __define__, __data section allocation__, __cpu instruction__,  __label__, and __c-style multi line comment__ <br>
 Everything else including __bss__, __macro__, etc.. is **_not supported_** <br>

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


