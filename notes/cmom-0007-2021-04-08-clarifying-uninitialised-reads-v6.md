# Unitialised Reads v6: post-WG14 summary

- there are some cases where it has to be deemed an error to read a specific object representation, e.g. to let some implementations trap - these cases are now rare, but they include signalling NaNs and perhaps other exotic number formats.

- there are some cases where it has to be deemed an error to operate on a read specific object representation - primarily _Bool and floating-point cases - but where we have a free choice whether to deem it an error to read or write them

- it would be reasonable to require implementations to document the cases where they do each of the above

- copying partially initialised structs has to be allowed (either by an explicit struct read&write, or implicitly as a struct function argument or return value).  It's unclear whether copying partially initialised structs member-by-member has to be allowed.  

- reading the representation bytes of uninitialised automatic storage duration variables and malloc'd regions has to be allowed, eg to support library or user memcpy of partially initialised structs (deferring what one knows about the results of such reads for a moment)

    - in ISO C, representation-byte accesses have to be at character types, but real code also relies on representation-byte accesses at larger types - this is likely rare, but we should support it in some way. There is also type-aliasing of matrices, relatively commonly. 

- reading the padding bytes of structs has to be allowed, eg to support (in some cases, not always) polymorphic bytewise operations such as copying, and perhaps also memcmp, marshalling, encryption, and hashing  (deferring what one knows about the results of such reads for a moment). It's not clear how much the latter operations have to be supported. Jens+Martin think yes; Peter is skeptical. We would like more data.  Atomic cmpxchg on large structs, implemented with locks, would do such a memcmp/memcpy combination (in fact is described as such in the standard). 

- for these representation and padding bytes (they might not be treated identically, but let's suppose they are), one could either:

    - regard them as holding stable concrete values - but this is likely inconsistent with current behaviour for some compilers, or
	- regard them as holding wobbly values, but with a memcpy (or other bytewise read and write) as concretising them to a nondeterministically chosen concrete value in the target (this isn't sufficient for the memcpy/memcmp example, and we think it's ugly in any case), or
    - regard them as holding wobbly values that are propagated as wobbly values by memcpy (or other bytewise read and write), and then either:

        - propagate wobbliness through operations (including conversions), except for UB where some concrete value would give UB (e.g. division by zero) (this is what Cerberus does at present)
		- regard it as a programmer error (expressed either with UB or otherwise as below) to operate on them 

    - in the last two cases, the wobbliness of the source values could be either:
	
	    - intrinsic to the padding locations - uninitialisable, whatever happens
		- subject to being overwritten by explicit writes of non-wobbly values to padding, but reset to wobbly by preceding-adjacent or whole-struct writes.

- the answers to these questions determine e.g. whether one can memcpy a partially initialised struct (perhaps with padding) and then get a guarantee that a memcmp would compare equal.  Currently gcc seems to guarantee that, and so does clang head (though not older versions) for copy-and-compare, but not always if there's also some arithmetic. 

- we think it's important for the programmer to have some way to sanitise padding, in cases where they have to exclude bad information flow. This rules out the option of padding bytes being intrinsically wobbly. 

- we see no programmer use-case for the current ISO address-taken exception, so we can remove that (though we may wish to check that whatever semantics we end up with admits Itanium NaT-like behaviour in case future architectures do that).  

- all other cases of reading uninitialised variables can be regarded as programmer errors

- for these, we could either:

      - make them always UB (which is classically what ISO C would do)
      - make them, at the implementation's per-instance choice, either a compile-time or a runtime error (a trap), or a use-time nondeterministic particular concrete value. This is a bit like the current ISO semantics for conversion, 6.3.1.3.  This lets implementations detect and report errors wherever they can, while somewhat limiting "surprising" optimisations arising from UB.  (This approach could also be applied to some other UBs.)
      - make them always an unspecified value (with the above options)
      




