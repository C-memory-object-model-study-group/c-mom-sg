% Clarifying uninitialised reads v5 - working draft
% Kayvan Memarian, Peter Sewell. University of Cambridge
% 2021-03-09



Reading uninitialised values and padding bytes have many conceivable
semantics in C, there are conflicting demands and expectations, and
the current standard text is not clear about all aspects.  This note
summarises the design space with a series of questions.

WG14 has previously (DR451 CR) concluded "The committee agrees that
this area would benefit from a new definition of something akin to a
'wobbly' value and that this should be considered in any subsequent
revision of this standard. The committee also notes that padding bytes
within structures are possibly a distinct form of 'wobbly'
representation.".  This note explores the design space for what that
might mean in detail.

This is another part of the C memory object model, orthogonal to provenance. 

# Related papers

This is a revision of part of 
[notes98: Clarifying Uninitialised Values (Q47-Q59) v4 - working draft
Kayvan Memarian, Victor Gomes, Peter Sewell. University of Cambridge
2018-04-21](http://www.cl.cam.ac.uk/users/pes20/cerberus/notes98-2018-04-21-uninit-v4.html) and the padding discussion of [N2013](http://www.cl.cam.ac.uk/users/pes20/cerberus/notes30.pdf). 


notes98 was a WG14 Brno meeting working draft that did not have an N-number; it was a revision of
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


Papers by Juneyoung Lee et al.: 
(PLDI 2017) Taming Undefined Behavior in LLVM; 
(OOPSLA 2018) Reconciling High-level Optimizations and Low-level Code in LLVM.
	
# Uninitialised reads

## From the programmer and existing-code points of view

In most cases, reading an uninitialised value is a programmer error, and it is/would be desirable for implementations to report this promptly, at compile-time or run-time, wherever they reasonably can.

There are some significant exceptions, where reading uninitialised values is either desirable or is endemic in practice:

- It should be possible to copy a partially initialised struct, either explicitly by a struct assignment, or implicitly as a function-call arguments, or by `memcpy`.

- There's an occasional but real use-case of debug printing partially initialised structs. Making that UB could be very confusing if it gets exploited by compilers.

- Sometimes sets of flags (stored as bits in integer-typed variables, not as bitfields) are initialised incrementally, e.g. by reading a possibly-uninitialised value, doing some arithmetic or bitwise-logical operations on it, and storing the result back. Kostya Serebryany reported some time ago that this had to be allowed in their sanitisers, as there are too many instances to require them to be fixed.

  That said, we guess (without evidence) that not much memory is used in this way, and hence that the runtime/code-size cost of requiring zero-initialisation would be small. Although that might reduce error-detection opportunities, e.g. if this is allowed by special-casing some set-bit operations.

- Some polymorphic bytewise operations on structs should be supported e.g. (library or user) implementations of `memcpy` and `memcmp`, serialisation, encryption, and hashing. These necessarily will read and write any uninitialised members and padding bytes in the struct layout, and there should be some way for the programmer to make them well-defined and deterministic. 

- Closely related to that, for struct copies and for those polymorphic bytewise operations, there should be some way (either always on by default, or enabled by some compiler option or annotation, or some specific programming idiom) for the user to ensure they have results that cannot leak potentially-security-relevant information.


Our 2015 survey of C experts gave basically bimodal results between options (a) and (d) below. We asked (survey question 2/15): Is reading an uninitialised variable or struct member (with a current mainstream compiler):

- 139 (43%) undefined behaviour (meaning that the compiler is free to arbitrarily miscompile the program, with or without a warning)

-  42 (13%) going to make the result of any expression involving that value unpredictable

-  21 (6%) going to give an arbitrary and unstable value (maybe with a different value if you read again)

- 112 (35%) going to give an arbitrary but stable value (with the same value if you read again)

A straw poll at EuroLLVM 2018 gave roughly the same distribution. The
survey comments suggest that some (but perhaps not much) real code
depends on one of the stronger semantics.


## Compiler behaviour

Compilers do sometimes optimise in ways that make multiple reads of an
uninitialised value observably different (e.g. as a natural
consequence of SSA form).

There is ongoing work on the LLVM poison, freeze, and undef, since we
started to look at these questions.  We don't know the current state
of that in detail, but some preliminary discussions have suggested that the C source semantics proposed here and the poison+freeze semantics proposed for LLVM can be made consistent. We also don't know the gcc internals.


## C++

It's probably essential for C and C++ to have broadly the same semantics for this, but we don't discuss the current C++ semantics in this note.


## The current C standard text - uninitialised values

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


## Design questions - trap representations

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


## Design questions - unspecified values

### Reading uninitialised values

```c
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
```c
Example unspecified_value_stability.c
#include <stdio.h>
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
"instance" is per-initialisation or per-read, or whether
"uninitialisedness" propagates through expression evaluation.

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
```c
Example unspecified_value_strictness_and_1.c
#include <stdio.h>
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


Option 1a is implemented in the Cerberus C semantics, so we have experience of working through all the details. 


### Q54 Must unspecified values be considered daemonically for identification of other possible undefined behaviours?

```c
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


### Q50 Can control-flow choices based on unspecified values be assumed to make an unspecified (arbitrary) choice (not giving rise to undefined behaviour)?
```c
Example unspecified_value_control_flow_choice.c
#include <stdio.h>
int main() 
{
  unsigned char c; 
  unsigned char *p = &c;
  if (c == 'a') 
    printf("equal\n");
  else
    printf("nonequal\n");
  // should this have defined behaviour?
}
```

We suggest "yes": 

a. permit a runtime unspecified (nondeterministic) choice at any control-flow choice between specified alternatives based on an unspecified values. 
b. more conservatively, one could treat any control-flow choice whose controlling expression has an unspecified value as having undefined behaviour. 
c. other

The only potential argument against (a) that we are aware of is (as noted by Joseph Myers) jump tables indexed by an unspecified value, if implementations don't do a range check.  Do they? That seems likely to lead to security weaknesses. 

Computed gotos (if they were allowed in the standard) on unspecified values should give undefined behaviour in any case.

IIRC in LLVM branches on poison are UB, so choosing (a) for the source
semantics would necessitate a freeze for controlling expressions that
might be unspecified values.


### Q49 Can library calls with unspecified-value arguments be assumed to execute with an arbitrary choice of a concrete value (not necessarily giving rise to undefined behaviour)?

```c
Example unspecified_value_library_call_argument.c
#include <stdio.h>
int main() 
{
  unsigned char c; 
  unsigned char *p = &c;
  printf("char 0x%x\n",(unsigned int)c);
  // should this have defined behaviour?
}
```
The DR451 CR says "library functions will exhibit undefined behavior when used on indeterminate values" but here we are more specifically looking at unspecified values. We see no benefit from making this undefined behaviour, and we are not aware that compilers assume so. It prevents (e.g.) serialising or debug printing of partially uninitialised structs, or (if padding bytes are treated the same as other uninitialised values) byte-by-byte serialising of structs containing padding. Accordingly, we suggest that library functions such as printf, when called with an unspecified value, are executed by first making an unspecified (nondeterministic) choice at call-time of a concrete value. This permits the instability of uninitialised values that we see in practice.



### Can a structure containing an uninitialised member can be copied as a whole?

```c
Example unspecified_value_struct_copy.c
#include <stdio.h>
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

```c
Example unspecified_value_struct_copy_2.c
#include <stdio.h>
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

```c
Example besson_blazy_wilke_bitfields_1u.c
#include <stdio.h>
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
```c
Example unspecified_value_representation_bytes_1.c
#include <stdio.h>
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
```c
Example unspecified_value_representation_bytes_4.c
#include <stdio.h>
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
```c
Example unspecified_value_representation_bytes_2.c
#include <stdio.h>
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


## Summary of proposal

We could express the above Option 1 by a modest change to the C abstract machine: for any scalar type, we extend the set of values of that type with a symbolic "unspecified value" token, then we can give rules defining how that is propagated, e.g. if one adds an unspecified value and a concrete integer. The unspecified value token does not have a bit-level representation. 
notes

[notes98: Clarifying Uninitialised Values (Q47-Q59) v4 - working draft
Kayvan Memarian, Victor Gomes, Peter Sewell. University of Cambridge
2018-04-21](http://www.cl.cam.ac.uk/users/pes20/cerberus/notes98-2018-04-21-uninit-v4.html) detailed a possible technical corrigendum for this. 

As always, the "as if" rule applies: the fact that the abstract machine manipulates an explicit "unspecified value" token doesn't mean that implementations have to. Normal implementations may at compile-time but will not at runtime: at runtime they will typically have some arbitrary bit pattern where the abstract machine has the unspecified value token, and the looseness of the rules for operations on unspecified values licenses compiler optimisations. 

This semantics seems to be a reasonable and coherent choice, with several benefits:

- it makes bytewise copying of uninitialised objects (and partially uninitialised objects) legal, e.g. by user-code analogues of memcpy;

- it makes bytewise (un)serialisation of such objects legal, though with nondeterministic results for the values of uninitialised bytes as viewed on disc;

- it permits SSA optimisations for unspecified values;


However, there are some things it does not support:

- it does not support bytewise hashing or comparison of such objects, or (un)serialisation that involves compression, as the unspecified values will infect any computation more complex than a copy;

- it does not support copying or (un)serialisation at larger than byte granularities, even with -fno-strict-aliasing, for the same reason; and

- the semantics for printf and other library calls effectively presumes that their arguments are "frozen", which isn't really coherent with the fact that they will be compiled by the same compiler.

So all this should be discussed.

Perhaps there should be a language-level analogue of the "freeze" being introduced in LLVM, to support code that knowingly manipulates potentially uninitialised values. We do not here attempt to specify that.

If we are keeping the concept of trap representations, with _Bool as a
type (possibly the only one in normal implementations) that has them,
then copying a partly uninitialised struct member-by-member will give
rise to UB. How about copying a partially uninitialised struct as a
whole? Is this an argument for making _Bool have no trap
representations, instead making it UB to use non-canonical values in
boolean operations or control-flow choices?


# Padding bytes

## The current C standard text - padding bytes

The standard has two quite distinct notions of padding: padding bytes
in structures and unions, and padding bits within the representation
of integer types.  We focus here just on the former.

Padding can be added by an implementation between the members of a
structure, or at the end of a structure or union, but not before the
first member:

6.7.2.1p17 "...There may be unnamed padding within a structure object, but not
at its beginning."

6.7.2.1p19 "There may be unnamed padding at the end of a structure or union."


Padding for (implicitly initialised) static or thread storage duration
objects is initialised to zero:

6.7.9p10 "If an object that has static or thread storage duration is
not initialized explicitly, then: [...] any padding is initialized to
zero bits"

Curiously, that omits to specify the initialisation of padding for
such an object that _is_ initialised explicitly; that should
presumably be fixed.


For automatic storage duration objects, the initialisation of padding
is not described.  All that is specified is that, if there is not an
explicit initialiser, the abstract value is indeterminate:

6.7.9p10 "If an object that has automatic storage duration is not
initialized explicitly, its value is indeterminate."


When a value is written to a struct or union, the padding bytes "take
unspecified values":

6.2.6.1 "When a value is stored in an object of structure or union
type, including in a member object, the bytes of the object
representation that correspond to any padding bytes take unspecified
values. 55)"

(curiously, 6.2.6.1 Footnote 55 says "Thus, for example, structure
assignment need not copy any padding bits", but should refer to
padding _bytes_; that should be fixed)

The force of "including in a member object" here is unclear: does that
mean that if one writes to a struct member member, then the padding
bytes of the enclosing struct, and perhaps also of any of its other
members, also take unspecified values?


7.24.4.3p2 (`memcmp`) Footnote 331): "The contents of "holes" used as padding for purposes of alignment within structure objects are indeterminate. [...]"

