# A collection of libraries inspired by zig

I like zig, but I really like writing C. I am starting with implementing
different allocator types. I don't know how the other parts of the library will
fit in, but they will certainly use the allocators. 

I want a series of standard collection types with nice interfaces. ArrayLists,
LinkedLists, value-based HashMaps, reference-based HashMaps.

I also want to provide snippets for generating safe bit-flags and tagged
unions. This will be done with the X-macro pattern. As well as snippets I want
some sort of explanation of why the snippets are made the way they are, and
what value it provides to follow those patterns.

I probably want a ui library. It too wil rely on a graphical drawing interface.
It will be a little different though. It is inspired from my experience with
using raygui. instead of making some sort of layout engine, I will provide a
set of useful functions to manipulate rectangles. I think this produces
programs that are easier to reason about, while still reducing needless
duplication of code. The workflow relies heavily on function-static variables,
which are a c superpower in this scenario. You will essentially start with some
'fixed' rectangle (probably derived from the window size) and then perform
splitting operations on that rectangle. Even a dynamic split that creates a
draggable border, updating the local function-static variable (which is a
normalized float) to let you resize sections.
