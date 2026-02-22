# **Contributing to inLimbo**

> [!NOTE]
> 
> The main architecture and e2e flow doc is coming soon.
> 
> It is highly advised to give that a read before trying to
> contribute.
> 

The guide will keep it brief and informative here without taking too much of your time:

1. **Learn the project's build pipeline**:

Entine project's pipeline works like so:

Source tree structure:

```text
include/inlimbo
    <audio>
    <config>
    <mpris>
    <telemetry>
    <query>
    ...
    <frontend>
        <cmdline>
        ...

src/
    <audio>
    <config>
    <mpris>
    <telemetry>
    <query>
    ...
    <frontend>
        <cmdline>
        ...
```

The project uses `CMake` to setup and build the project, that is tightly knit into simple commands interfaced
by a `Makefile`

> [!IMPORTANT]
> 
> Run `make help` to see what all it can do.
> 

Understanding this will automatically help you understand the design flow of the project.

> [!NOTE]
>
> For understanding how frontends are registered and loaded and runtime
> check out [Frontend Interface](https://github.com/nots1dd/inLimbo/tree/develop/include/inlimbo/frontend/Interface.hpp)
> 
> The comments there should explain most of it.
> 

2. **Repo centric guidelines**:

- Directories are named in small letters, header/src files are named in `PascalCase`
- namespaces are in `lowercase` only (note that there is no global namespace)
- Classes are in `PascalCase` and methods are in `camelCase`
- Do not write `using namespace xx;` in header files (its obvious why)
- For variable decl you *can* freestyle it as long as it makes sense (depends on readability)
- Always use `include/inlimbo/InLimbo-Types.hpp` for custom aliases for better readability
- Use `Logger.hpp` (do not invoke `spdlog` directly)
- Always run `make fmt` before committing 
- Yes, there are **A LOT** of nested namespaces and yes it is a burden to write, but do follow it,
  I can understand stuff better that way.

- All source files except for `inLimbo.cpp` use `.cc` extension, all headers use `.hpp` extension
- No header guards, just use `#pragma once`
- Write useful comments only either as a block of info at the top (detailed), or single line 
  explanations above the line of code.

- Use `StackTrace.hpp` in a limited fashion, it already explodes in memory as the program executes over time.

Example:

Let us say you want to debug something in `telemetry`'s source code. Mark a singular function in the backend like so:

```cpp 
auto func(...)
{
    RECORD_FUNC_TO_BACKTRACE("telemetry::<>::func");
    ...
}

// this func is called in frontend or main context

/// namespace frontend::cmdline (example)

auto printToTerminal(...) -> void
{
    func(...); // invoked here
}
```

Now you can track whether `telemetry` is truly to blame after the program dumps the stack trace.

### **READING STACK TRACE**:

It is pretty intuitive but to put it simply it will look like so:

```text 
<MAIN>
entered main at LOC 13:1
...
  <frontend::cmdline::init()>
  entered at ...
  <frontend::cmdline::init()>
  exited
...
  <frontend::cmdline::printToTerminal()>
  entered ...
  <telemetry::func()>
  entered...
DUMP OVER (SIGSEGV signal caught)
```

In this above example, we see that a segfault (memory accessing problem) has occurred and stack is dumped.

We see that `telemetry::func()` was invoked and entered the stack but never exited cleanly. Investigate what 
happened there and work your way **DOWN** (take `telemetry::func` as the parent caller and see which logic
breaks under it).

I will add a screenshot maybe to highlight it better, but recording a function to backtrace simply
records whether a function with respect to the parent caller has entered and exited successfully or not.

It is powerful to use only in debugging to find out where the key problem maybe. The `RECORD_FUNC_TO_BACKTRACE`
macro does nothing in the release build.


3. **Commit guidelines**:

> [!IMPORTANT]
> 
> Always run `make fmt` before committing!
> 

Just follow `.gitmessage` style you are good to go.

4. **Writing a new frontend**:

Check out [Frontend Interface](https://github.com/nots1dd/inLimbo/tree/develop/include/inlimbo/frontend/Interface.hpp)

## **AI in contributions**

Since this is a raging topic nowadays I just wanted to mention, I personally dont care if you use AI to understand or try to modify the codebase.

But when submitting a patch or MR, do keep in mind that I will expect a clear and concise idea behind the change and why was it needed.
Pure AI slop that only makes the codebase more unreadable or does not achive anything will not be accepted.

That being said, I cannot say that I am better than AI and can do everything that it can. So if a patch/MR that is complete slop seems
to be functional and does the job is submitted, I will request a lot of reviews to make it work within the codebase and do some rigorous
testing on that feature.