It's unclear whether this applies just for `memcmp` or more
generally.  If padding bytes were intrinsically indeterminate, there would be _no_ way for programmers to ensure an absence of information flow.
Though as a footnote this is non-normative in any case.


## From the programmer and existing-code points of view

Usually programmers are not concerned with the contents of padding bytes.  The exceptions are those we mentioned above:

- Some polymorphic bytewise operations on structs should be supported e.g. (library or user) implementations of `memcpy` and `memcmp`, serialisation, encryption, and hashing. These necessarily will read and write any uninitialised members and padding bytes in the struct layout, and there should be some way for the programmer to make them well-defined and deterministic. 

- Closely related to that, for struct copies and for those polymorphic bytewise operations, there should be some way (either always on by default, or enabled by some compiler option or annotation, or some specific programming idiom) for the user to ensure they have results that cannot leak potentially-security-relevant information.

More specifically, (library or user bytewise) `memcpy` should always
just work, while for some other operations one might expect the
programmer to have to explicitly ensure that padding bytes have
particular values.


Our survey had quite mixed responses: 

[1/15] How predictable are reads from padding bytes?

If you zero all bytes of a struct and then write some of its members, do reads of the padding return zero? (e.g. for a bytewise CAS or hash of the struct, or to know that no security-relevant data has leaked into them.)

