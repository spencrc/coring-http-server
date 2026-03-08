# Notes On Coroutines

## Trivial Awaitables Behaviour
### With `initial_suspend`
When using `suspend_always` for `initial_suspend`, the coroutine's will **start suspended**. No code inside it will run until it is resumed!

When using `suspend_never` for `initial_suspend`, the coroutine will **immediately** run the code inside it.
Of course, it will suspend if you call `co_yield` or `co_await`.

### With `final_suspend`
When using `suspend_always` for `final_suspend`, it will mean the coroutine will suspend when exiting the scope. 
This means the coroutine frame (heap allocation) cannot be freed.
As a result, manual resource cleanup will need to be defined (likely through a destructor).

**Important to remember:** If you're using a destructor, then you **must not** discard the return value. 
If you discard it, then it will never destruct and the coroutine frame **will** leak.

When using `suspend_never` for `final_suspend`, the coroutine will destroy itself **as soon as it finishes**! Any data stored inside the coroutine frame will be immediately lost.