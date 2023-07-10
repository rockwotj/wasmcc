# VMThread

VMThread is a small (but mighty!) library to run code on another stack. This allows for the ability to "pause"
code executing on a stack and jump to another. This style of concurrency is usually known as coroutines.

See the details in VMThread for more, but this is a plesant C++ friendly version of stack switching only by 
specifying a lambda.

## Meta

VMThread uses assembly and was inspired from [minicoro], which itself took assembly from [luajit].
Both are licensed under MIT, and minicoro does not require attribution. [luajit] has attribution in the 
files that specifically implements the assembly specific parts.

[minicoro]: https://github.com/edubart/minicoro
[luajit]: https://coco.luajit.org/