Will that work in normal C compilers?

- yes : 116 (36%)
- only sometimes : 95 (29%)
- no : 21 ( 6%)
- don't know : 82 (25%)
- I don't know what the question is asking : 3 ( 1%)
- no response : 6

Do you know of real code that relies on it?
 
- yes : 46 (14%)
- yes, but it shouldn't : 31 ( 9%)
- no, but there might well be : 158 (49%)
- no, that would be crazy : 58 (18%)
- don't know : 25 ( 7%)
- no response : 5

If it won't always work, is that because [check all that apply]:

- you've observed compilers write junk into padding bytes : 31
- you think compilers will assume that padding bytes contain unspecified values and optimise away those reads : 120
- no response : 150
- other : 80 


## Compiler behaviour

We don't have good data about what modern compilers actually do for
padding - comment would be welcome - but they might write to padding
bytes for several reasons:

- if the target hardware architecture does not support the natural width of a write
- if it combines writes to adjacent objects into one
- if it converts a read-write pair (e.g. of a struct) into a `memcpy`

These might affect the padding within and after any members written,
but nowhere else.

The fact that concurrent access to distinct members is allowed
constrains wide writes to not touch unwritten members, at least in the
absence of sophisticated analysis.

Compilers might also not write to or read from padding when one might
expect them to, e.g. conceivably a `memcpy` of a struct could be
optimised by omitting to copy some padding, and a read of padding
could be omitted and replaced by use of whatever value happens to be
in some processor register.

