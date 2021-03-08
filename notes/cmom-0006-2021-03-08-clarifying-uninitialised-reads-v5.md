% Clarifying uninitialised reads v5 - working draft
% Kayvan Memarian, Peter Sewell. University of Cambridge
% 2021-03-08



Reading uninitialised values has many conceivable semantics in C, and
there are conflicting demands and expectations.


# Related papers

This is a revision of part of 
[notes98: Clarifying Uninitialised Values (Q47-Q59) v4 - working draft
Kayvan Memarian, Victor Gomes, Peter Sewell. University of Cambridge
2018-04-21](http://www.cl.cam.ac.uk/users/pes20/cerberus/notes98-2018-04-21-uninit-v4.html), a WG14 Brno meeting working draft that did not have an N-number. 
notes98 was a revision of
[N2220](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2220.htm) and 
[N2221](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2221.htm). The latter  was a revision of N2089. 
N2089 was based on N2012 (Section 2), adding a concrete
Technical Corrigendum proposal for discussion, revising the text, and adding concrete examples from N2013.


[N2223: Clarifying the C Memory Object Model: Introduction to N2219 - N2222](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2223.htm),
[N2221](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2221.htm),
[N2220](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2220.htm),
[N2089](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2089.pdf),
Section 2 of 
[N2012](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2012.htm),
Sections 3.1 and 3.2 (Q48-59) of 
[N2013](http://www.cl.cam.ac.uk/users/pes20/cerberus/notes30.pdf), 
[DR338](http://www.open-std.org/jtc1/sc22/wg14/www/docs/dr_338.htm), 
[DR451](http://www.open-std.org/jtc1/sc22/wg14/www/docs/summary.htm#dr_451), 
[N1793](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1793.pdf),
[N1818](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1818.pdf).  



# From the programmer and existing-code points of view

In most cases, reading an uninitialised value is a programmer error, and it is/would be desirable for implementations to report this promptly, at compile-time or run-time, wherever they reasonably can.

There are some significant exceptions, where reading uninitialised values is either desirable or is endemic in practice:

- It should be possible to copy a partially initialised struct, either explicitly by a struct assignment, or implicitly as a function-call arguments, or by `memcpy`.

- There's an occasional but real use-case of debug printing partially initialised structs. Making that UB could be very confusing if it gets exploited by compilers.

- It seems to be fairly common for sets of flags (stored as bits in integer-typed variables) to be initialised incrementally, e.g. by reading a possibly-uninitialised value, doing some arithmetic or bitwise-logical operations on it, and storing the result back. Kostya Serebryany reported some time ago that this had to be allowed in their sanitisers, as there are too many instances to require them to be fixed.

  That said, we guess (without evidence) that not much memory is used in this way, and hence that the runtime/code-size cost of requiring zero-initialisation would be small. Although that might reduce error-detection opportunities, e.g. if this is allowed by special-casing some set-bit operations.

- Some polymorphic bytewise operations on structs should be supported e.g. serialisation, encryption, hashing, and (library or user) implementations of memcpy. These necessarily will read and write any padding bytes in the struct layout, and it's desirable for them to be deterministic - or, at least, for it to be possible for the programmer to make them deterministic. 

- Closely related to that, for struct copies and for those polymorphic bytewise operations, there should be some way (either always on by default, or enabled by some compiler option or annotation, or some specific programming idiom) for the user to ensure they have results that cannot leak potentially-security-relevant information.


Our 2015 survey of C experts gave basically bimodal results between options (a) and (d) below. We asked (survey question 2/15): Is reading an uninitialised variable or struct member (with a current mainstream compiler):

- 139 (43%) undefined behaviour (meaning that the compiler is free to arbitrarily miscompile the program, with or without a warning)

-  42 (13%) going to make the result of any expression involving that value unpredictable

-  21 (6%) going to give an arbitrary and unstable value (maybe with a different value if you read again)

- 112 (35%) going to give an arbitrary but stable value (with the same value if you read again)

A straw poll at EuroLLVM 2018 gave roughly the same distribution. The
survey comments suggest that some (but perhaps not much) real code
depends on one of the stronger semantics.


# Compiler behaviour

Compilers do sometimes optimise in ways that make multiple reads of an
uninitialised value observably different (e.g. as a natural
consequence of SSA form).

There is ongoing work on the LLVM poison, freeze, and undef, since we
started to look at these questions.  We don't know the current state
of that, or the analogous situation for gcc.


# C++

It's probably essential for C and C++ to have broadly the same semantics for this, but we don't discuss the current C++ semantics in this note.


# The current C standard text

3.19.2 "indeterminate value: either an unspecified value or a trap
representation"

3.19.3 "unspecified value: valid value of the relevant type where this
International Standard imposes no requirements on which value is
chosen in any instance"

3.19.4 "trap representation: an object representation that need not
represent a value of the object type"

6.2.4p6,7 For objects with automatic storage duration "The initial
value of the object is indeterminate."

6.2.6.1p5 "Certain object representations need not represent a value
of the object type. If the stored value of an object has such a
representation and is read by an lvalue expression that does not have
character type, the behavior is undefined. If such a representation is
produced by a side effect that modifies all or any part of the object
by an lvalue expression that does not have character type, the
behavior is undefined. 54) Such a representation is called a trap
representation."

6.2.6.1p6 "When a value is stored in an object of structure or union
type, including in a member object, the bytes of the object
representation that correspond to any padding bytes take unspecified
values. 51) The value of a structure or union object is never a trap
representation, even though the value of a member of the structure or
union object may be a trap representation."

6.2.6.1p7 "When a value is stored in a member of an object of union
type, the bytes of the object representation that do not correspond to
that member but do correspond to other members take unspecified
values."

6.3.2.1p2 "...If the lvalue designates an object of automatic storage
duration that could have been declared with the register storage class
(never had its address taken), and that object is uninitialized (not
declared with an initializer and no assignment to it has been
performed prior to use), the behavior is undefined."


# Design questions

## Trap representations

The 3.19.4 standard text is clear that trap representations are object
_representations_, not abstract values of some kind.  It follows that
for many common implementations, many types cannot have trap
representations, because there are no object representations that do
not represent a value.  (`_Bool` is the main type that often can have
trap representations; standard integer types usually cannot.)

However, this is confused by 6.2.6.1p6, which speaks of values being
trap representations.

We assume the former reading, and propose to clarify the terminology
to make that plain - c.f. the note by Jens Gustedt and Martin Uecker.


## Unspecified values

### Reading uninitialised values

```
Example trap_representation_1.c 
int main() {
  int i;
  int *p = &i;
  int j=i;   // should this have undefined behaviour?
  // note that i is read but the value is not used
}

Example trap_representation_2.c 
int main() {
  int i;
  int j=i;   // should this have undefined behaviour?
  // note that i is read but the value is not used
}
```
In the current standard, for implementations in which `int` has no trap representations (so 6.2.6.1p5 is irrelevant), the first has defined behaviour. By 6.2.4p6 the value is indeterminate, so by the definition of indeterminate values, where there are no trap representations at `int`, the result is an unspecified value.

In the current standard, the second has undefined behaviour (by 6.3.2.1p2).

Historical question: what was the intent of the address-taken
distinction?  From previous discussion, it seems to have been an
attempt to model the Itanium NaT not a thing behaviour, but it's not
clear whether it's sufficient for that (c.f. mail with Hans Boehm, who
pointed out that inlining and optimisation might keep values in
registers, potentially with a NaT flag, even if in the source its
address is taken).

Design question: should we allow non-UB reads of uninitialised variables (at types that do not have trap representations):

1. always
2. only if their address has been taken
3. never
4. other


### Stability of uninitialised values

In the absence of any writes, is the result of reading an uninitialised object  potentially unstable, i.e., can multiple usages of it give different results?
```
Example unspecified_value_stability.c
include <stdio.h>
int main() {
  // assume here that int has no trap representations and 
  // that printing an unspecified value is not itself 
  // undefined behaviour
  int i;
  int *p = &i;
  // can the following print different values?
  printf("i=0x%x\n",i);
  printf("i=0x%x\n",i);
  printf("i=0x%x\n",i);
  printf("i=0x%x\n",i);
}
```

The current standard text is not completely clear: the 3.19.3
definition of unspecified value says "valid value of the relevant type
where this International Standard imposes no requirements on which
value is chosen in any instance", but it's unclear whether the
"instance" is per-initialisation or per-read.

Clang sometimes prints distinct values here (this is consistent with
the Clang internal documentation). Accordingly, we think the answer
has to be "yes".

Design question: In the absence of any writes, is the result of
reading an uninitialised object potentially unstable, i.e., can
multiple usages of it give different results?

1. yes
2. no
3. other


### Q52 Do operations on unspecified values result in unspecified values?
```
Example unspecified_value_strictness_and_1.c
include <stdio.h>
int main() {
  unsigned char c; 
  unsigned char *p=&c;
  unsigned char c2 = (c | 1);
  unsigned char c3 = (c2 & 1);
  // does c3 hold an unspecified value (not 1)?  
  printf("c=%i  c2=%i  c3=%i\n",(int)c,(int)c2,(int)c3);
}
```

(An LLVM developer remarked (some time ago) that different parts of
LLVM assume that undef is propagated aggressively or that it
represents an unknown particular number.)

Design question: either

1. Unspecified values are symbolic, i.e. operations on unspecified values result in unspecified-value tokens (modulo other UBs; see the next question).
2. Unspecified values exist only in the abstract-machine memory, not in expression evaluation, and a read of an unspecified value makes a read-time nondeterministic choice of a concrete value of the appropriate type.
3. Other

In Option 1, the read of c will give the unspecified value token, the result of the binary operation |, if given at least one unspecified-value arguments, will also be the unspecified value token, which will be written to c2. Likewise, the binary & will be strict in unspecified-value-ness, and c3 will end up as the unspecified value. The printf will then make nondeterministic choices for each of these, allowing arbitrary character-valued integers to be printed by implementations.

Design question: For option 1, should the unspecified value token be:

a. a per-scalar-type-object entity
b. a per-bit entity 
c. other

We think a. 

Note that (a) makes incremental bitwise initialisation not useful.
It's hard to imagine that such initialisation is desirable in new
code, but it may exist; we don't know how common it is.

It would also make the N1793 Fig.4 printhexdigit not useful when
applied to an uninitialised structure member. 


### Q54 Must unspecified values be considered daemonically for identification of other possible undefined behaviours?

```
Example unspecified_value_daemonic_1.c
int main() {
  int i;
  int *p = &i;
  int j = i;   
  int k = 1/j; // should this have undefined behaviour?
}
```
The division operation has undefined behaviour for certain concrete
argument values, i.e. 0, to accommodate implementation behaviour. If
there is an abstract-machine execution in which the second argument is
an unspecified value, then a corresponding execution of an actual
implementation might divide by zero, so in the abstract machine
division should be daemonic: division by an unspecified value should
be just as "bad" as division by zero. The same holds for other partial
operations and library calls.

For Option 1 (symbolic unspecified values) this seems forced.

For Option 2 (read-time nondeterministic choices of concrete values),
the UB will arise on one execution and hence the program will be UB.


### Can a structure containing an uninitialised member can be copied as a whole?

```
Example unspecified_value_struct_copy.c
include <stdio.h>
typedef struct { int i1; int i2; } st;
int main() {
  st s1;
  s1.i1 = 1;
  st s2;
  s2 = s1; // should this have defined behaviour?
  printf("s2.i1=%i\n",s2.i1);
}
```

There are two versions of this question: one where the uninitialised member might be a trap representation and one where it cannot be, and so is known to be an unspecified value. 

In either case, this seems to be relied on in practice, and the
6.2.6.1p6 "The value of a structure or union object is never a trap
representation" suggests the previous WG14 intent was to permit it.

In the Option 1 (symbolic unspecified-value tokens) semantics, the copy will
have the unspecified value token for the same member; the struct as a
whole will not become an unspecified value (i.e. forming a structure
value should not be strict in unspecified-value-ness).  In the Option
2 (read-time nondeterministic value) semantics, the copy would have a
nondeterministically chosen but thereafter fixed concrete value.


### Can a union-as-a-whole be an unspecified value?

In principle there is a similar question for unions: can a union value
as a whole be an unspecified value? There might be a real semantic
difference, between an unspecified value as whole and a union that
contains a specific member which itself is an unspecified value. But
it's unclear whether there is a test in ISO C that distinguishes the
two.  We presume "no".


### Can a structure containing an uninitialised member can be copied member-by-member?

```
Example unspecified_value_struct_copy_2.c
include <stdio.h>
typedef struct { int i1; _Bool b2; } st;
int main() {
  st s1;
  s1.i1 = 1;
  st s2;
  s2.i1 = s1.i1; 
  s2.b2 = s1.b2; // does this have defined behaviour?
  printf("s2.i1=%i\n",s2.i1);
}
```

A unfortunate consequence of the choices so far is that a whole-struct
copy and a member-by-member struct copy might not be equally
well-defined: the former will always be defined behaviour, while the
latter will be UB iff there is an uninitialised member of a type that
has trap representations (e.g. `_Bool` on many platforms).

If we wanted to avoid this, we would have to change the notion of trap
representation to give UB when any non-read/write operation is
performed, rather than when any read is performed.

Design question: do we want to support copying of uninitialised values
at types that have trap representations?

a. yes
b. no
c. other
 
We presume no for the time being, mostly to be conservative w.r.t. the
current standard text.


### Q56 Given multiple bitfields that may be in the same word, can one be a well-defined value while another is an unspecified value?

```
Example besson_blazy_wilke_bitfields_1u.c
include <stdio.h>
struct f {
  unsigned int a0 : 1; unsigned int a1 : 1;
} bf ;

int main() {
  unsigned int a;
  bf.a1 = 1; 
  a = bf.a1; 
  printf("a=%u\n",a);
}
```
This example is from Besson, Blazy, and Wilke 2015.

For consistency with the rest of our per-leaf-value proposal, we suggest "yes".


### Q57 Are the representation bytes of an unspecified value themselves also unspecified values? (not an arbitrary choice of concrete byte values)
```
Example unspecified_value_representation_bytes_1.c
include <stdio.h>
int main() {
  // assume here that the implementation-defined 
  // representation of int has no trap representations
  int i;
  unsigned char c = * ((unsigned char*)(&i)); 
  // does c now hold an unspecified value?
  printf("i=0x%x  c=0x%x\n",i,(int)c);
  printf("i=0x%x  c=0x%x\n",i,(int)c);
}
```
The answer to this is unclear from multiple points of view: ISO C11 doesn't address the question; we don't know whether existing compilers assume these are unspecified values, and we don't know whether existing code relies on them not being unspecified values (e.g. by assuming that one can construct a deterministic hash from the representation bytes of an uninitialised value). 

Options:

a. If we've gone for Option 1 (symbolic unspecified-value tokens), it's stylistically consistent and technically straightforward to treat the representation bytes of uninitialised values the same way.  That means representation-byte reads would give symbolic unspecified values, a bytewise `memcpy` will preserve unspecified-value-ness, and we can regard a read of a value for which any of the representation bytes have been written to be the unspecified-value token as returning an unspecified value.  This is our (at least Memarian+Sewell) currently preferred option.

b. We could conceivably have Option 1 (symbolic unspecified-value tokens) but for non-`unsigned char` types make a nondeterministic choice of concrete representation bytes at object creation time.  That would mean representation-bytes reads would deterministically return those concrete bytes, and a bytewise `memcpy` would copy them.   But this isn't really self-consistent - e.g., repeated printf of an uninitialised value would show it as unstable (consistent with observed compiler behaviour), but repeated printf of a `memcpy` of that value would show an arbitrary but stable value.

c. Other.



###  Q58 If one writes some but not all of the representation bytes of an uninitialized value, do the other representation bytes still hold unspecified values?
```
Example unspecified_value_representation_bytes_4.c
include <stdio.h>
int main() {
  // assume here that the implementation-defined
  // representation of int has no trap representations
  int i;
  printf("i=0x%x\n",i);
  printf("i=0x%x\n",i);
  unsigned char *cp = (unsigned char*)(&i);
  *(cp+1) = 0x22;
  // does *cp now hold an unspecified value?
  printf("*cp=0x%x\n",*cp);
  printf("*cp=0x%x\n",*cp);
}
```
One could conceivably take the first such access as "freezing" the unspecified value and its representation bytes, but that would be pretty strange.  The simplest choice is "yes".

a. yes
b. no
c. other



### Q59 If one writes some but not all of the representation bytes of an uninitialized value, does a read of the whole value still give an unspecified value?
```
Example unspecified_value_representation_bytes_2.c
include <stdio.h>
int main() {
  // assume here that the implementation-defined
  // representation of int has no trap representations
  int i;
  printf("i=0x%x\n",i);
  printf("i=0x%x\n",i);
  * (((unsigned char*)(&i))+1) = 0x22;
  // does i now hold an unspecified value?
  printf("i=0x%x\n",i);
  printf("i=0x%x\n",i);
}
```

One could conceivably require that a read of the whole should give a nondeterministically chosen value consistent with the concretely written representation bytes, but that would be complex and not obviously useful..  The simplest choice is "yes".

a. yes
b. no
c. other


## Padding bytes

