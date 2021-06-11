# Uninitialised Reads v7

Perhaps by focussing tightly on just the core uninitialised-read case
we can get some clarity and agreement.

First consider reads of scalar uninitialised variables at
non-character types of automatic storage duration (deferring
consideration of representation-byte reads, whole-struct reads,
padding, and allocated-storage-duration regions).

Such reads are normally (perhaps always?) a programmer error, and the
standard should let implementations report it where they can, either
statically or dynamically.

The current standard address-taken exception to this was introduced to
support Itanium NaT without impacting other users.  We are not aware
of real software use-cases for the exception (in this scalar
non-character-type case), and see no reason to keep it (in this case). 

We can let implementations report such errors either:

a) by making them UB

b) by explicitly permitting, at the implementation's per-instance
choice, either a compile-time error, a non-silent runtime error, or
some other stronger-than-UB behaviour

The first is simplest, gives the most implementation flexibility, and
is in the style of most of the rest of the current standard.  The
second more clearly expresses our intent, and prevents (e.g.)
implementations reasoning that control-flow paths on which this occurs
are not reachable - thereby hopefully providing more predictable
behaviour for users.

(The second case is somewhat like the existing 6.3.1.3 semantics for
conversion, "either the result is implementation-defined or an
implementation-defined signal is raised".)

A straw poll at the last WG14 meeting was roughly evenly split (10 vs
8).  Our 2015 survey of C expert expectations was also roughly evenly
split (139 vs 112), as was a straw poll at EuroLLVM.  The survey
comments suggest that some (but perhaps not much) real code depends on
some stronger-than-UB semantics.

We advocate (b). 

Given (b), what could the stronger-than-UB semantics (in the case that
the implementation does not report an error) be?

There are some cases where it has to be deemed an error to read a
specific object representation, e.g. to let some implementations
trap - these cases are now rare, but they include signalling NaNs and
perhaps other exotic number formats. The explicit licence to report an
error covers those.

There are some cases where it has to be deemed an error to operate on
the result of a read of a specific object representation - primarily
_Bool and floating-point cases - but where we have a free choice
whether to deem it an error to read or write them.  We'll return to
those in a moment.

Then the options are, in increasing looseness:

b.0) a default (zero) particular concrete value. 

b.1) an object-creation-time nondeterministic particular concrete value.

b.2) an abstract-machine-read-time nondeterministic particular
concrete value (i.e., a wobbly value which itself remains wobbly, but
reads of which are nondeterministic concrete values).

b.3) propagate uninitialisedness through stores (with the target
becoming uninitialised in the same way), but make a use-time
nondeterministic choice of a particular concrete value whenever it is
used in some operation (including arithmetic).

b.4) propagate uninitialisedness through stores and all operations
(including arithmetic), except daemonically giving UB for operations
where some concrete value could give UB (e.g. division by zero, and
perhaps control-flow choices).  Nondeterministically choose particular
concrete values only at external library calls.

b.5) propagate uninitialisedness through stores, but make it UB to
operate on the uninitialised value in any other way.


What criteria do we have for choosing between these?

(b.0) and (b.1) are the most predictable.

(b.2-5), and perhaps especially (b.3-5), accommodate compiler SSA optimisations

(recall that Nuno remarked that the code-size effect of requiring a concrete read was deemed significant)

The later options (b.2-5) are not giving much useful strength to the
programmer beyond (a), except that they are all making it defined
behaviour to copy an uninitialised or partially initialised struct
member-by-member.

