Uninitialised reads again, 2021-08-30
-------------------------------------

Peter Sewell, with feedback from Jens Gustedt, Kayvan Memarian, Martin Uecker

Discussed with WG14 and edited live 2021-08-31



Consider uninitialised reads of scalar non-character typed objects of
automatic storage duration, for types that do not (on the platform in
question) have trap representations.

Currrent ISO C:

1. if their address is taken: an "unspecified value"
2. if their address is never taken: UB

(from 6.2.4p6,7 "the initial value is indeterminate", and 3.19.2
"indeterminate value: either an unspecified value or a trap representation")

From the discussion last time, we believe the committee wants:

- to keep this address-taken distinction (though some of us would
  prefer to remove it).

- to have some non-error semantics for Case 1 (as existing code may be
  depending on that).

We assume both below.  If not, say so now. 


## Case 1 - address taken:

If we're giving non-error semantics to an uninitialised read, there are
two basic options:

a. an allocation-time nondeterministic choice of a concrete value
   (stable if re-read) 

b. some flavour of wobbly value

   (there are multiple options, i/ii/iii below, but we can leave them
   to later if needed)

(a) is more predictable for programmers
(b) allows certain SSA-based optimisations

The existing standard text is (in my view - some differ) ambiguous: 

  3.19.3 "unspecified value: valid value of the relevant type
  where this International Standard imposes no requirements on which
  value is chosen in any instance"

A priori, that "in any instance" could be read as:

  - any allocation-time instance 
  - any read-time instance 
  - any use-time instance 

Straw poll 1: For an uninitialised read of a scalar non-character typed
object of automatic storage duration, for a type that does not (on the
platform in question) have trap representations, if the address is
taken, which of the following should be reasonable options? 

Bearing in mind that if there are several good candidates, the TS might identify 
a set of them that compilers could adopt, not just a single option.

You can vote for as many as you like.

a. an allocation-time nondeterministic choice of a concrete value (stable if re-read) 

b. some flavour of wobbly value

some error semantics: either 
c.1 plain UB or
c.2 diagnosed compile-time or run-time error or (a) (at the impl's per-instance choice)
c.3 diagnosed compile-time or run-time error or some flavour of wobbly value (atipic)

d. some other semantics

for the address-taken variant:

a:  yes 18   no 3   abstain 3
b:    	12     10    		1
c.1:  	9      12    		2
c.2:  	16      5    		2
c.3:  	9      14    		0
d:    	2      12    		9

Straw poll 2 - the same, but a preference vote:
a: 5    b: 1  c.1:  7    c.2: 9   c.3:  2   d: 0


## Case 2 - address never taken:

Straw poll 3: for the address never taken variant of the above, preference vote:

0. the same as whatever semantics is chosen for the address-taken case

a. an allocation-time nondeterministic choice of a concrete value (stable if re-read) 

b. some flavour of wobbly value

some error semantics: either 
c.1 plain UB or
c.2 diagnosed compile-time or run-time error or (a) (at the impl's per-instance choice)
c.3 diagnosed compile-time or run-time error or some flavour of wobbly value (atipic)

d. some other semantics


0(same): 11   a: 0   b: 0  c.1: 5   c.2: 7    c.3: 0    d: 0








---------------------------------------------------------

#  Further notes, not discussed in the meeting

## Case 1 - address taken:

If there was a majority for (b), then we should ask:

Straw poll 4: For an uninitialised read of a scalar non-character typed
object of automatic storage duration, for a type that does not (on the
platform in question) have trap representations, if the address is
taken, which flavour of wobbly value should it be:

   i) an abstract-machine-read-time nondeterministic particular
   concrete value (i.e., a wobbly value which itself remains wobbly, but
   reads of which are nondeterministic concrete values).

   ii) propagate uninitialisedness through stores (with the target
   becoming uninitialised in the same way), but make a use-time
   nondeterministic choice of a particular concrete value whenever it is
   used in some operation (including arithmetic).

   iii) propagate uninitialisedness through stores and all operations
   (including arithmetic), except allowing an explicit error (or
   daemonically giving UB) for operations where some concrete value could
   give UB (e.g. division by zero, and perhaps control-flow choices).
   Nondeterministically choose particular concrete values only at
   external library calls.

   iv)  Freek WV: you can read and copy it, but get UB if you try to
   do anything else with it. 

   v) something else
   

## Case 2 - address never taken:

We could either keep UB or give a more constrained semantics,
e.g. that it must either (however the implementation prefers) be
identified at compile-time (with a diagnostic message) or give rise to
a non-silent run-time error or be treated as a concrete
allocation-time nondeterministic value.

Straw poll 5: For an uninitialised reads of a scalar non-character typed
object of automatic storage duration, for a type that does not (on the
platform in question) have trap representations, if the address is
never taken, should it be:

a. UB

b. either (however the implementation prefers) be identified at
    compile-time (with a diagnostic message) or give rise to a
    non-silent run-time error or be treated as a concrete
    allocation-time nondeterministic value.

c. either (however the implementation prefers) be reported
    as a compile-time or run-time error or be treated as a wobbly value.

d. some other semantics


  