A struct copy could therefore either write to or not write any padding
in the struct.

The values of writes to padding could come from a read of that padding
in another struct of the same type (e.g. in a struct copy), or from
whatever value happens to be in some processor register, or be zero or
another fixed value chosen by the compiler.

It's also conceivable that a compiler would reserve space in a
structure or union type for its own purposes, e.g. for some
bounds-checking or debug metadata, but we imagine that this
does not actually happens in mainstream implementations.


## Design questions - padding bytes

There are several options for the semantics of padding. Presuming one has adopted the Option 1 symbolic unspecified-value token semantics, the choices for padding include:

a. regarding padding bytes as holding unspecified-value tokens throughout the lifetime of the object, irrespective of any writes to them

b. when a struct/union or a struct/union member is written, deeming the semantics as also having written unspecified-value tokens to all padding bytes of that struct/union (including of all sub-members)

c. when a member is written, deeming the semantics as also having written unspecified-value tokens to adjacent padding (and when a struct/union is written as a whole, deeming the semantics as also having written unspecified-value tokens to all padding bytes of that struct/union (including of all sub-members))

d. as (c) but "following" instead of "adjacent": when a member is written, deeming the semantics as also having written unspecified-value tokens to subsequent padding  (and when a struct/union is written as a whole, deeming the semantics as also having written unspecified-value tokens to all padding bytes of that struct/union (including of all sub-members))

