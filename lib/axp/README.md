# AXP :: C++ Support Library

## Basic exception handling

Standard `C++` exceptions (e.g. `runtime_exception`, `logic_exception`, ...) are very minimal and only accept a static "what" string and do not tell you anything about the location where the exception actually occurred.

This library, among everything else, provides `axp::exception`, which should address this problem:
+ On creation it stores `axp::source_location` associated with the place at which `axp::exception` was created, you don't need to pass anything or use macros to use this feature, thanks to C++20

```cpp
void some_function_that_might_fail() {
	throw axp::exception { "You have failed me!" };
	//    ^ "what" message will include information about
	//       this exact point where exception was created
}
```

**Beware, to achieve this behavior, this library uses `__builtins` that tell in which point of the program they were called, but now these are only reliably supported in Clang.** Hopefully we will reimplement it with `std::source_location` when it's reliably available in both **Clang** and **GCC**.

+ Also, it supports not just static strings, but a `printf`-like interface that allows you to easily include any helpful information about the error you might have. `axp` used to implement these itself, but now it just uses `fmt` and supports whatever `fmt` supports:

```cpp
throw axp::exception { "Hello, {}!\n{:>13}", "World", "==>" };
// format spec, substitutes    ^~   ^~~~~~
// to "World!" in the string        |
//                                  |
//                                  these specifier allows to
//                                  align "==>" left so it
//                                  fills 13 characters total
```

See [fmt documentation](fmt.dev) for list of all available specifiers, treat `axp::exception` the same way you would `fmt::format(format_string, ...)`, which is exactly what it does under the hood.

## Nested exceptions

Sometimes exceptions (and errors in general) have different levels and can nest.

For example, imagine you have an app for a big bank, which happens to have an embedded game (let's say Tetris). This game does some low level thing like basic networking to retrieve the highest score from Tetris server.

But, unfortunately, this attempt is unsuccessful, the server is deemed unreachable. For Tetris this is an exceptional case, it cannot continue its hassle and throws an exception telling "Server `173.194.222.102` is unreachable, here is a detailed network dump: ..."

This message is alright, it tells what happened, no joke. Yet it's completely useless to the user and, likely, to the bank too. It's a lot better to tell user "Tetris failed, couldn't connect to its server" and assure him that the bank is still operational and everything is going to be alright.

But at the same time we don't want to lose more detailed information, it's still very useful to a professional who will hopefully debug and fix this error.

How to get the best out of both worlds? We need some way to add new information to an exception further up the stack, yet preserve all underlying information. There is `std::throw_with_nested` just for this purpose:

```cpp
void my_buisness_logic() {
	try {
		function_that_might_fail();
	} catch (...) {
		std::throw_with_nested(axp::exception {
			"I couldn't do important logic stuff!"
		});
	}
}
```

**But** standard's committee again messes up, because `what()` of the nested expression tells nothing about it's nesting. They might say: Of course it doesn't, it can literally warp anything, even a thrown `int`, which doesn't even have `what()`, so to access the underlying exception you need to figure out that it's nested, and then yourself write weird-looking recursive `std::throw...try...catch` thing. And, yeah, I see where they're coming from, yet it makes this system completely useless for any *serious* use.


What to do? `axp::nested_exception` is to rescue! Let's see how the same logic would look like:

```cpp
void my_buisness_logic() {
	try {
		function_that_might_fail();
	} catch (...) {
		throw axp::nested_exception { "I couldn't do important logic stuff!" };
	}
}
```

`axp::nested_exception` will automatically capture the current exception in its constructor, and wrap `axp::exception` for a new message. It supports the same quirks and perks of `axp::exception` since it's literally it's inheritor, so `fmt` format strings and automatic location capture are here for you! 

It even supports wrapping custom exceptions if you need it. Let us create one at first:

```cpp 
// i'm sorry for all formatted_types... thing, in ideal world,
// there's shouldn't be a reason for them to be an
// axp::exceptions template parameterization, but it's cpp...

// and to my knowledge that's the only way now to get axp::exception's
// variadic template passed to fmt::format and axp::source_location
// default argument to automatically pick up location

// but, maybe, i will come up with something and change it, hopefully
template <typename... formatted_types>
class my_custom_exception: public axp::exception<formatted_types...> {
public:
	// let's inherit axp::exception's constructor, so we don't
	// have to reimplement it ourselfs:
	using axp::exception<formatted_types...>::exception;
};
```

We now have a new exception ready for real-life battle:
```cpp
throw my_custom_exception { "Same fmt:: string{}\n", "!" };
```

You can even rethrow it nested by using `axp::nested_exception` with explicit parameterization:
```cpp
throw axp::nested_exception<my_custom_exception> { "You're a failure!" };
```

**Important:** Your new exception should be a template parametrized on format types for this to work, because `axp::nested_exception` uses them to inherit from `axp::exception` and accepts this specific `template template` (with default being, as you might have guessed, `axp::exception`)

**Note:** creating marker exceptions, just like `my_custom_exception`, but with a different name, can be useful to distinguish them in catch blocks from a general `axp::exception` and therefore have a finer control of which exceptions to catch, and which not.

Therefore, for convenience purposes, I created macro:
```cpp
AXP_CREATE_NEW_EXCEPTION(my_custom_exception)
```

Which does *exactly* what I did to create `my_custom_exception` the first time, but all in a single line. You're encouraged to use it if you need to created new marker exceptions.
