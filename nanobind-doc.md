## Extending C++ Classes in Python with Nanobind

Nanobind allows you to extend C++ types within Python, enabling dynamic extension of compiled code with additional fields and functionality. You can prevent subclassing by using the `is_final` annotation when binding classes.

### Basic Inheritance

Let's start with a simple example using `Dog` and `DogHouse` classes:

```cpp
#include <nanobind/stl/string.h>

namespace nb = nanobind;

struct Dog {
    std::string name;
    std::string bark() const { return name + ": woof!"; }
};

struct DogHouse {
    Dog dog;
};

NB_MODULE(my_ext, m) {
    nb::class_<Dog>(m, "Dog")
       .def(nb::init<const std::string&>())
       .def("bark", &Dog::bark)
       .def_rw("name", &Dog::name);

    nb::class_<DogHouse>(m, "DogHouse")
       .def(nb::init<Dog>())
       .def_rw("dog", &DogHouse::dog);
}
```

In Python, you can create a `GuardDog` class that inherits from `my_ext.Dog` and adds an `alarm` method:

```python
import my_ext

class GuardDog(my_ext.Dog):
    def alarm(self, count=3):
        for i in range(count):
            print(self.bark())

gd = GuardDog("Max")
gd.alarm()
# Expected Output:
# Max: woof!
# Max: woof!
# Max: woof!
```

This Python subclass acts as a wrapper around the C++ base object. By default, this wrapper is lost when nanobind copies or transfers ownership to C++.

```python
d = my_ext.DogHouse()
d.dog = gd
try:
    d.dog.alarm()
except AttributeError as e:
    print(f"Caught Error: {e}")
    # Expected Output: Caught Error: 'Dog' object has no attribute 'alarm'
```

To preserve the Python extensions, use shared ownership with shared pointers or intrusive reference counting. Here's how to modify `DogHouse` to use `std::shared_ptr`:

```cpp
#include <nanobind/stl/shared_ptr.h>

struct DogHouse {
    std::shared_ptr<Dog> dog;
};
```

Now, the Python extension is preserved:

```python
d = my_ext.DogHouse()
d.dog = gd
d.dog.alarm()
# Expected Output:
# Max: woof!
# Max: woof!
# Max: woof!
```

### Overriding Virtual Functions

Let's explore how to override C++ virtual functions in Python. Consider a `Dog` class with a virtual `bark` method called by a C++ `alarm` function:

```cpp
#include <iostream>

struct Dog {
    std::string name;
    Dog(const std::string& name) : name(name) {}
    virtual std::string bark() const { return name + ": woof!"; }
};

void alarm(Dog* dog, size_t count = 3) {
    for (size_t i = 0; i < count; ++i) {
        std::cout << dog->bark() << std::endl;
    }
}
```

A naive binding might look like this:

```cpp
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(my_ext, m) {
    nb::class_<Dog>(m, "Dog")
       .def(nb::init<const std::string&>())
       .def("bark", &Dog::bark)
       .def_rw("name", &Dog::name);

    m.def("alarm", &alarm, "dog"_a, "count"_a = 3);
}
```

However, this won't propagate virtual function calls from C++ to Python:

```python
class ShihTzu(my_ext.Dog):
    def bark(self):
        return self.name + ": yip!"

dog = ShihTzu("Mr. Fluffles")
print(dog.bark())
# Expected Output: Mr. Fluffles: yip!

import my_ext
my_ext.alarm(dog)
# Expected Output:
# Mr. Fluffles: woof!
# Mr. Fluffles: woof!
# Mr. Fluffles: woof!
```

To fix this, implement a trampoline class. A trampoline captures virtual function calls in C++ and forwards them to Python:

```cpp
#include <nanobind/trampoline.h>

struct PyDog : Dog {
    NB_TRAMPOLINE(Dog, 1);

    std::string bark() const override {
        NB_OVERRIDE(bark);
    }
};
```

Here, `NB_TRAMPOLINE(Dog, 1)` marks `PyDog` as a trampoline for `Dog`, with 1 indicating the number of virtual method slots. `NB_OVERRIDE(bark)` intercepts the virtual call, checks for a Python override, and forwards the call if found. Otherwise, it falls back to the base class implementation.

**Note:** The number of virtual method slots is crucial. Insufficient slots can lead to runtime errors.

The `NB_OVERRIDE` macro can accept additional arguments if the virtual function has parameters:

```cpp
std::string bark(int volume) const override {
    NB_OVERRIDE(bark, volume);
}
```

Use `NB_OVERRIDE_PURE()` for pure virtual functions and `NB_OVERRIDE()` for functions with default implementations. `NB_OVERRIDE_PURE_NAME()` and `NB_OVERRIDE_NAME()` allow specifying the Python function name if it differs from the C++ name.

Adapt the binding to use the trampoline:

```cpp
nb::class_<Dog, PyDog /* trampoline */>(m, "Dog")
   .def(nb::init<const std::string&>())
   .def("bark", &Dog::bark); // Note: Bind against the actual class, not the trampoline
```

Now, the virtual function call works as expected:

```python
import my_ext
my_ext.alarm(dog)
# Expected Output:
# Mr. Fluffles: yip!
# Mr. Fluffles: yip!
# Mr. Fluffles: yip!
```

### Caveats

You cannot implement a Python trampoline for a method returning a reference or pointer to a type requiring type casting. For example, this will fail:

```cpp
const std::string& get_name() const override {
    NB_OVERRIDE(get_name);
}
```

This results in a static assertion error because the Python side would return a temporary `str` object, which would be invalid when accessed from C++. The solution is to return by value or use bindings instead of type casters.
