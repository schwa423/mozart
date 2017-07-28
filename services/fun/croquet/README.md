# Croquet

TODO(MZ-??): unimplemented.

The Croquet service will be a modern instantiation of the Croquet architecture,
a simple but flexible and performant basis for experimenting with multi-device
scenarios, both local and wide-area.

From the start, Croquet was deeply influenced by the object-capability security
model, and targeted at supporting networked VR experiences.  It is particularly
well-suited for the sort of low-latency interactions that the Ledger struggles
with (and conversely, the Ledger is strong in areas where Croquet is weak: the
two are complementary).

## Background reading:

"Croquet - a collaboration system architecture" - Smith, Kay, Raab and
Reed, C5 2003.  Describes the vision, but necessarily short on the technical
details that evolved over the next 5 years.

"Robust Composition: Towards a Unified Approach to Access Control".  Mark
Miller's seminal thesis, which coins the term "object-capability security".
A recommended read for any Fuchsia engineer with an interest in multi-device
module interaction!
