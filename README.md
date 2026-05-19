# what is the goal?
- A standard library inspired by languages like zig, odin and rust.
- Single header, multiple source files, and no build step.

# current and coming features

- [x] allocators
- [x] dynamic arrays
- [x] simple cli arg parsing
- [-] functions to build parsers
- [-] string builder
- [ ] hashmap
- [ ] easing functions
- [ ] simple gui engine

# example

## allocators and dynamic arrays
```c
#include <stdio.h>
#include <stdbool.h>

#define CIG_IMPL
#include "cig/cig.h"

int main(void) {

    with_borrow(allocator) {
        // initial capacity is optional
        int *ns = make_arr(int, allocator, .initial_capacity=10);

        for (int i = 0; i < 100; i++) {
            // the array grows as the capacity is reached
            arr_append(ns, i);
        }

        for (int i = 0; i < 10; i++) {
            int last = arr_pop(ns);
            printf("last value was %d\n", last);
        }

        for (int i = 0; i < arr_len(ns); i++) {
            printf("value of ns[%d] is %d\n", i, ns[i]);
        }

        // you can break out of a with_borrow scope
        break;

        printf("this is never printed\n");

        // WARNING: returning from within a with_borrow
        // scope will cause a memory leak
        return;
    }
    // everything allocated by the borrow allocator is
    // freed here, and the borrow allocator is also out of
    // scope.


    // you can also make the borrow allocator like this:
    allocator_t backing = borrow_allocator_create();

    // this arena allocator is slightly clever. You assign
    // an initial size of the memory block, but uses a
    // backing arena to allow you to allocate more than
    // that. Whenever allocator_reset is called it figures
    // out how much was allocated outside the big chunk of
    // memory, frees everything and allocates a bigger
    // block. So it's size will move towards what you
    // happen to use in the game loop. It assumes that it
    // is the owner of the given backing allocator, and it
    // should probably be a borrow_allocator.
    allocator_t arena_allocator = arena_allocator_create(backing, 100 * MB);

    // Imagine a game loop
    while (true) {
        // every time this resets, assuming the backing
        // memory chunk doesn't need to grow, this
        // operation is very fast.
        allocator_reset(arena_allocator);

        float *fs = make_arr(float, arena_allocator);
        arr_append(fs, 1.1);
        arr_append(fs, 1.2);
        arr_append(fs, 1.3);

        break;
    }

    allocator_reset(backing);
    return 0;
}
```

## cli
```c
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define CIG_IMPL
#include "cig/cig.h"

int main(int argc, const char **argv) {
    args_t args = cli_make_args(argc, argv);

    if (cli_command(&args, "say-foo")) {
        printf("foo\n");
    } else
    if (cli_command(&args, "say-bar")) {
        printf("bar\n");
    } else
    if (cli_command(&args), "greet") {
        bool do_nothing = cli_bool(args, "--do-nothing");
        const char *name = cli_req_str(args, "--name");
        if (args.help) exit(0);

        if (do_nothing) {
            return;
        }

        printf("Hello %s!\n", name);
    } else {
        printf("use the -h or --help flag to see available commands\n");
    }
}

```

