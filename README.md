# ISSUES
When a memory bug is detected in one of the treads it won't report a non-zero
exit code while running the tests.

# A collection of libraries inspired by zig, and other stuff.

I like zig, but I really like writing C. I am starting with implementing
different allocator types. I don't know how the other parts of the library will
fit in, but they will certainly use the allocators. 

I want a series of standard collection types with nice interfaces. ArrayLists,
LinkedLists, value-based HashMaps, reference-based HashMaps.

I also want to provide template header files that use the X-macro pattern so
that users can easily make tagged unions, option types, automatic bit-flags,
and maybe even an ECS inspierd struct filled with optional fields.

I probably want a ui library. It will rely on a graphical drawing interface. It
will be a little different though. It is inspired from my experience with using
raygui. instead of making some sort of layout engine, I will provide a set of
useful functions to manipulate rectangles. I think this produces programs that
are easier to reason about, while still reducing needless duplication of code.
The workflow relies heavily on function-static variables, which are a c
superpower in this scenario. You will essentially start with some 'fixed'
rectangle (probably derived from the window size) and then perform splitting
operations on that rectangle. Even a dynamic split that creates a draggable
border, updating the local function-static variable (which is a normalized
float) to let you resize sections. Also, the interface will be like immediate
mode guis, with one exception from the way raygui works. The drawing operations
are deferred, and called in the reverse order. This way the first gui function
that captrues input can signal to the other function that input is captured,
and the drawing of the gui elements can reflect the priority of the gui
functions. Overlapping gui elements in raygui is the main painpoint imo.
