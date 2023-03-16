# SHA 256 hashing library

This library has one and only one purpose: provide an implementation of a sha256 hashing algorithm.

It's very simple to use, just link provided `CMakeLists.txt` library called `sha256` with your target and include `sha256/sha256.h`. Then you can:

```cpp
void my_important_business_logic() {
	// select block to hash:
	sha256::block block_to_hash = { buffer, size };

	// call hash(...) function to do the job:
	sha256::sha256_t output_hash =
	    sha256::hash(block_to_hash);
		
	// use retrieved hash as you wish, it's
	// actually just an array of HASH_SIZE (which is 8)
	// uint32_t integers wrapped in std::array:
	
	uint32_t one_eighth_of_sha256_hash = output_hash[0];

	// use it as you wish!
}
```

The code implementing *sha256* should be good enough so you can understand what's going on if you want to (at least it's the best I could come up with, and the best I seen in the internet)

It's also compliant (I tried to implement standard closely), for example:
```sh
 ~ $ echo -ne "assembly" | sha256sum
e020bdf568793dee747a220086c5e53c8b3af5abd959383a31297f303577efe6  -
 ~ $ echo -ne "assembly" | ./mine_implementation
e020bdf568793dee747a220086c5e53c8b3af5abd959383a31297f303577efe6
```

And it's pretty quick! For example, hashing string of length 64 10000 times over compered to `sha256sum` gives (I create a process every time, so it's a lot quicker when used with an API and not from a `bash` script, but still):

+ `10.3s` for `sha256sum`
+ `11.8s` for my implementation

Yeah, it's slower, but not by a big margin (still noticeable and important though)! And what would you expect, `sha256sum` has a lot more man power backing it up!
