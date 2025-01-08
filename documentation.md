## Array

* `join(delimiter: String = ", ", prefix: String = "[", suffix: String = "]"): String`: join all elements into one string with the specified `delimiter`, `prefix`, and `suffix`. 

* `string_join(delimiter: String = ", ", prefix: String = "[", suffix: String = "]"): String `: similar to `join()`, but is faster if all elements in the array are already string. Throw `TypeError` is any element is not a string. 

* `subarray(start: Int = 0, end: Int = this.length, new_length: Int = end - start): Array `: return a subarray
* `iterator(): Iterator`
* `static copy(src: Array, dest: Array, src_index: Int, dest_index: Int, length: Int)`: Similar to `memcpy` in c. If `src` and `dest` are the same array, use `memmove` instead. 


## Map

* `get_or(key, default_value): Value`: if `key` does not exist in the map, return  `default_value`, otherwise return the value associated with `key`. 
* `delete(key): Value`: delete a key-value pair and return the value.
* `iterator(): Iterator`: Each time, the iterator's returns an array of length 2 `key, value` 

## String

* `substring(start: Int, end: Int): String`: return a new substring.
* `replace(start: Int, end: Int, insert: String): String`: return a new string which is equal to the old string with the part `[start, end)` replaced by the `insert` string. 
* `iterator(): Iterator`: Each time, the iterator returns a character.
* `static concat(values...): String`: concatenate values into one string. The values can be of any types. 

## Class

* `subclass_of(c: Class): Bool`: true if this class is a subclass of `c` (or `this == c`). Otherwise, return false. 

## File

The File class only supports text io. 

* `init(path: String, mode: String)`: open a file at specified path with the specified mode
* `read_line(): String`: return the next line. If end of file, return nil
* `read_chars(limit: Int): String`: return next `limit` characters. If end of file, return nil
* `write(content: String, new_line: Bool = false)`: write `content` into the file. Append a newline if `new_line` is true.
* `tell(): Int`: return the position of the file pointer. Similar to `fseek()` in c and `seek()` in python.
* `seek(offset: Int, whence: Int = 0)`: move the file pointer. Similar to `fseek()` in c and `seek()` in python.
* `close()`: close the file
* `iterator(): Iterator`: Each time, the iterator moves the file pointer and returns next line. 

## Vector

* `init(elements...)`: initialize the vector with given elements. 
* `insert(element, index: Int)`: insert an element to a specific index
* `delete(index: Int): Value`: delete and return the value at a specific index
* `get(index: Int): Value`
* `set(index: Int, element)`: set the element at an existing index. Cannot use this function to add new elements.
* `append(element)`: add to the end
* `pop(): Value`: remove and return the last element. 
* `iterator(): Iterator`
* `size`: number of elements in the vector. 

## Builtin

* `fun range(start, end = nil, step = 1): Iterable`
* `fun benchmark(task: Function)`
* `fun foreach(iterable, handler: Function)`
* `fun enum(iterable): Iterable`
* `endl: String`: a string of one newline character. This is how newline is explicitly specified since lox does not support `"\n"` as newline. 
* `stdin: File`
* `stdout: File`
* `stderr: File`
