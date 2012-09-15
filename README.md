lame-obj-c
==========

An attempt at getting some object-orientedness and reference counting to C. It worked!

It was also an attempt to revisit programming in C by itself. I haven't really done it since school and it seemed like it is a good skill to keep around. I ended up emulating how I handle memory in Objective-C but it taught me how to implement the retain/release/autorelease semantics of manual reference counting so bonus.

I can compile it with gcc without any warnings like so:

`gcc -ansi main.c lame-obj-c.c`

I wouldn't use it in any production code. It was mainly made as a sort of exploratory exercise.