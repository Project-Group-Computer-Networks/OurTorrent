# Custom Bencoder

A small, single-header C++ serialization library for [bencoded](https://en.wikipedia.org/wiki/Bencode) data.

The library supports serializing and deserializing bencoded data with a simple API. It utilizes smart pointers, STL containers, templates, and concepts for efficient memory management and type safety.

**This was written to support [OurTorrent](https://github.com/Project-Group-Computer-Networks/OurTorrent) done as a group project for Computer Networks course.**

It follows the specifications in the [standard](http://bittorrent.org/beps/bep_0003.html).

## Requirements

This library has no external dependencies and only requires a C++20 compiler.

It has been tested on [GCC](https://gcc.gnu.org/) 10+ and [Clang](http://clang.llvm.org/) 10+. (Remember to compile with -std=c++20 flag as and when required depending on the compiler version.)

## Installation

Since custom_bencoder is a single-file, header-only library, just
copy `include/custom_bencoder.h` to your destination of choice either directly or after cloning the repository.

## Usage

Four types can be created : `bencode_integer`, `bencode_string`, `bencode_list`, `bencode_dict`. The list and dict store unique pointers to their contained objects.

### bencode_integer: 

**API:**

- Construct using default or parametrized (param = value) ctors
- Assignment operator for assigning values
- Default copy and move operations
- `get()` : Returns the value stored by the variable.
- `encode()` : Returns a string of bencoded data from stored value.
- `encode_n_dump(std::ostream)` : Outputs the bencoded data on the passed output stream
- `decode(std::string, std::string::const_iterator)` : Deserializes bencoded string (whose beginning is pointed to by the const_iterator) and extracts the integer data.
- `get_as_str()` : Returns the stored integer value as a string.

`struct bencoding::string_subs` can be used for conveniently passing a string and its beginning iterator to the methods. 

**Example:**

```cpp
#include "custom_bencoder.h"

int main()
{
    bencoding::bencode_integer bInt = 7;
    bInt.encode_n_dump(std::cout); // prints i7e
    std::cout << "\n";
    bInt = 999;
    std::cout << bInt.encode() << "\n"; // prints i999e
    bencoding::string_subs sa("i56e"); // for convenience
    bInt.decode(sa.str, sa.citer);
    std::cout << bInt.get() << "\n"; // prints 56
    return 0;
}
```

### bencode_string:

**API:**

- Default and parametrized ctors to construct bencode_string from `std::string` or `char *`.
- Default copy and move operations.
- Assignment operator to assign another `std::string` or `char *`.
- Standard iterator interface of `begin()`, `end()`, `cbegin()` and `cend()`.
- `size()` to return bencode_string size.
- `get()` : Returns the value stored by the variable.
- `encode()` : Returns a string of bencoded data from stored value.
- `encode_n_dump(std::ostream)` : Outputs the bencoded data on the passed output stream.
- `decode(std::string, std::string::const_iterator)` : Deserializes bencoded string (whose beginning is pointed to by the const_iterator) and extracts the string data.
- `get_as_str()` : Returns the stored string.

**Example:**

```cpp
#include "custom_bencoder.h"

int main()
{
    bencoding::bencode_string bs("hello");
    std::cout << bs.encode() << "\n"; // prints 5:hello
    bencoding::bencode_string b = "bye";
    b.encode_n_dump(std::cout); // prints 3:bye
    std::cout << "\n";
    bs = "ok";
    std::cout << bs.encode() << "\n"; // prints 2:ok
    bencoding::string_subs sa(bs.encode());
    b.decode(sa.str, sa.citer);
    std::cout << b.get() << "\n"; // prints ok
    return 0;
}
```

### bencode_list:

**API:**

- Default ctor for constructing an empty list and a ctor which allocates a list of given size.
- Default copy and move operations.
- Standard iterator interface of `begin()`, `end()`, `cbegin()` and `cend()`.
- Subscript operator `[]` is overloaded and behaves similar to that of `std::vector`.
- `push_back()` accepts any of the data types and inserts it to end of list.

Note : bencode_list and bencode_dict need to be passed as rvalues (`std::move`) since they store unique pointers. Check example for clarity.

- `pop_back()` removes last element from list.
- `size()` to return bencode_list size.
- `encode()` : Returns a string of bencoded data from stored list of values.
- `encode_n_dump(std::ostream)` : Outputs the bencoded data on the passed output stream.
- `decode(std::string, std::string::const_iterator)` : Deserializes bencoded string (whose beginning is pointed to by the const_iterator) and extracts the data into the list. Any prior data is overwritten.
- `get_as_str()` : Returns the stored list encoded as a string.

**Examples:**

1.

```cpp
int main()
{
    bencoding::bencode_list ls;
    bencoding::bencode_integer a(8);
    ls.push_back<bencoding::bencode_integer>(a);
    ls.push_back<bencoding::bencode_integer>(bencoding::bencode_integer(856));
    ls.push_back<bencoding::bencode_integer>(777);
    ls.push_back<bencoding::bencode_string>("bye");
    ls.encode_n_dump(std::cout); // prints li8ei856ei777e3:byee
    return 0;
}
```

2.

```cpp
#include "custom_bencoder.h"

int main()
{
    bencoding::bencode_list bl(5);
    bl[0] = std::make_unique<bencoding::bencode_integer>(7);
    std::cout << bl[0]->encode() << "\n"; // prints i7e
    bl[1] = std::make_unique<bencoding::bencode_string>("hiaf");
    bencoding::bencode_integer c = 999;
    bl[2] = std::make_unique<bencoding::bencode_integer>(c);
    std::cout << bl.encode() << "\n"; // prints li7e4:hiafi999ee
    bencoding::bencode_string str("ok");
    bl[2] = std::make_unique<bencoding::bencode_string>(str);
    std::cout << bl.encode() << "\n"; // prints li7e4:hiaf2:oke
    return 0;
}
```

3.

```cpp
#include "custom_bencoder.h"

int main()
{
    bencoding::bencode_list ls;
    ls.push_back<bencoding::bencode_integer>(777);
    ls.push_back<bencoding::bencode_string>("bye");
    bencoding::bencode_dict bd;
    bd.insert_or_assign<bencoding::bencode_string>("key", "value");

    bencoding::bencode_list bl;
    // bl.push_back<bencoding::bencode_list>(ls); // this is incorrect
    // bl.push_back<bencoding::bencode_dict>(bd); // this is incorrect
    // this is the right way for lists and dicts
    bl.push_back<bencoding::bencode_list>(std::move(ls));
    bl.push_back<bencoding::bencode_dict>(std::move(bd));
    // You are NOT supposed to use ls or bd after moving them.

    bl.encode_n_dump(std::cout); // prints lli777e3:byeed3:key5:valueee
    return 0;
}
```

### bencode_dict:

The dict will have `bencode_string` as keys arranged in lexicographical order and values as unique pointers pointing to respective data types.

**API:**

- Default ctor for constructing an empty dict.
- Standard iterator interface of `begin()`, `end()`, `cbegin()` and `cend()`.
- Subscript operator `[]` is overloaded and behaves similar to that of `std::map`.
- `insert_or_assign()` accepts a key and any of the data types as value. If the key already exists then it is assigned the value otherwise the (key, value) pair is inserted into the dict.

Note : bencode_list and bencode_dict need to be passed as rvalues (`std::move`) since they store unique pointers. Check example for clarity.

- `insert` accepts a key and a **unique pointer** to any of the data types as value. The (key, value) pair is inserted into the dict only if the dict does not already contain that key.

- erase() has two overloads : One accepts a key and erases the corresponding (key, value) pair from the dict. Other accepts an iterator to the dict and erases the corresponding node.
- find() accepts a key and returns an iterator pointing to the node containing (key, value) of the key.
- `size()` to return bencode_dict size.
- `encode()` : Returns a string of bencoded data from stored dict of keys and values.
- `encode_n_dump(std::ostream)` : Outputs the bencoded data on the passed output stream.
- `decode(std::string, std::string::const_iterator)` : Deserializes bencoded string (whose beginning is pointed to by the const_iterator) and extracts the data into the dict. Any prior data is overwritten.
- `get_as_str()` : Returns the stored dict encoded as a string.

**Examples:**

1.

```cpp
#include "custom_bencoder.h"

int main()
{
    bencoding::bencode_dict bd;
    bd.insert_or_assign<bencoding::bencode_integer>("key1", 42);
    std::cout << bd["key1"]->encode() << "\n"; // prints i42e
    bd.insert("key2", std::make_unique<bencoding::bencode_string>("ok bye"));
    bd["abc"] = std::make_unique<bencoding::bencode_integer>(-89);
    std::cout << bd.encode() << "\n"; // prints d3:abci-89e4:key1i42e4:key26:ok byee
    bd.erase(bd.find("key1"));
    std::cout << bd.encode() << "\n"; // d3:abci-89e4:key26:ok byee
    return 0;
}
```

2.

```cpp
#include "custom_bencoder.h"

int main()
{
    bencoding::bencode_dict bd;
    bd.insert_or_assign<bencoding::bencode_integer>("key1", 42);
    bd.insert_or_assign<bencoding::bencode_string>("abc", "ok bye");

    bencoding::bencode_list bl;
    bl.push_back<bencoding::bencode_integer>(88);
    bl.push_back<bencoding::bencode_string>("hello");

    bencoding::bencode_dict new_dict;
    new_dict.insert_or_assign<bencoding::bencode_list>("key_list", std::move(bl));
    new_dict["key_dict"] = std::make_unique<bencoding::bencode_dict>(std::move(bd));
    // bl and bd are NOT supposed to be used now since they have been moved.

    std::cout << new_dict.encode() << "\n";
    // prints d8:key_dictd3:abc6:ok bye4:key1i42ee8:key_listli88e5:helloee
    return 0;
}
```

### string_subs

It is a struct for easily passing arguments to decode() methods.

**API**:

- Public data members : `str` : for accessing underlying data members and `citer` : a const_iterator to some position in the string.
- Parametrized ctor which accepts a `std::string` or `const char*` and intializes str with the argument and citer to beginning of the string.
- refresh() to reset citer's position back to beginning of string. Useful if required to parse a string again.

Example usage should be clear from preceding examples.

## License

This library is licensed under the [MIT License](LICENSE).