e. as (d) but "zeros" instead of "unspecified values": as when a member is written, nondeterministically either deeming the semantics as having written zeros to the subsequent padding or leaving it alone  (and when a struct/union is written as a whole, deeming the semantics as also having written zeros to all padding bytes of that struct/union (including of all sub-members))

f. as (e) but nondeterministically also allowing a struct copy to copy the source padding  (this would need to record padding values in abstract-machine struct values)

g. as (f) but also nondeterministically also allowing member copies to copy any following source padding (this would need padding values attached to all abstract-machine values, and it's not always clear what code is a "member copy")

Option (a) makes it impossible for programmers to control what's in the padding.

Options (b) and (c) are probably needlessly liberal for compilers.

Option (d) would be a plausible conservative-with-respect-to-current-implementations choice, but not very useful for programmers.

Options (e,f,g) aim to give programmers a way to maintain the
invariant that padding is zero'd, e.g. where that matters for
security. We're not sure which are sound w.r.t. current optimisations,
or (if not) which could be made to be.

As for effective types, it might be that the semantics has to consider
lvalue construction, not just the lvalue type, e.g. to know what
counts as a struct member write.


### Q61. After an explicit write of a padding byte, does that byte hold a well-defined value? (not an unspecified value)

```c
Example padding_unspecified_value_1.c
#include <stdio.h>
#include <stddef.h>
typedef struct { char c; float f; int i; } st;
int main() {
  // check there is a padding byte between c and f
  size_t offset_padding = offsetof(st,c)+sizeof(char);
  if (offsetof(st,f)>offset_padding) {
      st s; 
      unsigned char *p = ((unsigned char*)(&s))
        + offset_padding;
      *p = 'A';
      unsigned char c1 = *p; 
      // does c1 hold 'A', not an unspecified value?
      printf("c1=%c\n",c1);
  }
  return 0;
}
```
For objects with static, thread, or automatic storage durations, and
leaving aside unions, for each byte it's fixed whether it’s a padding
byte or not for the lifetime of the object, and one could conceivably
regard the padding bytes as being unspecified values irrespective of
any explicit writes to them (for a union, the padding status of a byte
depends on which member the union "currently contains") (For objects
with allocated storage duration, as malloc'd region can be reused,
that would have to be intertwined with effective-type semantics, which
is still unclear.)  But this would give programmers no way to ensure
the absence of information leaks.

If we've gone for Option 1 for uninitialised reads, it'd be
straightforward to use the same mechanism for uninitialised padding
bytes, and write the unspecified-value token to padding in some
circumstances.  That would let it be overwritten with a concrete value
if desired.

### Q62. After an explicit write of a padding byte followed by a write to the whole structure, does the padding byte hold a well-defined value? (not an unspecified value)

```c
Example padding_unspecified_value_2.c:
#include <stdio.h>
#include <stddef.h>
typedef struct { char c; float f; int i; } st;
int main() {
  // check there is a padding byte between c and f
  size_t offset_padding = offsetof(st,c)+sizeof(char);
  if (offsetof(st,f)>offset_padding) {
      st s; 
      unsigned char *p = 
        ((unsigned char*)(&s)) + offset_padding;
      *p = 'B';
      s = (st){ .c='E', .f=1.0, .i=1};
      unsigned char c2 = *p; 
      // does c2 hold 'B', not an unspecified value?
      printf("c2=0x%x\n",(int)c2);
  }
  return 0;
}
```
Here we see reads both of `B` and of `0x0`.
Changing the example to one in which the compiler might
naturally use a 4-byte copy, we sometimes see an overwrite
of the padding byte on the write of the struct value.

### Q63. After an explicit write of a padding byte followed by a write to adjacent members of the structure, does the padding byte hold a well-defined value? (not an unspecified value)


```c
Example padding_unspecified_value_7.c:
#include <stdio.h>
#include <stddef.h>
typedef struct { char c; float f; int i; } st;
int main() {
  // check there is a padding byte between c and f
  size_t offset_padding = offsetof(st,c)+sizeof(char);
  if (offsetof(st,f)>offset_padding) {
      st s; 
      unsigned char *p = 
        ((unsigned char*)(&s)) + offset_padding;
      *p = 'C';
      s.c = 'A';
      s.f = 1.0;
      s.i = 42;
      unsigned char c3 = *p; 
      // does c3 hold 'C', not an unspecified value?
      printf("c3=%c\n",c3);
  }
  return 0;
}
```

This is perhaps the most relevant of these cases in practice, covering the case where the whole footprint of the struct
has been filled with zero before use, and also covering the
case where all members of the struct have been written (and
hence where compilers might coalesce the writes). By requiring the explicit write to be of zero, compilers could implement this either by preserving the in-memory padding
byte value or by writing a constant zero to it. Whether that
would be sound w.r.t. actual practice is unclear.
But it would be useful.

### Q65. After an explicit write of a padding byte followed by a write to a non-adjacent member of the whole structure, does the padding byte hold a well-defined value? (not an unspecified value)

```c
Example padding_unspecified_value_5.c:
#include <stdio.h>
#include <stddef.h>
typedef struct { char c; float f; int i; } st;
int main() {
  // check there is a padding byte between c and f
  size_t offset_padding = offsetof(st,c)+sizeof(char);
  if (offsetof(st,f)>offset_padding) {
      st s; 
      unsigned char *p = 
        ((unsigned char*)(&s)) + offset_padding;
      *p = 'C';
      s.i = 42;
      unsigned char c3 = *p; 
      // does c3 hold 'C', not an unspecified value?
      printf("c3=%c\n",c3);
  }
  return 0;
}
```



### Q66. After an explicit write of a padding byte followed by a writes to adjacent members of the whole structure, but accessed via pointers to the members rather than via the structure, does the padding byte hold a well-defined value? (not an unspecified value)

```c
Example padding_unspecified_value_6.c:
#include <stdio.h>
#include <stddef.h>
void g(char *c, float *f) {
  *c='A';
  *f=1.0;
}
typedef struct { char c; float f; int i; } st;
int main() {
  // check there is a padding byte between c and f
  size_t offset_padding = offsetof(st,c)+sizeof(char);
  if (offsetof(st,f)>offset_padding) {
      st s; 
      unsigned char *p = 
        ((unsigned char*)(&s)) + offset_padding;
      *p = 'D';
      g(&s.c, &s.f);
      unsigned char c4 = *p; 
      // does c4 hold 'D', not an unspecified value?
      printf("c4=%c\n",c4);
  }
  return 0;
}
```

Compiler optimisations could perhaps recognise the accesses are to
adjacent struct members and coalesce them.


### Q60. Can structure-copy copy padding?

```c
Example padding_unspecified_value_1.c:
#include <stdio.h>
#include <stddef.h>
typedef struct { char c; float f; int i; } st;
int main() {
  // check there is a padding byte between c and f
  size_t offset_padding = offsetof(st,c)+sizeof(char);
  if (offsetof(st,f)>offset_padding) {
      st s; 
      unsigned char *p = ((unsigned char*)(&s))
        + offset_padding;
      *p = 'A';
      unsigned char c1 = *p; 
      // does c1 hold 'A', not an unspecified value?
      printf("c1=%c\n",c1);
  }
  return 0;
}
```


