Uninitialised reads again, 2021-08-30
-------------------------------------

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

Straw poll 1: For an uninitialised reads of a scalar non-character typed
object of automatic storage duration, for a type that does not (on the
platform in question) have trap representations, if the address is
taken, should it be:

a. an allocation-time nondeterministic choice of a concrete value (stable if re-read) 

b. some flavour of wobbly value

c. some other semantics


If there was a majority for (b), then we should ask:

Straw poll 2: For an uninitialised read of a scalar non-character typed
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

   iv) something else
   

## Case 2 - address never taken:

We could either keep UB or give a more constrained semantics,
e.g. that it must either (however the implementation prefers) be
identified at compile-time (with a diagnostic message) or give rise to
a non-silent run-time error or be treated as a concrete
allocation-time nondeterministic value.

Straw poll 3: For an uninitialised reads of a scalar non-character typed
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


  
