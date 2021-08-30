# Uninitialised Reads v8

Focus tightly on just the core uninitialised-read case,
in the hope of some clarity and agreement.

Consider just reads of uninitialised variables at scalar non-character
types of automatic storage duration. Ignore (for now):

- allocated-storage-duration regions
- representation-byte reads, at character types
- whole-struct reads
- padding

---------------------

There are some cases where it has to be deemed an error to read a
specific object representation, e.g. to let some implementations
trap - these cases are now rare, but they include signalling NaNs and
perhaps other exotic number formats.  Call these "true trap
representations" for now.

There are some cases where it has to be deemed an error to operate on
the result of a read of a specific object representation - primarily
_Bool and floating-point cases - but where we have a free choice
whether to deem it an error to read or write them.  We'll return to
those later.

---------------------

Current standard: reads from uninitialised variables are UB unless the
address of the variable is taken, with indeterminate value.

Current compiler behaviour: 

int f() {
    int i;
    int j=i;
    return(j);
}

x86-64 gcc 11.1    		 warns, returns zero
clang 12.0.0       		 warns, compiles to just "ret"
x86-64 icc 19.0.1  		 no warning, compiles to just "ret"
x64 msvc v19.14 (WINE)   warns, returns zero

(compiling with -O2 -Wall -pedantic -std=c11)


imaging small (within one field of view) and large (2x2 tile) embryos with Luxendo light-sheet microscope

---------------------

Think uncontroversial: in this case, uninitialised reads are almost always programmer errors.
The only exception we know: partial initialisation of a bitmask.

- volatile?  no
- bitfield? out of scope (but abstract machine should perhaps only read bitfield)
- for debugging, read the previous function's variables
- for garbage collection, scanning the stack
those last two are intentional but clearly outside the scope of the standard
- implementation of strlen, reading memory in long words - different issue
- entropy - but that's terrible
- idioms like the partially-initialised struct, except for a flat set of local variables
    (where the initialisation is based on some complex control flow)
	
https://wiki.sei.cmu.edu/confluence/display/c/ERR33-C.+Detect+and+handle+standard+library+errors. covers cases of reading uninit memory that CERT knows of (incl for entropy) 

---------------------

Issues with the current semantics:

1) the address-taken exception doesn't really make sense in this case
  (especially post-Itanium).  We don't imagine examples where code
  does or should rely on it to avoid UB, and if it does, the semantics
  of such reads is unclear.

    Just for reference, here's current compiler behaviour if the address is taken.
    It removes clang and msvc warnings, but the behaviour on this example is unchanged. 

    int f() {
        int i;
        int *p=&i;
        int j=i;
        return(j);
    }

    x86-64 gcc 11.1    		 still warns, returns zero
    clang 12.0.0       		 now doesn't warn, compiles to just "ret"
    x86-64 icc 19.0.1  		 still doesn't warn,  compiles to just "ret"
    x64 msvc v19.14 (WINE)   now doesn't warn, returns zero

  We propose to make (in this case!) the semantics identical
  irrespective of whether the address is taken.

    MartinU: complete rule was for Itanium, not the address-taken exception. Not sure why the exception was there - perhaps to constrain it to automatic variables that do not escape. 
    Joseph: from 2007 notes, without address-taken had concerns about UB leaking into memcpy.  But think shouldn't have the rule now. 
    David: confirms. The exception was because people didn't find UB palatable without some out. Without that, NaT wasn't going to get into the standard; it was a concession.
	 
  Straw poll: for reads of scalar non-character types of uninitialised
  automatic storage duration variables, should the semantics be
  independent of whether the address of the variable is taken?

  (NB this is not asking "should we just delete the exception text")

  Yes:    15
  No:      2
  Abstain: 4

  Why no?  Rajan: because anyone depending on the current text shouldn't be broken. 
    (would be fine if it became more defined)
  Victor: agree with Rajan - don't see motivation
  David: would have voted no, but presuming would find some other way to allow NaT 

---------------------

2) just having UB is simple in the standard text, gives the most
implementation flexibility, and is in the style of most of the rest of
the current standard.  But it is not very helpful for programmers:

   a) Actual implementation behaviour may be build-time or run-time
   nondeterministic, which is bad for debugging.

   b) ...and that might include surprising information flow, bad for
   security.
	
   c) It allows surprising optimisations, e.g. optimising

		   int f(_Bool b) {
		       if (b) {
		           int i;
		           int j=i;
		       }
		       if (b)
		           return(3); 
		        else 
		           return(9);
		   }

   to always return 9 (though the above compilers seem not to do that).

Can we do better? 

---------------------

We could explicitly permit, at the implementation's per-instance
choice, either a compile-time error, a non-silent runtime error, or
some other stronger-than-UB behaviour

This is somewhat like the existing 6.3.1.3 semantics for
conversion, "either the result is implementation-defined or an
implementation-defined signal is raised".

Explicitly permiting errors like this accommodates the "true trap
representation" cases, and sanitiser-like debugging tools, but it's
arguably not very useful beyond that - compilers can always warn where
they wish, and to avoid false positives they would need to show
reachability.

---------------------

What could the stronger-than-UB semantics be? 

In increasing looseness:

0) a default (zero) particular concrete value, with some annotation to
let the programmer say that specific variables (e.g. large arrays)
should not be default-initialised.

https://bugzilla.mozilla.org/show_bug.cgi?id=1514965 gives some figures for Firefox benchmarks.

from the Clang review (https://reviews.llvm.org/D54604) it says: "What's the performance like? Not too bad! Previous publications [0] have cited 2.7 to 4.5% averages. " 

0.5) initialisation to a strange concrete value for better error detection

1) an object-creation-time nondeterministic particular concrete value

2) some flavour of wobbly value:

  2a) an abstract-machine-read-time nondeterministic particular
  concrete value (i.e., a wobbly value which itself remains wobbly, but
  reads of which are nondeterministic concrete values).

  2b) propagate uninitialisedness through stores (with the target
  becoming uninitialised in the same way), but make a use-time
  nondeterministic choice of a particular concrete value whenever it is
  used in some operation (including arithmetic).

  2c) propagate uninitialisedness through stores and all operations
  (including arithmetic), except allowing an explicit error (or
  daemonically giving UB) for operations where some concrete value could
  give UB (e.g. division by zero, and perhaps control-flow choices).
  Nondeterministically choose particular concrete values only at
  external library calls.

3) propagate uninitialisedness through stores, but make it UB to
operate on the uninitialised value in any other way.

---------------------

What criteria do we have for choosing between these?

(0) and (1) are the most predictable.  They have some costs, but
perhaps those are negligible in enough cases that an explicit opt-out
would suffice?

(2) and (3) accommodate compiler SSA optimisations

(2) and (3) don't give much useful strength to the programmer beyond
making uninitialised reads always UB, except that they are all making
it defined behaviour to copy an uninitialised value.


Or we can give up on increased predictability and just make it UB in
all cases...


------------

Straw poll:

Does the committee support in principle the approach of defining
a small number of alternative options for uninitialised-read semantics,
allowing users to choose as they wish? 

Yes: 6
No: 9
Abstain: 6


Straw poll:

Does the committee support in principle the approach of explicitly
permitting, at the implementation's per-instance choice, either a
compile-time error, a non-silent runtime error, or some other
more-defined-than-UB behaviour for uninitialised read semantics. 

Yes:  10
No:    9
Abstain: 3


Straw poll:

Does the committee want to make uninitialised reads of scalar non-character
typed objects of automatic storage duration always UB.

Yes:  10
No:    9
Abstain: 3
