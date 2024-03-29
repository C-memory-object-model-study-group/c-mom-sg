Clarifying uninitialised reads, v10
-------------------------------------

Peter Sewell,  2021-09-01

## Introduction

There are many potential semantics for uninitialised reads. 
From many previous discussions, including the 2021-08-31 WG14
discussion and straw polls in the C memory object model study group
note
<https://github.com/C-memory-object-model-study-group/c-mom-sg/blob/master/notes/cmom-0010-2021-08-29-clarifying-uninitialised-reads-v9.md>,
it seems clear that:

1. there is no clear preference within WG14 for any single alternative, and
2. in existing practice, compilers provide several alternatives, controlled by various compiler options.

We therefore fall back from our previous attempts to identify a
generally acceptable single alternative, and instead propose to define
several alternatives, so that compilers can document, in terms of
these, which semantics they provide in what circumstances.

The potential downside of defining multiple alternatives is
fragmentation and confusion, but both already exist in practice.

Recall that these are proposals (for C) for the draft Technical
Specification TS 6010, which is being produced to establish more
consensus and practical experience for a better-defined memory object
model, in advance of possible later adoption into the C standard.

This should be harmonised with the C++ state and plans; that mostly 
remains to be done.

It would be useful at this point to collect the state of existing practice:
for each major compiler, to identify which of the following alternatives it
implements (or aims to implement), under which circumstances - and also to 
identify whether the following alternatives suffice. 

## The 2021-08-31 WG14 discussion and straw polls

We first asked, for the address-taken case: 

<em>
Straw poll 1: For an uninitialised read of a scalar non-character typed
object of automatic storage duration, for a type that does not (on the
platform in question) have trap representations, if the address is
taken, which of the following should be reasonable options? 

Bearing in mind that if there are several good candidates, the TS might identify 
a set of them that compilers could adopt, not just a single option.

You can vote for as many as you like.

- a: an allocation-time nondeterministic choice of a concrete value (stable if re-read) 
- b: some flavour of wobbly value
- c.1: plain UB 
- c.2: at the implementations's per-instance choice: diagnosed compile-time or run-time error or (a)
- c.3: at the implementations's per-instance choice: diagnosed compile-time or run-time error or some flavour of wobbly value
- d:  some other semantics

</em>

|     |   yes  |   no  | abstain |
|:----|-------:|------:|--------:|
|a    |     18 |     3 |       3 |
|b    |     12 |    10 |       1 |
|c.1  |      9 |    12 |       2 |
|c.2  |     16 |     5 |       2 |
|c.3  |      9 |    14 |       0 |
|d    |      2 |    12 |       9 |

We then asked for people's single preference among these options, and later also for the preference in the address-never-taken case

|     |   preference (address-taken)|                         |   preference (address-never-taken)|
|:----|-----------------------------|------------------------:|----------------------------------:|
|     |                             | same as address-taken:  |     11                            |
|a    |   5 		 				|    					  |		0							  |
|b    |   1 		 				|						  |		0							  |
|c.1  |   7 		 				|						  |		5							  |
|c.2  |   9 		 				|						  |		7							  |
|c.3  |   2 		 				|						  |		0							  |
|d    |   0 		 				|                         |     0                             |


Our main conclusion from this is that multiple alternatives receive significant yes votes in the first vote (identifying "reasonable" alternatives), that no alternative gets even a majority in the preferences votes, that a, c.1, and c.2 are the main preferences, and that d has no preference votes (hence these are, for WG14 at the time, not missing any major alternatives). 

The wobbly-value alternatives, b and c.3, have rather low support in the preference votes, perhaps indicating that people feel these provide little of use (to programmers or compilers) beyond UB.   For the purposes of these polls, the semantics (which at least one person favoured) in which one can read and write an uninitialised value but with UB on any other operation was included in the wobbly-value bucket, and we didn't go into detailed votes among the many different flavours of wobbly values. 

It's also clear from earlier discussion that two other semantics are sometimes provided by existing compilers and valued in practice: automatic zero initalisation, and automatic initialisation with some sentinel value for error detection. 

The votes for c.2, which aims to allow implementation error reporting whenever the implementation can identify an uninitialised read, but a tolerably predictable semantics otherwise, indicate significant but not overwhelming support for providing such guarantees as an alternative to plain UB. 

The vote for the address-never-taken case was evenly split between those who thought it should be the same as the address-taken case (whatever that is), and those who picked c.1 and c.2.

## The proposed alternatives

That leads us to these alternatives:

- a: an allocation-time nondeterministic choice of a concrete value (stable if re-read) 
- c.1: plain UB 
- c.2: at the implementations's per-instance choice: diagnosed compile-time or run-time error or (a)
- e: uninitialised values can be read and written (with the target becoming uninitialised), but give UB if operated on in any other way
- f.1: automatic zero initialisation
- f.2: automatic sentinel-value initialisation

The proposal would be to make it implementation-defined which
alternative(s) a compiler provides, in which circumstances - depending
on compiler options, on the type, and on whether the address is ever
taken.

Does that suffice to describe existing practice? 

I.e., can the existing major compiler teams describe the behaviour
their current implementations provide (or aim to provide) in terms of
these?


