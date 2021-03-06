# Unitialised Reads v6: Summary

Peter Sewell, Jens Gustedt, Martin Uecker, Kayvan Memarian

2021-04-13

The semantics of uninitialised values and padding bytes in C has long been a vexed question: there are many conceivable semantics, there are conflicting demands and expectations, and the current standard text is not clear about all aspects.  This note summarises our view of the design space following recent discussions in WG14 and in the C memory object model.  To keep this concise, we focus only on the design options as we see them now, without examples or history.

As the existing standard (with its concepts of trap representation, unspecified value, and indeterminate value) is problematic in various respects (which we do not detail here), we focus on what the semantics _should be_, and especially on the constraints on it that arise from practice, rather than on what the current standard text says.

C and C++ should ideally be closely aligned for all this, but here we focus just on the C case.  We also don't discuss the ongoing LLVM work on freeze and poison.

- there are some cases where it has to be deemed an error to read a specific object representation, e.g. to let some implementations trap - these cases are now rare, but they include signalling NaNs and perhaps other exotic number formats.  JM: in C++ the hook for UB is the lvalue-to-rvalue conversion.

- there are some cases where it has to be deemed an error to operate on the result of a read of a specific object representation - primarily _Bool and floating-point cases - but where we have a free choice whether to deem it an error to read or write them

- it would be reasonable to require implementations to document the cases where they do each of the above

- in most cases, reading uninitialised values or padding should be deemed an error. We return below to what exactly that should mean, but first consider the exceptions.

- copying partially initialised structs has to be allowed (either by an explicit struct read&write, or implicitly as a struct function argument or return value).  JM: for C++ it's just UB. And normally you pass a pointer, not by value, so not a big problem.

- it's unclear whether copying partially initialised structs member-by-member has to be allowed. We tentatively assume not.  JM: in C++, no struct-as-a-whole case; it falls back to the member-by-member case. Unlikely to change soon.

- we see no programmer use-case for the current ISO address-taken exception (leaving representation-byte accesses aside), so we can remove that (though we may wish to check that whatever semantics we end up with admits Itanium NaT-like behaviour in case future architectures do that).  Though some of the consequences are arguably desirable: uninitialised chars being an error to read, except when doing representation-byte accesses on an address-taken object - which allows bytewise memcpy.

- reading the representation bytes of uninitialised automatic storage duration variables and malloc'd regions has to be allowed, eg to support library or user memcpy of partially initialised structs (deferring what one knows about the results of such reads for a moment)

    - aside: in ISO C, representation-byte accesses have to be at character types, but real code also relies on representation-byte accesses at larger types - this is likely rare, but we should support it in some way. There is also type-aliasing of matrices, relatively commonly.  E.g. looking at bits of doubles with an unsigned long. 

- reading the padding bytes of structs similarly has to be allowed, to support library or user memcpy

- reading uninitialised representation bytes and padding bytes is also necessary for other bytewise polymorphic operations: memcmp, marshalling, encryption, and hashing  (deferring what one knows about the results of such reads for a moment). It's not clear how generally these operations have to be supported, and we would like more data.  Atomic cmpxchg on large structs, implemented with locks, would do a memcmp/memcpy combination (in fact is described as such in the standard).    Supporting this more motivated in C than C++, because C++ can generate memberwise == easily, and work underway on reflection.

- for reads of uninitialised representation and padding bytes (they might not be treated identically, but we don't so far see any reason why not to), and of any other uninitialised value that one doesn't deem to be programmer errors, one could either:

    1. regard them as holding stable concrete values. This is inconsistent with current behaviour for some compilers,
	
	2. regard them as holding stable concrete values, but let implementations nondeterministically read either those or zero. This would let programmers control information flow via padding.  (In this model memcpy;memcmp could give false)

    3. regard them as holding wobbly values, but with a memcpy (or other bytewise read and write) as concretising them to a nondeterministically chosen concrete value in the target (we think this is ugly), or

    4. regard them as holding wobbly values that are propagated as wobbly values by memcpy (or other bytewise read and write), and then either:

		a. deem it to be a programmer error to operate on them in any way except writing them, or

        b. propagate wobbliness through operations (including conversions), except for UB where some concrete value would give UB (e.g. division by zero, and perhaps control-flow choices), nondeterministically picking concrete values only at external library calls (this is what Cerberus does at present)

    In the wobbly-value options, for padding bytes, the wobbliness of the source values could be either:
	
	i. intrinsic to the padding locations - uninitialisable, whatever happens, or

    ii. subject to being overwritten by explicit writes of non-wobbly values to padding, but reset to wobbly by preceding-adjacent or whole-struct writes.

- the point of the wobbly-value options is to admit implementation behaviour while giving stronger guarantees for programmers than just "any access of any uninitialised data is UB", for the cases where the latter is necessary.  One would not expect programmers to be intentionally manipulating wobbly values in many circumstances, and they can be hard to reason about. 

- the answers to these questions determine e.g. whether one can memcpy a partially initialised struct (perhaps containing padding) and then get a guarantee that a memcmp would compare equal.  It's unclear whether this is a hard requirement for the semantics.  Currently some tests suggest gcc may guarantee it, and similarly clang for copy-and-compare, but not always if there's also some arithmetic. 

- we think it's important for the programmer to have some way to sanitise padding, in cases where they have to exclude bad information flow. This rules out the option (i) of padding bytes being intrinsically wobbly. 

- all other cases (i.e., those that are neither representation-byte accesses, padding-byte accesses, or partially initialised struct member accesses) of reading uninitialised variables can be regarded as programmer errors

- for these, we could either:

     x. make them always UB

     y. make them, at the implementation's per-instance choice, either a compile-time or a runtime error (a trap or signal), or either
	 
	     p. an object-creation-time nondeterministic particular concrete value, or

	     q. a read-time nondeterministic particular concrete value, or

		 r. a wobbly value following 4a or 4b.
		 
         This is a bit like the current ISO semantics for conversion, 6.3.1.3.  This lets implementations detect and report errors wherever they can, while somewhat limiting "surprising" optimisations arising from UB.  (This approach could also be applied to some other UBs.)

     z. make them wobbly, with one of the above options 1-3. This wouldn't permit desirable implementation error reporting.

------------------


2021-04-22 meeting: Jens Gustedt, Vadim Zaliva, Jens Maurer, Freek Wiedijk, Kayvan Memarian, Miguel Ojeda, Martin Uecker, Peter Sewell

todo? 
ask cost of option 1  (wrt base and wrt zero-init (also for heap))







