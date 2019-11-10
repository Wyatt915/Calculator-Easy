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

    $ ce '(log(exp(2))*phi-sin(pi/2))^2'
    5

    $ echo 3*5 | ce
    15

### Operators and Functions
The following basic operators are supported by ce:

Operator | Description
-------- | -----------
+,-,\*,/ | add, subtract, multiply, and divide
d        | roll dice. "3d6" will roll 3 six-sided dice
^        | exponentiation
!        | factorial. Evaluates as Γ(x+1) for non-integers.
sin      | Basic Trig
cos      | "
tan      | "
csc      | "
sec      | "
cot      | "
sqrt     | square root
log      | computes the natural (base e) logarithm
exp      | raises e to the power of the supplied argument


### Builtin Constants

Symbol | Description
------ | -----------
e      | Euler's number; base of the natural log
pi     | Circle constant; ratio of circumference to diameter
phi    | Golden ratio; ½(1+√5)

### interactive-mode
Interactive-mode is reached by running
    $ ce -i
When in interactive-mode, the user is presented with a prompt. The user may input an an expression,
followed by pressing enter. The result will be presented like so:

    $ ./ce -i
    interactive-mode
    >>> sqrt(5)
    2.23606797749979    [0]
    
    >>>

The result is printed, followed by a number in [brackets]. The number in [brackets] is used to
recall previous results. For example:

    $ ./ce -i
    interactive-mode
    >>> sqrt(5)
    2.23606797749979    [0]
    
    >>> 1 + [0]
    3.23606797749979    [1]
    
    >>> [1]/2
    1.61803398874989    [2]
    
    >>>

Furthermore, brackets can contain not only integer literals, but expressions and nested brackets
too:

    $ ./ce -i
    interactive-mode
    >>> 1+1
    2                   [0]

    >>> [1-1] + 1
    3                   [1]

    >>> [3-[0]]
    3                   [2]

    >>>

To exit interactive-mode, simply type 'q':
    
    $ ./ce -i
    interactive-mode
    >>> q
    $

