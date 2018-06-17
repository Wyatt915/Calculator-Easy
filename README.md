# ce — Calculator (Easy)
ce is an easy to use calculator for people born in the common era.

## Build
Calculator (Easy) is easy to build:
    
    $ make

Calculator (Easy) is easy to install:
    
    $ make install

## Usage
Calculator (Easy) is easy to use:
    
    $ ce 2+2
    4

    $ ce '(e^pi)-pi'
    19.9990999791895

    $ ce '(log(e^2)*phi-sin(pi/2))^2'
    5

    $ echo 3*5 | ce
    15

### Operators and Functions
The following basic operators are supported by ce:

Operator | Description
-------- | -----------
+,-,\*,/ | add, subtract, multiply, and divide
d | roll dice. "3d6" will roll 3 six-sided dice
^ | exponentiation
! | factorial. Evaluates as Γ(x+1) for non-integers.
sin | Basic Trig
cos | "
tan | "
csc | "
sec | "
cot | "
sqrt | square root
log | computes the natural (base e) logarithm
exp | raises e to the power of the supplied argument


### Builtin Constants

Symbol | Description
------ | -----------
e | Euler's number; base of the natural log
pi | Circle constant; ratio of circumference to diameter
phi | Golden ratio; ½(1+√5)
