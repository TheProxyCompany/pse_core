.. _api:

.. cpp:namespace:: nanobind

C++ API Reference (Core)
========================

Macros
------

.. c:macro:: NB_MODULE(name, variable)

   This macro creates the entry point that will be invoked when the Python
   interpreter imports an extension module. The module name is given as the
   first argument and it should not be in quotes. It **must** match the module
   name given to the :cmake:command:`nanobind_add_module()` function in the
   CMake build system.

   The second macro argument defines a variable of type :cpp:class:`module_`.
   The body of the declaration typically contains a sequence of operations
   that populate the module variable with contents.

   .. code-block:: cpp

       NB_MODULE(example, m) {
           m.doc() = "Example module";

           // Add bindings here
           m.def("add", []() {
               return "Hello, World!";
           });
       }

.. c:macro:: NB_MAKE_OPAQUE(T)

   The macro registers a partial template specialization pattern for the type
   `T` that marks it as *opaque*, meaning that nanobind won't try to run its
   type casting template machinery on it.

   This is useful when trying to register a binding for `T` that is simultaneously
   also covered by an existing type caster.

   This macro should be used at the top level (outside of namespaces and
   program code).

Python object API
-----------------

Nanobind ships with a wide range of Python wrapper classes like
:cpp:class:`object`, :cpp:class:`list`, etc. Besides class-specific operations
(e.g., :cpp:func:`list::append`), these classes also implement core operations
that can be performed on *any* Python object. Since it would be tedious to
implement this functionality over and over again, it is realized by the
following mixin class that lives in the ``nanobind::detail`` namespace.

.. cpp:namespace:: nanobind::detail

.. cpp:class:: template <typename Derived> api

   This mixin class adds common functionality to various nanobind types using
   the `curiously recurring template pattern
   <https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern>`_
   (CRTP). The only requirement for the `Derived` template parameter is that it
   implements the member function ``PyObject *ptr() const`` that gives access
   to the underlying Python object pointer.

   .. cpp:function:: Derived &derived()

      Obtain a mutable reference to the derived class.

   .. cpp:function:: const Derived &derived() const

      Obtain a const reference to the derived class.

   .. cpp:function:: handle inc_ref() const

      Increases the reference count and returns a reference to the Python object.

   .. cpp:function:: handle dec_ref() const

      Decreases the reference count and returns a reference to the Python object.

   .. cpp:function:: iterator begin() const

      Return a forward iterator analogous to ``iter()`` in Python. The object
      must be a collection that supports the iteration protocol. This interface
      provides a generic iterator that works any type of Python object. The
      :cpp:class:`tuple`, :cpp:class:`list`, and :cpp:class:`dict` wrappers
      provide more efficient specialized alternatives.

   .. cpp:function:: iterator end() const

      Return a sentinel that ends the iteration.

   .. cpp:function:: handle type() const

      Return a :cpp:class:`handle` to the underlying Python type object.

   .. cpp:function:: operator handle() const

      Return a :cpp:class:`handle` wrapping the underlying ``PyObject*`` pointer.

   .. cpp:function:: detail::accessor<obj_attr> attr(handle key) const

      Analogous to ``self.key`` in Python, where ``key`` is a Python object.
      The result is wrapped in an :cpp:class:`accessor <detail::accessor>` so
      that it can be read and written.

   .. cpp:function:: detail::accessor<str_attr> attr(const char * key) const

      Analogous to ``self.key`` in Python, where ``key`` is a C-style string.
      The result is wrapped in an :cpp:class:`accessor <detail::accessor>` so
      that it can be read and written.

   .. cpp:function:: detail::accessor<str_attr> doc() const

       Analogous to ``self.__doc__``. The result is wrapped in an
       :cpp:class:`accessor <detail::accessor>` so that it can be read and
       written.

   .. cpp:function:: detail::accessor<obj_item> operator[](handle key) const

      Analogous to ``self[key]`` in Python, where ``key`` is a Python object.
      The result is wrapped in an :cpp:class:`accessor <detail::accessor>` so that it can be read and
      written.

   .. cpp:function:: detail::accessor<str_item> operator[](const char * key) const

      Analogous to ``self[key]`` in Python, where ``key`` is a C-style string.
      The result is wrapped in an :cpp:class:`accessor <detail::accessor>` so that it can be read and
      written.

   .. cpp:function:: template <typename T, enable_if_t<std::is_arithmetic_v<T>> = 1> detail::accessor<num_item> operator[](T key) const

      Analogous to ``self[key]`` in Python, where ``key`` is an arithmetic
      type (e.g., an integer). The result is wrapped in an :cpp:class:`accessor <detail::accessor>` so
      that it can be read and written.

   .. cpp:function:: template <rv_policy policy = rv_policy::automatic_reference, typename... Args> object operator()(Args &&...args) const

      Assuming the Python object is a function or implements the ``__call__``
      protocol, `operator()` invokes the underlying function, passing an
      arbitrary set of parameters, while expanding any detected variable length
      argument and keyword argument packs. The result is returned as an
      :cpp:class:`object` and may need to be converted back into a Python
      object using :cpp:func:`cast()`.

      Type conversion is performed using the return value policy `policy`

      When type conversion of arguments or return value fails, the function
      raises a :cpp:type:`cast_error`. When the Python function call fails, it
      instead raises a :cpp:class:`python_error`.

   .. cpp:function:: args_proxy operator*() const

      Given a a tuple or list, this helper function performs variable argument
      list unpacking in function calls resembling the ``*`` operator in Python.
      Applying `operator*()` twice yields ``**`` keyword argument
      unpacking for dictionaries.

   .. cpp:function:: bool is(handle value) const

      Analogous to ``self is value`` in Python.

   .. cpp:function:: bool is_none() const

      Analogous to ``self is None`` in Python.

   .. cpp:function:: bool is_type() const

      Analogous to ``isinstance(self, type)`` in Python.

   .. cpp:function:: bool is_valid() const

      Checks if this wrapper contains a valid Python object (in the sense that
      the ``PyObject *`` pointer is non-null).

   .. cpp:function:: template <typename T> bool equal(const api<T> &other)

      Equivalent to ``self == other`` in Python.

   .. cpp:function:: template <typename T> bool not_equal(const api<T> &other)

      Equivalent to ``self != other`` in Python.

   .. cpp:function:: template <typename T> bool operator<(const api<T> &other)

      Equivalent to ``self < other`` in Python.

   .. cpp:function:: template <typename T> bool operator<=(const api<T> &other)

      Equivalent to ``self <= other`` in Python.

   .. cpp:function:: template <typename T> bool operator>(const api<T> &other)

      Equivalent to ``self > other`` in Python.

   .. cpp:function:: template <typename T> bool operator>=(const api<T> &other)

      Equivalent to ``self >= other`` in Python.

   .. cpp:function:: object operator-()

      Equivalent to ``-self`` in Python.

   .. cpp:function:: object operator~()

      Equivalent to ``~self`` in Python.

   .. cpp:function:: template <typename T> object operator+(const api<T> &other)

      Equivalent to ``self + other`` in Python.

   .. cpp:function:: template <typename T> object operator-(const api<T> &other)

      Equivalent to ``self - other`` in Python.

   .. cpp:function:: template <typename T> object operator*(const api<T> &other)

      Equivalent to ``self * other`` in Python.

   .. cpp:function:: template <typename T> object operator/(const api<T> &other)

      Equivalent to ``self / other`` in Python.

   .. cpp:function:: template <typename T> object floor_div(const api<T> &other)

      Equivalent to ``self // other`` in Python.

   .. cpp:function:: template <typename T> object operator|(const api<T> &other)

      Equivalent to ``self | other`` in Python.

   .. cpp:function:: template <typename T> object operator&(const api<T> &other)

      Equivalent to ``self & other`` in Python.

   .. cpp:function:: template <typename T> object operator^(const api<T> &other)

      Equivalent to ``self ^ other`` in Python.

   .. cpp:function:: template <typename T> object operator<<(const api<T> &other)

      Equivalent to ``self << other`` in Python.

   .. cpp:function:: template <typename T> object operator>>(const api<T> &other)

      Equivalent to ``self >> other`` in Python.

   .. cpp:function:: template <typename T> object operator+=(const api<T> &other)

      Equivalent to ``self += other`` in Python. Note that the `api<T>` version
      of the in-place operator does not update the ``self`` reference, which
      may lead to unexpected results when working with immutable types that
      return their result instead of updating ``self``.

      The :cpp:class:`object` class and subclasses override the in-place
      operators to achieve more intuitive behavior.

   .. cpp:function:: template <typename T> object operator-=(const api<T> &other)

       Equivalent to ``self -= other`` in Python. See :cpp:func:`operator+=` for limitations.

   .. cpp:function:: template <typename T> object operator*=(const api<T> &other)

       Equivalent to ``self *= other`` in Python. See :cpp:func:`operator+=` for limitations.

   .. cpp:function:: template <typename T> object operator/=(const api<T> &other)

       Equivalent to ``self /= other`` in Python. See :cpp:func:`operator+=` for limitations.

   .. cpp:function:: template <typename T> object operator|=(const api<T> &other)

       Equivalent to ``self |= other`` in Python. See :cpp:func:`operator+=` for limitations.

   .. cpp:function:: template <typename T> object operator&=(const api<T> &other)

       Equivalent to ``self &= other`` in Python. See :cpp:func:`operator+=` for limitations.

   .. cpp:function:: template <typename T> object operator^=(const api<T> &other)

       Equivalent to ``self ^= other`` in Python. See :cpp:func:`operator+=` for limitations.

   .. cpp:function:: template <typename T> object operator<<=(const api<T> &other)

       Equivalent to ``self <<= other`` in Python. See :cpp:func:`operator+=` for limitations.

   .. cpp:function:: template <typename T> object operator>>=(const api<T> &other)

       Equivalent to ``self >>= other`` in Python. See :cpp:func:`operator+=` for limitations.

.. cpp:class:: template <typename Impl> accessor

   This helper class facilitates attribute and item access. Casting an
   :cpp:class:`accessor` to a :cpp:class:`handle` or :cpp:class:`object`
   subclass causes a corresponding call to ``__getitem__`` or ``__getattr__``
   depending on the template argument `Impl`. Assigning a
   :cpp:class:`handle` or :cpp:class:`object` subclass causes a call to
   ``__setitem__`` or ``__setattr__``.

.. cpp:namespace:: nanobind

Handles and objects
-------------------

nanobind provides two styles of Python object wrappers: classes without
reference counting deriving from :cpp:class:`handle`, and reference-counted
wrappers deriving from :cpp:class:`object`. Reference counting bugs can be
really tricky to track down, hence it is recommended that you always prefer
:cpp:class:`object`-style wrappers unless there are specific reasons that
warrant the use of raw handles.

Without reference counting
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. cpp:class:: handle: public detail::api<handle>

   This class provides a thin wrapper around a raw ``PyObject *`` pointer. Its
   main purpose is to intercept various C++ operations and convert them into
   Python C API calls. It does *not* do any reference counting and can be
   somewhat unsafe to use.

   .. cpp:function:: handle() = default

      Default constructor. Creates an invalid handle wrapping a null pointer.
      (:cpp:func:`detail::api::is_valid()` is ``false``)

   .. cpp:function:: handle(const handle &) = default

      Default copy constructor.

   .. cpp:function:: handle(handle &&) = default

      Default move constructor.

   .. cpp:function:: handle(const PyObject * o)

      Initialize a handle from a Python object pointer. Does not change the reference count of `o`.

   .. cpp:function:: handle(const PyTypeObject * o)

      Initialize a handle from a Python type object pointer. Does not change the reference count of `o`.

   .. cpp:function:: handle &operator=(const handle &) = default

      Default copy assignment operator.

   .. cpp:function:: handle &operator=(handle &&) = default

      Default move assignment operator.

   .. cpp:function:: explicit operator bool() const

      Check if the handle refers to a valid Python object. Equivalent to
      :cpp:func:`detail::api::is_valid()`

   .. cpp:function:: handle inc_ref() const noexcept

      Increases the reference count and returns a reference to the Python object.
      Never raises an exception.

   .. cpp:function:: handle dec_ref() const noexcept

      Decreases the reference count and returns a reference to the Python object.
      Never raises an exception.

   .. cpp:function:: PyObject * ptr() const

      Return the underlying ``PyObject*`` pointer.

With reference counting
^^^^^^^^^^^^^^^^^^^^^^^

.. cpp:class:: object: public handle

   This class provides a convenient `RAII
   <https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization>`_
   wrapper around a ``PyObject*`` pointer. Like :cpp:class:`handle`, it
   intercepts various C++ operations and converts them into Python C API calls.

   The main difference to :cpp:class:`handle` is that it uses reference
   counting to keep the underlying Python object alive.

   Use the :cpp:func:`borrow()` and :cpp:func:`steal()` functions to create an
   :cpp:class:`object` from a :cpp:class:`handle` or ``PyObject*`` pointer.

   .. cpp:function:: object() = default

      Default constructor. Creates an invalid object wrapping a null pointer.
      (:cpp:func:`detail::api::is_valid()` is ``false``)

   .. cpp:function:: object(object &&o)

      Move constructor. Steals the object from `o` without
      changing its reference count.

   .. cpp:function:: object(const object &o)

      Copy constructor. Acquires a new reference to `o` (if valid).

   .. cpp:function:: ~object()

      Decrease the reference count of the referenced Python object (if valid).

   .. cpp:function:: object& operator=(object &&o)

      Move assignment operator. Decreases the reference count of the currently
      held object (if valid) and steals the object from `o` without
      changing its reference count.

   .. cpp:function:: object& operator=(const object &o)

      Copy assignment operator. Decreases the reference count of the currently
      held object (if valid) and acquires a new reference to the object
      `o` (if valid).

   .. cpp:function:: void reset()

      Decreases the reference count of the currently held object (if valid) and
      resets the internal pointer to ``nullptr``.

   .. cpp:function:: handle release()

      Resets the internal pointer to ``nullptr`` and returns its previous
      contents as a :cpp:class:`handle`. This operation does not change
      the object's reference count and should be used carefully.

   .. cpp:function:: template <typename T> object& operator+=(const api<T> &other)

      Equivalent to ``self += other`` in Python.

   .. cpp:function:: template <typename T> object& operator-=(const api<T> &other)

       Equivalent to ``self -= other`` in Python.

   .. cpp:function:: template <typename T> object& operator*=(const api<T> &other)

       Equivalent to ``self *= other`` in Python.

   .. cpp:function:: template <typename T> object& operator/=(const api<T> &other)

       Equivalent to ``self /= other`` in Python.

   .. cpp:function:: template <typename T> object& operator|=(const api<T> &other)

       Equivalent to ``self |= other`` in Python.

   .. cpp:function:: template <typename T> object& operator&=(const api<T> &other)

       Equivalent to ``self &= other`` in Python.

   .. cpp:function:: template <typename T> object& operator^=(const api<T> &other)

       Equivalent to ``self ^= other`` in Python.

   .. cpp:function:: template <typename T> object& operator<<=(const api<T> &other)

       Equivalent to ``self <<= other`` in Python.

   .. cpp:function:: template <typename T> object& operator>>=(const api<T> &other)

       Equivalent to ``self >>= other`` in Python.


.. cpp:function:: template <typename T = object> T borrow(handle h)

   Create a reference-counted Python object wrapper of type `T` from a raw
   handle or ``PyObject *`` pointer. The target type `T` must be
   :cpp:class:`object` (the default) or one of its derived classes. The
   function does not perform any conversions or checks---it is up to the user
   to make sure that the target type is correct.

   The function *borrows* a reference, which means that it will increase the
   reference count while constructing ``T``.

   For example, consider the Python C API function `PyList_GetItem()
   <https://docs.python.org/3/c-api/list.html#c.PyList_GetItem>`_, whose
   documentation states that it returns a borrowed reference. An interface
   between this API and nanobind could look as follows:

   .. code-block:: cpp


       PyObject* list = ...;
       Py_ssize_t index = ...;
       nb::object o = nb::borrow(PyList_GetItem(obj, index));

   Using :cpp:func:`steal()` in this setting is incorrect and would lead to a
   reference underflow.

.. cpp:function:: template <typename T = object> T steal(handle h)

   Create a reference-counted Python object wrapper of type `T` from a raw
   handle or ``PyObject *`` pointer. The target type `T` must be
   :cpp:class:`object` (the default) or one of its derived classes. The
   function does not perform any conversions or checks---it is up to the user
   to make sure that the target type is correct.

   The function *steals* a reference, which means that constructing ``T``
   leaves the object's reference count unchanged.

   For example, consider the Python C API function `PyObject_Str()
   <https://docs.python.org/3/c-api/object.html#c.PyObject_Str>`_, whose
   documentation states that it returns a *new reference*. An interface
   between this API and nanobind could look as follows:

   .. code-block:: cpp

       PyObject* value = ...;
       nb::object o = nb::steal(PyObject_Str(value));

   Using :cpp:func:`borrow()` in this setting is incorrect and would lead to a
   reference leak.


Attribute access
----------------

.. cpp:function:: bool hasattr(handle h, const char * key) noexcept

   Check if the given object has an attribute string ``key``. The function never
   raises an exception and returns ``false`` in case of an internal error.

   Equivalent to ``hasattr(h, key)`` in Python.

.. cpp:function:: bool hasattr(handle h, handle key) noexcept

   Check if the given object has a attribute represented by the Python object
   ``key``. The function never raises an exception and returns ``false`` in
   case of an internal error.

   Equivalent to ``hasattr(h, key)`` in Python.

.. cpp:function:: object getattr(handle h, const char * key)

   Equivalent to ``h.key`` and ``getattr(h, key)`` in Python.
   Raises :cpp:class:`python_error` if the operation fails.

.. cpp:function:: object getattr(handle h, handle key)

   Equivalent to ``h.key`` and ``getattr(h, key)`` in Python.
   Raises :cpp:class:`python_error` if the operation fails.

.. cpp:function:: object getattr(handle h, const char * key, handle def) noexcept

   Equivalent to ``getattr(h, key, def)`` in Python. Never raises an
   exception and returns ``def`` when the operation fails, or when the desired
   attribute could not be found.

.. cpp:function:: object getattr(handle h, handle key, handle def) noexcept

   Equivalent to ``getattr(h, key, def)`` in Python. Never raises an
   exception and returns ``def`` when the operation fails, or when the desired
   attribute could not be found.

.. cpp:function:: void setattr(handle h, const char * key, handle value)

   Equivalent to ``h.key = value`` and ``setattr(h, key, value)`` in Python.
   Raises :cpp:class:`python_error` if the operation fails.

.. cpp:function:: void setattr(handle h, handle key, handle value)

   Equivalent to ``h.key = value`` and ``setattr(h, key, value)`` in Python.
   Raises :cpp:class:`python_error` if the operation fails.

.. cpp:function:: void delattr(handle h, const char * key)

   Equivalent to ``del h.key`` and ``delattr(h, key)`` in Python.
   Raises :cpp:class:`python_error` if the operation fails.

.. cpp:function:: void delattr(handle h, handle key)

   Equivalent to ``del h.key`` and ``delattr(h, key)`` in Python.
   Raises :cpp:class:`python_error` if the operation fails.

.. cpp:function:: template <typename T> void del(detail::accessor<T> &)

   Remove an element from a sequence or mapping. The C++ statement

   .. code-block:: cpp

      nb::del(o[key]);

   is equivalent to ``del o[key]`` in Python.

   When the element cannot be removed, the function will raise
   :cpp:class:`python_error` wrapping either a Python ``IndexError`` (for
   sequence types) or a ``KeyError`` (for mapping types).

.. cpp:function:: template <typename T> void del(detail::accessor<T> &&)

   Rvalue equivalent of the above expression.

Size queries
------------

.. cpp:function:: size_t len(handle h)

   Equivalent to ``len(h)`` in Python. Raises :cpp:class:`python_error` if the
   operation fails.

.. cpp:function:: size_t len(const tuple &t)

   Equivalent to ``len(t)`` in Python. Optimized variant for tuples.

.. cpp:function:: size_t len(const list &l)

   Equivalent to ``len(l)`` in Python. Optimized variant for lists.

.. cpp:function:: size_t len(const dict &d)

   Equivalent to ``len(d)`` in Python. Optimized variant for dictionaries.

.. cpp:function:: size_t len(const set &d)

   Equivalent to ``len(d)`` in Python. Optimized variant for sets.

.. cpp:function:: size_t len_hint(handle h)

   Equivalent to ``operator.length_hint(h)`` in Python. Raises
   :cpp:class:`python_error` if the operation fails.

Type queries
------------

.. cpp:function:: template <typename T> isinstance(handle h)

   Checks if the Python object `h` represents a valid instance of the C++ type
   `T`. This works for bound C++ classes, basic types (``int``, ``bool``,
   etc.), and Python type wrappers ( :cpp:class:`list`, :cpp:class:`dict`,
   :cpp:class:`module_`, etc.).

   *Note*: the check even works when `T` involves a type caster (e.g., an STL
   types like ``std::vector<float>``). However, this involve a wasteful attempt
   to convert the object to C++. It may be more efficient to just perform the
   conversion using :cpp:func:`cast` and catch potential raised exceptions.

.. cpp:function:: isinstance(handle inst, handle cls)

   Checks if the Python object `inst` is an instance of the Python type `cls`.

.. cpp:function:: template <typename T> handle type() noexcept

   Returns the Python type object associated with the C++ type `T`. When the
   type not been bound via nanobind, the function returns an invalid handle
   (:cpp:func:`detail::api::is_valid()` is ``false``).

   *Note*: in contrast to the :cpp:func:`isinstance()` function above, builtin
   types, type wrappers, and types handled using type casters, are *not*
   supported.

Wrapper classes
---------------

.. cpp:class:: tuple: public object

   Wrapper class representing Python ``tuple`` instances.

   Use the standard ``operator[]`` C++ operator with an integer argument to
   read tuple elements (the bindings for this operator are provided by the
   parent class and not listed here). Once created, the set is immutable and
   its elements cannot be replaced.

   Use the :py:func:`make_tuple` function to create new tuples.

   .. cpp:function:: tuple()

      Create an empty tuple

   .. cpp:function:: tuple(handle h)

      Attempt to convert a given Python object into a tuple. Analogous to the
      expression ``tuple(h)`` in Python.

   .. cpp:function:: size_t size() const

      Return the number of tuple elements.

   .. cpp:function:: detail::fast_iterator begin() const

      Return a forward iterator analogous to ``iter()`` in Python. The function
      overrides a generic version in :cpp:class:`detail::api` and is more
      efficient for tuples.

   .. cpp:function:: detail::fast_iterator end() const

      Return a sentinel that ends the iteration.

   .. cpp:function:: template <typename T, enable_if_t<std::is_arithmetic_v<T>> = 1> detail::accessor<num_item_tuple> operator[](T key) const

      Analogous to ``self[key]`` in Python, where ``key`` is an arithmetic
      type (e.g., an integer). The result is wrapped in an :cpp:class:`accessor <detail::accessor>` so
      that it can be read and converted. Write access is not possible.

      The function overrides the generic version in :cpp:class:`detail::api`
      and is more efficient for tuples.


.. cpp:class:: list : public object

   Wrapper class representing Python ``list`` instances.

   Use the standard ``operator[]`` C++ operator with an integer argument to
   read and write list elements (the bindings for this operator are provided by
   the parent class and not listed here).

   Use the :cpp:func:`nb::del <del>` function to remove elements.

   .. cpp:function:: list()

      Create an empty list

   .. cpp:function:: list(handle h)

      Attempt to convert a given Python object into a list. Analogous to the
      expression ``list(h)`` in Python.

   .. cpp:function:: size_t size() const

      Return the number of list elements.

   .. cpp:function:: template <typename T> void append(T&& value)

      Append an element to the list. When `T` does not already represent a
      wrapped Python object, the function performs a cast.

   .. cpp:function:: template <typename T> void insert(Py_ssize_t index, T&& value)

      Insert an element to the list (at index ``index``, which may also be
      negative). When `T` does not already represent a wrapped Python object,
      the function performs a cast.

   .. cpp:function:: void clear()

      Clear the list entries.

   .. cpp:function:: void extend(handle h)

      Analogous to the ``.extend(h)`` method of ``list`` in Python.

   .. cpp:function:: void sort()

      Analogous to the ``.sort()`` method of ``list`` in Python.

   .. cpp:function:: void reverse()

      Analogous to the ``.reverse()`` method of ``list`` in Python.

   .. cpp:function:: template <typename T, enable_if_t<std::is_arithmetic_v<T>> = 1> detail::accessor<num_item_list> operator[](T key) const

      Analogous to ``self[key]`` in Python, where ``key`` is an arithmetic
      type (e.g., an integer). The result is wrapped in an :cpp:class:`accessor <detail::accessor>` so
      that it can be read and written.

      The function overrides the generic version in :cpp:class:`detail::api`
      and is more efficient for lists.

   .. cpp:function:: detail::fast_iterator begin() const

      Return a forward iterator analogous to ``iter()`` in Python. The operator
      provided here overrides the generic version in :cpp:class:`detail::api`
      and is more efficient for lists.

   .. cpp:function:: detail::fast_iterator end() const

      Return a sentinel that ends the iteration.


.. cpp:class:: dict: public object

   Wrapper class representing Python ``dict`` instances.

   Use the standard ``operator[]`` C++ operator to read and write dictionary
   elements (the bindings for this operator are provided by the parent class
   and not listed here).

   Use the :cpp:func:`nb::del <del>` function to remove elements.

   .. cpp:function:: dict()

      Create an empty dictionary

   .. cpp:function:: size_t size() const

      Return the number of dictionary elements.

   .. cpp:function:: template <typename T> bool contains(T&& key) const

      Check whether the dictionary contains a particular key. When `T` does not
      already represent a wrapped Python object, the function performs a cast.

   .. cpp:function:: detail::dict_iterator begin() const

      Return an item iterator that returns ``std::pair<handle, handle>``
      key-value pairs analogous to ``iter(dict.items())`` in Python.

      In free-threaded Python, the :cpp:class:``detail::dict_iterator`` class
      acquires a lock to the underlying dictionary to enable the use of the
      efficient but thread-unsafe ``PyDict_Next()`` Python C traversal routine.

   .. cpp:function:: detail::dict_iterator end() const

      Return a sentinel that ends the iteration.

   .. cpp:function:: list keys() const

      Return a list containing all dictionary keys.

   .. cpp:function:: list values() const

      Return a list containing all dictionary values.

   .. cpp:function:: list items() const

      Return a list containing all dictionary items as ``(key, value)`` pairs.

   .. cpp:function:: void clear()

      Clear the contents of the dictionary.

   .. cpp:function:: void update(handle h)

      Analogous to the ``.update(h)`` method of ``dict`` in Python.

.. cpp:class:: set: public object

   Wrapper class representing Python ``set`` instances.

   .. cpp:function:: set()

      Create an empty set

   .. cpp:function:: set(handle h)

      Attempt to convert a given Python object into a set. Analogous to the
      expression ``set(h)`` in Python.

   .. cpp:function:: size_t size() const

      Return the number of set elements.

   .. cpp:function:: template <typename T> void add(T&& key)

      Add a key to the set. When `T` does not already represent a wrapped
      Python object, the function performs a cast.

   .. cpp:function:: template <typename T> bool contains(T&& key) const

      Check whether the set contains a particular key. When `T` does not
      already represent a wrapped Python object, the function performs a cast.

   .. cpp:function:: void clear()

      Clear the contents of the set.

   .. cpp:function:: template <typename T> bool discard(T&& key)

      Analogous to the ``.discard(h)`` method of the ``set`` type in Python.
      Returns ``true`` if the item was deleted successfully, and ``false`` if
      the value was not present. When `T` does not already represent a wrapped
      Python object, the function performs a cast.

.. cpp:class:: module_: public object

   Wrapper class representing Python ``module`` instances. The underscore at
   the end disambiguates the class name from the C++20 ``module`` declaration.

   .. cpp:function:: template <typename Func, typename... Extra> module_ &def(const char * name, Func &&f, const Extra &...extra)

      Bind the function `f` to the identifier `name` within the module. Returns
      a reference to ``*this`` so that longer sequences of binding declarations
      can be chained, as in ``m.def(...).def(...);``. The variable length
      `extra` parameter can be used to pass docstrings and other :ref:`function
      binding annotations <function_binding_annotations>`.

      Example syntax:

      .. code-block:: cpp

         void test() { printf("Hello world!"); }

         NB_MODULE(example, m) {
             // here, "m" is variable of type 'module_'.
             m.def("test", &test, "A test function")
              .def(...); // more binding declarations
         }


   .. cpp:function:: module_ import_(const char * name)

      Import the Python module with the specified name and return a reference
      to it. The underscore at the end disambiguates the function name from the
      C++20 ``import`` statement.

      Example usage:

      .. code-block:: cpp

         nb::module_ np = nb::module_::import_("numpy");
         nb::object np_array = np.attr("array");

   .. cpp:function:: module_ import_(handle name)

      Import the Python module with the specified name and return a reference
      to it. In contrast to the version above, this function expects a Python
      object as key.

   .. cpp:function:: module_ def_submodule(const char * name, const char * doc = nullptr)

      Create a Python submodule within an existing module and return a
      reference to it. Can be chained recursively.

      Example usage:

      .. code-block:: cpp

         NB_MODULE(example, m) {
             nb::module_ m2 = m.def_submodule("sub", "A submodule of 'example'");
             nb::module_ m3 = m2.def_submodule("subsub", "A submodule of 'example.sub'");
         }

.. cpp:class:: capsule: public object

   Capsules are small opaque Python objects that wrap a C or C++ pointer and a cleanup routine.

   .. cpp:function:: capsule(const void * ptr, void (* cleanup)(void*) noexcept = nullptr)

      Construct an *unnamed* capsule wrapping the pointer `p`. When the
      capsule is garbage collected, Python will call the destructor `cleanup`
      (if provided) with the value of `p`.

   .. cpp:function:: capsule(const void * ptr, const char * name, void (* cleanup)(void*) noexcept = nullptr)

      Construct a *named* capsule with name `name` wrapping the pointer `p`.
      When the capsule is garbage collected, Python will call the destructor
      `cleanup` (if provided) with the value of `p`.

   .. cpp:function:: const char * name() const

      Return the capsule name (or ``nullptr`` when the capsule is unnamed)

   .. cpp:function:: void * data() const

      Return the pointer wrapped by the capsule.


.. cpp:class:: bool_: public object

   This wrapper class represents Python ``bool`` instances.

   .. cpp:function:: bool_(handle h)

      Performs a boolean cast within Python. This is equivalent to the Python
      expression ``bool(h)``.

   .. cpp:function:: explicit bool_(bool value)

      Convert an C++ boolean instance into a Python ``bool``.

   .. cpp:function:: explicit operator bool() const

      Extract the boolean value underlying this object.


.. cpp:class:: int_: public object

   This wrapper class represents Python ``int`` instances. It can handle large
   numbers requiring more than 64 bits of storage.

   .. cpp:function:: int_(handle h)

      Performs an integer cast within Python. This is equivalent to the Python
      expression ``int(h)``.

   .. cpp:function:: template <typename T, detail::enable_if_t<std::is_arithmetic_v<T>> = 0> explicit int_(T value)

      Convert an C++ arithmetic type into a Python integer.

   .. cpp:function:: template <typename T, detail::enable_if_t<std::is_arithmetic_v<T>> = 0> explicit operator T() const

      Convert a Python integer into a C++ arithmetic type.


.. cpp:class:: float_: public object

   This wrapper class represents Python ``float`` instances.

   .. cpp:function:: float_(handle h)

      Performs an floating point cast within Python. This is equivalent to the
      Python expression ``float(h)``.

   .. cpp:function:: explicit float_(double value)

      Convert an C++ double value into a Python float objecct

   .. cpp:function:: explicit operator double() const

      Convert a Python float object into a C++ double value


.. cpp:class:: str: public object

   This wrapper class represents Python unicode ``str`` instances.

   .. cpp:function:: str(handle h)

      Performs a string cast within Python. This is equivalent equivalent to
      the Python expression ``str(h)``.

   .. cpp:function:: str(const char * s)

      Convert a null-terminated C-style string in UTF-8 encoding into a Python string.

   .. cpp:function:: str(const char * s, size_t n)

      Convert a C-style string in UTF-8 encoding of length ``n`` bytes into a Python string.

   .. cpp:function:: const char * c_str() const

      Convert a Python string into a null-terminated C-style string with UTF-8
      encoding.

      *Note*: The C string will be deleted when the `str` instance is garbage
      collected.

   .. cpp:function:: template <typename... Args> str format(Args&&... args)

      C++ analog of the Python routine ``str.format``. Can be called with
      positional and keyword arguments.


.. cpp:class:: bytes: public object

   This wrapper class represents Python unicode ``bytes`` instances.

   .. cpp:function:: bytes(handle h)

      Performs a cast within Python. This is equivalent to
      the Python expression ``bytes(h)``.

   .. cpp:function:: bytes(const char * s)

      Convert a null-terminated C-style string encoding into a Python ``bytes`` object.

   .. cpp:function:: bytes(const void * buf, size_t n)

      Convert a byte buffer ``buf`` of length ``n`` bytes into a Python ``bytes`` object.  The buffer can contain embedded null bytes.

   .. cpp:function:: const char * c_str() const

      Convert a Python bytes object into a null-terminated C-style string.

   .. cpp:function:: size_t size() const

      Return the size in bytes.

   .. cpp:function:: const void * data() const

      Convert a Python ``bytes`` object into a byte buffer of length :cpp:func:`bytes::size()` bytes.


.. cpp:class:: bytearray: public object

   This wrapper class represents Python ``bytearray`` instances.

   .. cpp:function:: bytearray()

      Create an empty ``bytearray``.

   .. cpp:function:: bytearray(handle h)

      Performs a cast within Python. This is equivalent to
      the Python expression ``bytearray(h)``.

   .. cpp:function:: bytearray(const void * buf, size_t n)

      Convert a byte buffer ``buf`` of length ``n`` bytes into a Python ``bytearray`` object.  The buffer can contain embedded null bytes.

   .. cpp:function:: const char * c_str() const

      Convert a Python ``bytearray`` object into a null-terminated C-style string.

   .. cpp:function:: size_t size() const

      Return the size in bytes.

   .. cpp:function:: void * data()

      Convert a Python ``bytearray`` object into a byte buffer of length :cpp:func:`bytearray::size()` bytes.

   .. cpp:function:: const void * data() const

      Convert a Python ``bytearray`` object into a byte buffer of length :cpp:func:`bytearray::size()` bytes.

   .. cpp:function:: void resize(size_t n)

      Resize the internal buffer of a Python ``bytearray`` object to ``n``. Any
      space added by this method, which calls `PyByteArray_Resize`, will not be
      initialized and may contain random data.


.. cpp:class:: type_object: public object

   Wrapper class representing Python ``type`` instances.

.. cpp:class:: sequence: public object

   Wrapper class representing arbitrary Python sequence types.

.. cpp:class:: mapping : public object

   Wrapper class representing arbitrary Python mapping types.

   .. cpp:function:: template <typename T> bool contains(T&& key) const

      Check whether the map contains a particular key. When `T` does not
      already represent a wrapped Python object, the function performs a cast.

   .. cpp:function:: list keys() const

      Return a list containing all of the map's keys.

   .. cpp:function:: list values() const

      Return a list containing all of the map's values.

   .. cpp:function:: list items() const

      Return a list containing all of the map's items as ``(key, value)`` pairs.

.. cpp:class:: iterator : public object

   Wrapper class representing a Python iterator.

   .. cpp:function:: iterator& operator++()

      Advance to the next element (pre-increment form).

   .. cpp:function:: iterator& operator++(int)

      Advance to the next element (post-increment form).

   .. cpp:function:: handle operator*() const

      Return the item at the current position.

   .. cpp:function:: handle operator->() const

      Convenience routine for pointer-style access.

   .. static iterator sentinel();

      Return a sentinel that ends the iteration.

   .. cpp:function:: friend bool operator==(const iterator &a, const iterator &b);

      Iterator equality comparison operator.

   .. cpp:function:: friend bool operator!=(const iterator &a, const iterator &b);

      Iterator inequality comparison operator.

.. cpp:class:: iterable : public object

   Wrapper class representing an object that can be iterated upon (in the sense
   that calling :cpp:func:`iter()` is valid).

.. cpp:class:: slice : public object

   Wrapper class representing a Python slice object.

   .. cpp:function:: slice(handle start, handle stop, handle step)

      Create the slice object given by ``slice(start, stop, step)`` in Python.

   .. cpp:function:: template <typename T, detail::enable_if_t<std::is_arithmetic_v<T>> = 0> slice(T stop)

      Create the slice object ``slice(stop)``, where `stop` is represented by a
      C++ integer type.

   .. cpp:function:: template <typename T, detail::enable_if_t<std::is_arithmetic_v<T>> = 0> slice(T start, T stop)

      Create the slice object ``slice(start, stop)``, where `start` and `stop`
      are represented by a C++ integer type.

   .. cpp:function:: template <typename T, detail::enable_if_t<std::is_arithmetic_v<T>> = 0> slice(T start, T stop, T step)

      Create the slice object ``slice(start, stop, step)``, where `start`,
      `stop`, and `step` are represented by a C++ integer type.

   .. cpp:function:: detail::tuple<Py_ssize_t, Py_ssize_t, Py_ssize_t, size_t> compute(size_t size) const

      Adjust the slice to the `size` value of a given container. Returns a tuple containing
      ``(start, stop, step, slice_length)``.

.. cpp:class:: ellipsis: public object

   Wrapper class representing a Python ellipsis (``...``) object.

   .. cpp:function:: ellipsis()

      Create a wrapper referencing the unique Python ``Ellipsis`` object.

.. cpp:class:: not_implemented: public object

   Wrapper class representing a Python ``NotImplemented`` object.

   .. cpp:function:: not_implemented()

      Create a wrapper referencing the unique Python ``NotImplemented`` object.

.. cpp:class:: callable: public object

   Wrapper class representing a callable Python object.

.. cpp:class:: weakref: public object

   Wrapper class representing a Python weak reference object.

   .. cpp:function:: explicit weakref(handle obj, handle callback = { })

      Construct a new weak reference that points to `obj`. If provided,
      Python will invoke the callable `callback` when `obj` expires.

.. cpp:class:: args : public tuple

   Variable argument keyword list for use in function argument declarations.

.. cpp:class:: kwargs : public dict

   Variable keyword argument keyword list for use in function argument declarations.

.. cpp:class:: any : public object

   This wrapper class represents Python ``typing.Any``-typed values. On the C++
   end, this type is interchangeable with :py:class:`object`. The only
   difference is the type signature when used in function arguments and return
   values.

Parameterized wrapper classes
-----------------------------

.. cpp:class:: template <typename T> handle_t : public handle

   Wrapper class representing a handle to a subclass of the C++ type `T`. It
   can be used to bind functions that take the associated Python object in its
   wrapped form, while rejecting objects with a different type (i.e., it is
   more discerning than :cpp:class:`handle`, which accepts *any* Python object).

   .. code-block:: cpp

      // Bind the class A
      class A { int value; };
      nb::class_<A>(m, "A");

      // Bind a function that takes a Python object representing a 'A' instance
      m.def("process_a", [](nb::handle_t<A> h) {
         PyObject * a_py = h.ptr();   // PyObject* pointer to wrapper
         A &a_cpp = nb::cast<A &>(h); // Reference to C++ instance
      });

.. cpp:class:: template <typename T> type_object_t : public type_object

   Wrapper class representing a Python type object that is a subtype of the C++
   type `T`. It can be used to bind functions that only accept type objects
   satisfying this criterion (i.e., it is more discerning than
   :cpp:class:`type_object`, which accepts *any* Python type object).

Error management
----------------

nanobind provides a range of functionality to convert C++ exceptions into
equivalent Python exceptions and raise captured Python error state in C++. The
:cpp:class:`exception` class is also relevant in this context, but is listed in
the reference section on :ref:`class binding <class_binding>`.

.. cpp:struct:: error_scope

   RAII helper class that temporarily stashes any existing Python error status.
   This is important when running Python code in the context of an existing
   failure that must be processed (e.g., to generate an error message).

   .. cpp:function:: error_scope()

      Stash the current error status (if any)

   .. cpp:function:: ~error_scope()

      Restore the stashed error status (if any)

.. cpp:struct:: python_error : public std::exception

   Exception that represents a detected Python error status.

   .. cpp:function:: python_error()

      This constructor may only be called when a Python error has occurred
      (``PyErr_Occurred()`` must be ``true``). It creates a C++ exception
      object that represents this error and clears the Python error status.

   .. cpp:function:: python_error(const python_error &)

      Copy constructor

   .. cpp:function:: python_error(python_error &&) noexcept

      Move constructor

   .. cpp:function:: const char * what() noexcept

      Return a stringified version of the exception. nanobind internally
      normalizes the exception and generates a traceback that is included
      as part of this string. This can be a relatively costly operation
      and should only be used if all of this detail is actually needed.

   .. cpp:function:: bool matches(handle exc) noexcept

      Checks whether the exception has the same type as `exc`.

      The argument to this function is usually one of the `Standard Exceptions
      <https://docs.python.org/3/c-api/exceptions.html#standard-exceptions>`_.

   .. cpp:function:: void restore() noexcept

      Restore the error status in Python and clear the `python_error`
      contents. This may only be called once, and you should not
      reraise the `python_error` in C++ afterward.

   .. cpp:function:: void discard_as_unraisable(handle context) noexcept

      Pass the error to Python's :py:func:`sys.unraisablehook`, which
      prints a traceback to :py:data:`sys.stderr` by default but may
      be overridden.  Like :cpp:func:`restore`, this consumes the
      error and you should not reraise the exception in C++ afterward.

      The *context* argument should be some object whose ``repr()``
      helps identify the location of the error. The default
      :py:func:`sys.unraisablehook` prints a traceback that begins
      with the text ``Exception ignored in:`` followed by
      the result of ``repr(context)``.

      Example use case: handling a Python error that occurs in a C++
      destructor where you cannot raise a C++ exception.

   .. cpp:function:: void discard_as_unraisable(const char * context) noexcept

      Convenience wrapper around the above function, which takes a C-style
      string for the ``context`` argument.

   .. cpp:function:: handle type() const

      Returns a handle to the exception type

   .. cpp:function:: handle value() const

      Returns a handle to the exception value

   .. cpp:function:: object traceback() const

      Returns a handle to the exception's traceback object

.. cpp:class:: cast_error

   The function :cpp:func:`cast` raises this exception to indicate that a cast
   was unsuccessful.

   .. cpp:function:: cast_error()

      Constructor

.. cpp:class:: next_overload

   Raising this special exception from a bound function informs nanobind that
   the function overload detected incompatible inputs. nanobind will then try
   other overloads before reporting a ``TypeError``.

   This feature is useful when a multiple overloads of a function accept
   overlapping or identical input types (e.g. :cpp:class:`object`) and must run
   code at runtime to select the right overload.

   You should probably write a thorough docstring that explicitly mentions the
   expected inputs in this case, since the behavior won't be obvious from the
   auto-generated function signature. It can be frustrating when a function
   call fails with an error message stating that the provided arguments aren't
   compatible with any overload, when the associated error message suggests
   otherwise.

   .. cpp:function:: next_overload()

      Constructor

.. cpp:class:: builtin_exception : public std::runtime_error

   General-purpose class to propagate builtin Python exceptions from C++. A
   number of convenience functions (see below) instantiate it.

.. cpp:function:: builtin_exception stop_iteration(const char * what = nullptr)

   Convenience wrapper to create a :cpp:class:`builtin_exception` C++ exception
   instance that nanobind will re-raise as a Python ``StopIteration`` exception
   when it crosses the C++ ↔ Python interface.

.. cpp:function:: builtin_exception index_error(const char * what = nullptr)

   Convenience wrapper to create a :cpp:class:`builtin_exception` C++ exception
   instance that nanobind will re-raise as a Python ``IndexError`` exception
   when it crosses the C++ ↔ Python interface.

.. cpp:function:: builtin_exception key_error(const char * what = nullptr)

   Convenience wrapper to create a :cpp:class:`builtin_exception` C++ exception
   instance that nanobind will re-raise as a Python ``KeyError`` exception
   when it crosses the C++ ↔ Python interface.

.. cpp:function:: builtin_exception value_error(const char * what = nullptr)

   Convenience wrapper to create a :cpp:class:`builtin_exception` C++ exception
   instance that nanobind will re-raise as a Python ``ValueError`` exception
   when it crosses the C++ ↔ Python interface.

.. cpp:function:: builtin_exception type_error(const char * what = nullptr)

   Convenience wrapper to create a :cpp:class:`builtin_exception` C++ exception
   instance that nanobind will re-raise as a Python ``TypeError`` exception
   when it crosses the C++ ↔ Python interface.

.. cpp:function:: builtin_exception buffer_error(const char * what = nullptr)

   Convenience wrapper to create a :cpp:class:`builtin_exception` C++ exception
   instance that nanobind will re-raise as a Python ``BufferError`` exception
   when it crosses the C++ ↔ Python interface.

.. cpp:function:: builtin_exception import_error(const char * what = nullptr)

   Convenience wrapper to create a :cpp:class:`builtin_exception` C++ exception
   instance that nanobind will re-raise as a Python ``ImportError`` exception
   when it crosses the C++ ↔ Python interface.

.. cpp:function:: builtin_exception attribute_error(const char * what = nullptr)

   Convenience wrapper to create a :cpp:class:`builtin_exception` C++ exception
   instance that nanobind will re-raise as a Python ``AttributeError`` exception
   when it crosses the C++ ↔ Python interface.

.. cpp:function:: void register_exception_translator(void (* exception_translator)(const std::exception_ptr &, void*), void * payload = nullptr)

   Install an exception translator callback that will be invoked whenever
   nanobind's function call dispatcher catches a previously unknown C++
   exception. This exception translator should follow a standard structure of
   re-throwing an exception, catching a specific type, and converting this into
   a Python error status upon "success".

   Here is an example for a hypothetical ``ZeroDivisionException``.

   .. code-block:: cpp

      register_exception_translator(
          [](const std::exception_ptr &p, void * /*payload*/) {
              try {
                  std::rethrow_exception(p);
              } catch (const ZeroDivisionException &e) {
                  PyErr_SetString(PyExc_ZeroDivisionError, e.what());
              }
          }, nullptr /*payload*/);

   Generally, you will want to use the more convenient exception binding
   interface provided by :cpp:class:`exception` class. This function provides
   an escape hatch for more specialized use cases.

.. cpp:function:: void chain_error(handle type, const char * fmt, ...) noexcept

   Raise a Python error of type ``type`` using the format string ``fmt``
   interpreted by ``PyErr_FormatV``.

   If a Python error state was already set prior to calling this method, then
   the new error is *chained* on top of the existing one. Otherwise, the
   function creates a new error without initializing its ``__cause__`` field.

.. cpp:function:: void raise_from(python_error &e, handle type, const char * fmt, ...)

   Convenience wrapper around :cpp:func:`chain_error <chain_error>`. It takes
   an existing Python error (e.g. caught in a ``catch`` block) and creates an
   additional Python exception with the current error as cause. It then
   re-raises :cpp:class:`python_error`. The argument ``fmt`` is a
   ``printf``-style format string interpreted by ``PyErr_FormatV``.

   Usage of this function is explained in the documentation section on
   :ref:`exception chaining <exception_chaining>`.

.. cpp:function:: void raise(const char * fmt, ...)

   This function takes a ``printf``-style format string with arguments and then
   raises a ``std::runtime_error`` with the formatted string. The function has
   no dependence on Python, and nanobind merely includes it for convenience.

.. cpp:function:: void raise_type_error(const char * fmt, ...)

   This function is analogous to :cpp:func:`raise`, except that it raises a
   :cpp:class:`builtin_exception` that will convert into a Python ``TypeError``
   when crossing the language interface.

.. cpp:function:: void raise_python_error()

   This function should only be called if a Python error status was set by a
   prior operation, which should now be raised as a C++ exception. The function
   is analogous to the statement ``throw python_error();`` but compiles into
   more compact code.

Casting
-------

.. cpp:function:: template <typename T, typename Derived> T cast(const detail::api<Derived> &value, bool convert = true)

   Convert the Python object `value` (typically a :cpp:class:`handle` or a
   :cpp:class:`object` subclass) into a C++ object of type `T`.

   When the `convert` argument is set to ``true`` (the default), the
   implementation may also attempt *implicit conversions* to perform the cast.

   The function raises a :cpp:type:`cast_error` when the conversion fails.
   See :cpp:func:`try_cast()` for an alternative that never raises.

.. cpp:function:: template <typename T, typename Derived> bool try_cast(const detail::api<Derived> &value, T &out, bool convert = true) noexcept

   Convert the Python object `value` (typically a :cpp:class:`handle` or a
   :cpp:class:`object` subclass) into a C++ object of type `T`, and store it
   in the output parameter `out`.

   When the `convert` argument is set to ``true`` (the default), the
   implementation may also attempt *implicit conversions* to perform the cast.

   The function returns ``false`` when the conversion fails. In this case, the
   `out` parameter is left untouched. See :cpp:func:`cast()` for an alternative
   that instead raises an exception in this case.

.. cpp:function:: template <typename T> object cast(T &&value, rv_policy policy = rv_policy::automatic_reference)

   Convert the C++ object ``value`` into a Python object. The return value
   policy `policy` is used to handle ownership-related questions when a new
   Python object must be created.

   The function raises a :cpp:type:`cast_error` when the conversion fails.

.. cpp:function:: template <typename T> object cast(T &&value, rv_policy policy, handle parent)

   Convert the C++ object ``value`` into a Python object. The return value
   policy `policy` is used to handle ownership-related questions when a new
   Python object must be created. A valid `parent` object is required when
   specifying a `reference_internal` return value policy.

   The function raises a :cpp:type:`cast_error` when the conversion fails.

.. cpp:function:: template <typename T> object find(const T &value) noexcept

   Return the Python object associated with the C++ instance `value`. When no
   such object can be found, the function it returns an invalid object
   (:cpp:func:`detail::api::is_valid()` is ``false``).

.. cpp:function:: template <rv_policy policy = rv_policy::automatic, typename... Args> tuple make_tuple(Args&&... args)

   Create a Python tuple from a sequence of C++ objects ``args...``. The return
   value policy `policy` is used to handle ownership-related questions when a
   new Python objects must be created.

   The function raises a :cpp:type:`cast_error` when the conversion fails.

Common binding annotations
--------------------------

The following annotations can be specified in both function and class bindings.

.. cpp:struct:: scope

   .. cpp:function:: scope(handle value)

      Captures the Python scope (e.g., a :cpp:class:`module_` or
      :cpp:class:`type_object`) in which the function or class should be
      registered.

.. _function_binding_annotations:

Function binding annotations
----------------------------

The following annotations can be specified using the variable-length ``Extra``
parameter of :cpp:func:`module_::def`, :cpp:func:`class_::def`,
:cpp:func:`cpp_function`, etc.

.. cpp:struct:: name

   .. cpp:function:: name(const char * value)

      Specify this annotation to override the name of the function.

      nanobind will internally copy the string when creating a function
      binding, hence dynamically generated arguments with a limited lifetime
      are legal.

.. cpp:struct:: arg

   Function argument annotation to enable keyword-based calling, default
   arguments, passing ``None``, and implicit conversion hints. Note that when a
   function argument should be annotated, you *must* specify annotations for all
   arguments of that function.

   Example use:

   .. code-block:: cpp

       m.def("add", [](int a, int b) { return a + b; }, nb::arg("a"), nb::arg("b"));

   It is usually convenient to add the following ``using`` declaration to your binding code.

   .. code-block:: cpp

       using namespace nb::literals;

   In this case, the argument annotations can be shortened:

   .. code-block:: cpp

       m.def("add", [](int a, int b) { return a + b; }, "a"_a, "b"_a);

   .. cpp:function:: explicit arg(const char * name = nullptr)

      Create a function argument annotation. The name is optional.

   .. cpp:function:: template <typename T> arg_v operator=(T &&value) const

      Return an argument annotation that is like this one but also assigns a
      default value to the argument. The default will be converted into a Python
      object immediately, so its bindings must have already been defined.

   .. cpp:function:: arg &none(bool value = true)

      Set a flag noting that the function argument accepts ``None``. Can only
      be used for python wrapper types (e.g. :cpp:class:`handle`,
      :cpp:class:`int_`) and types that have been bound using
      :cpp:class:`class_`. You cannot use this to implement functions that
      accept null pointers to builtin C++ types like ``int *i = nullptr``.

   .. cpp:function:: arg &noconvert(bool value = true)

      Set a flag noting that implicit conversion should never be performed for
      this function argument.

   .. cpp:function:: arg &sig(const char * sig)

      Override the signature of the default argument value. This is useful when
      the argument value is unusually complex so that the default method to
      explain it in docstrings and stubs (``str(value)``) does not produce
      acceptable output.

   .. cpp:function:: arg_locked lock()

      Return an argument annotation that is like this one but also requests that
      this argument be locked when dispatching a function call in free-threaded
      Python extensions. It does nothing in regular GIL-protected extensions.

.. cpp:struct:: is_method

   Indicate that the bound function is a method.

.. cpp:struct:: is_operator

   Indicate that the bound operator represents a special double underscore
   method (``__add__``, ``__radd__``, etc.) that implements an arithmetic
   operation.

   When a bound functions with this annotation is called with incompatible
   arguments, it will return ``NotImplemented`` rather than raising a
   ``TypeError``.

.. cpp:struct:: is_implicit

   Indicate that the bound constructor can be used to perform implicit conversions.

.. cpp:struct:: lock_self

   Indicate that the implicit ``self`` argument of a method should be locked
   when dispatching a call in a free-threaded extension. This annotation does
   nothing in regular GIL-protected extensions.

.. cpp:struct:: template <typename... Ts> call_guard

   Invoke the call guard(s) `Ts` when the bound function executes. The RAII
   helper :cpp:struct:`gil_scoped_release` is often combined with this feature.

.. cpp:struct:: template <size_t Nurse, size_t Patient> keep_alive

   Following evaluation of the bound function, keep the object referenced by
   index ``Patient`` alive *as long as* the object with index ``Nurse`` exists.
   This uses the following indexing convention:

   - Index ``0`` refers to the return value of methods. It should not be used
     in constructors or functions that do not return a result.

   - Index ``1`` refers to the first argument. In methods and constructors,
     index ``1`` refers to the implicit ``this`` pointer, while regular
     arguments begin at index ``2``.

   The annotation has the following runtime characteristics:

    - It does nothing when the nurse or patient object are ``None``.

    - It raises an exception when the nurse object is neither
      weak-referenceable nor an instance of a binding created via
      :cpp:class:`nb::class_\<..\> <class_>`.

   Two additional caveats regarding :cpp:class:`keep_alive <keep_alive>` are
   noteworthy:

   - It *usually* doesn't make sense to specify a ``Nurse`` or ``Patient`` for an
     argument or return value handled by a :ref:`type caster <type_casters>`
     (e.g., a STL vector handled via the include directive ``#include
     <nanobind/stl/vector.h>``). That's because type casters copy-convert the
     Python object into an equivalent C++ object, whose lifetime is decoupled
     from the original Python object. However, the :cpp:class:`keep_alive
     <keep_alive>` annotation *only* affects the lifetime of Python objects
     *and not their C++ copy*.

   - Dispatching a Python → C++ function call may require the :ref:`implicit
     conversion <noconvert>` of function arguments. In this case, the objects
     passed to the C++ function differ from the originally specified arguments.
     The ``Nurse`` and ``Patient`` annotation always refer to the *final* object
     following implicit conversion.

.. cpp:struct:: sig

   .. cpp:function:: sig(const char * value)

      This is *both* a class and a function binding annotation.

      1. When used in functions bindings, it provides complete control over
         the function's type signature by replacing the automatically generated
         version with ``value``. You can use it to add or change arguments and
         return values, tweak how default values are rendered, and add custom
         decorators.

         Here is an example:

         .. code-block:: cpp

            nb::def("function_name", &function_name,
                    nb::sig(
                        "@decorator(decorator_args..)\n
                        "def function_name(arg_1: type_1 = def_1, ...) -> ret"
                    ));


      2. When used in class bindings, the annotation enables complete control
         over how the class is rendered by nanobind's ``stubgen`` program. You
         can use it add decorators, specify ``typing.TypeVar``-parameterized
         base classes, metaclasses, etc.

         Here is an example:

         .. code-block:: cpp

            nb::class_<Class>(m, "Class",
                              nb::sig(
                                  "@decorator(decorator_args..)\n"
                                  "class Class(Base1[T], Base2, meta=Meta)"
                              ));

      Deviating significantly from the nanobind-generated signature likely
      means that the class or function declaration is a *lie*, but such lies
      can be useful to type-check complex binding projects.

      Specifying decorators isn't required---the above are just examples to
      show that this is possible.

      nanobind will internally copy the signature during function/type
      creation, hence dynamically generated strings with a limited lifetime are
      legal.

      The provided string should be valid Python signature, but *without* a
      trailing colon (``":"``) or trailing newline. Furthermore, nanobind
      analyzes the string and expects to find the name of the function or class
      on the *last line* between the ``"def"`` / ``"class"`` prefix and the
      opening parenthesis.

      For function bindings, this name must match the specified function name
      in ``.def("name", ..)``-style binding declarations, and for class
      bindings, the specified name must match the ``name`` argument of
      :cpp:class:`nb::class_ <class_>`.

.. cpp:enum-class:: rv_policy

   A return value policy determines the question of *ownership* when a bound
   function returns a previously unknown C++ instance that must now be
   converted into a Python object.

   Return value policies apply to functions that return values handled using
   :ref:`class bindings <bindings>`, which means that their Python equivalent
   was registered using :cpp:class:`class_\<...\> <class_>`. They are ignored
   in most other cases. One exception are STL types handled using :ref:`type
   casters <type_casters>` (e.g. ``std::vector<T>``), which contain a nested
   type ``T`` handled using class bindings. In this case, the return value
   policy also applies recursively.

   A return value policy is unnecessary when the type itself clarifies
   ownership (e.g., ``std::unique_ptr<T>``, ``std::shared_ptr<T>``, a type with
   :ref:`intrusive reference counting <intrusive>`).

   The following policies are available (where `automatic` is the default).
   Please refer to the :ref:`return value policy section <rvp>` of the main
   documentation, which clarifies the list below using concrete examples.

   .. cpp:enumerator:: take_ownership

      Create a Python object that wraps the existing C++ instance and takes
      full ownership of it. No copies are made. Python will call the C++
      destructor and ``delete`` operator when the Python wrapper is garbage
      collected at some later point. The C++ side *must* relinquish ownership
      and is not allowed to destruct the instance, or undefined behavior will
      ensue.

   .. cpp:enumerator:: copy

      Copy-construct a new Python object from the C++ instance. The new copy
      will be owned by Python, while C++ retains ownership of the original.

   .. cpp:enumerator:: move

      Move-construct a new Python object from the C++ instance. The new object
      will be owned by Python, while C++ retains ownership of the original
      (whose contents were likely invalidated by the move operation).

   .. cpp:enumerator:: reference

      Create a Python object that wraps the existing C++ instance *without
      taking ownership* of it. No copies are made. Python will never call the
      destructor or ``delete`` operator, even when the Python wrapper is
      garbage collected.

   .. cpp:enumerator:: reference_internal

      A safe extension of the `reference` policy for methods that implement
      some form of attribute access. It creates a Python object that wraps the
      existing C++ instance *without taking ownership* of it. Additionally, it
      adjusts reference counts to keeps the method's implicit ``self`` argument
      alive until the newly created object has been garbage collected.

   .. cpp:enumerator:: none

      This is the most conservative policy: it simply refuses the cast unless
      the C++ instance already has a corresponding Python object, in which case
      the question of ownership becomes moot.

   .. cpp:enumerator:: automatic

      This is the default return value policy, which falls back to
      `take_ownership` when the return value is a pointer, `move`  when it is a
      rvalue reference, and `copy` when it is a lvalue reference.

   .. cpp:enumerator:: automatic_reference

      This policy matches `automatic` but falls back to `reference` when the
      return value is a pointer.

.. cpp:struct:: kw_only

   Indicate that all following function parameters are keyword-only. This
   may only be used if you supply an :cpp:struct:`arg` annotation for each
   parameters, because keyword-only parameters are useless if they don't have
   names. For example, if you write

   .. code-block:: cpp

      int some_func(int one, const char* two);

      m.def("some_func", &some_func,
            nb::arg("one"), nb::kw_only(), nb::arg("two"));

   then in Python you can write ``some_func(42, two="hi")``, or
   ``some_func(one=42, two="hi")``, but not ``some_func(42, "hi")``.

   Just like in Python, any parameters appearing after variadic
   :cpp:class:`*args <args>` are implicitly keyword-only. You don't
   need to include the :cpp:struct:`kw_only` annotation in this case,
   but if you do include it, it must be in the correct position:
   immediately after the :cpp:struct:`arg` annotation for the variadic
   :cpp:class:`*args <args>` parameter.

.. cpp:struct:: template <typename T> for_getter

   When defining a property with a getter and a setter, you can use this to
   only pass a function binding attribute to the getter part. An example is
   shown below.

   .. code-block:: cpp

      nb::class_<MyClass>(m, "MyClass")
        .def_prop_rw("value", &MyClass::value,
                nb::for_getter(nb::sig("def value(self, /) -> int")),
                nb::for_setter(nb::sig("def value(self, value: int, /) -> None")),
                nb::for_getter("docstring for getter"),
                nb::for_setter("docstring for setter"));

.. cpp:struct:: template <typename T> for_setter

   Analogous to :cpp:struct:`for_getter`, but for setters.

.. cpp:struct:: template <typename Policy> call_policy

   Request that custom logic be inserted around each call to the
   bound function, by calling ``Policy::precall(args, nargs, cleanup)`` before
   Python-to-C++ argument conversion, and ``Policy::postcall(args, nargs, ret)``
   after C++-to-Python return value conversion.

   If multiple call policy annotations are provided for the same function, then
   their precall and postcall hooks will both execute left-to-right according
   to the order in which the annotations were specified when binding the
   function.

   The :cpp:struct:`nb::call_guard\<T\>() <call_guard>` annotation
   should be preferred over ``call_policy`` unless the wrapper logic
   depends on the function arguments or return value.
   If both annotations are combined, then
   :cpp:struct:`nb::call_guard\<T\>() <call_guard>` always executes on
   the "inside" (closest to the bound function, after argument
   conversions and before return value conversion) regardless of its
   position in the function annotations list.

   Your ``Policy`` class must define two static member functions:

   .. cpp:function:: static void precall(PyObject **args, size_t nargs, detail::cleanup_list *cleanup);

      A hook that will be invoked before calling the bound function. More
      precisely, it is called after any :ref:`argument locks <argument-locks>`
      have been obtained, but before the Python arguments are converted to C++
      objects for the function call.

      This hook may access or modify the function arguments using the
      *args* array, which holds borrowed references in one-to-one
      correspondence with the C++ arguments of the bound function.  If
      the bound function is a method, then ``args[0]`` is its *self*
      argument. *nargs* is the number of function arguments. It is actually
      passed as ``std::integral_constant<size_t, N>()``, so you can
      match on that type if you want to do compile-time checks with it.

      The *cleanup* list may be used as it is used in type casters,
      to cause some Python object references to be released at some point
      after the bound function completes. (If the bound function is part
      of an overload set, the cleanup list isn't released until all overloads
      have been tried.)

      ``precall()`` may choose to throw a C++ exception. If it does,
      it will preempt execution of the bound function, and the
      exception will be treated as if the bound function had thrown it.

   .. cpp:function:: static void postcall(PyObject **args, size_t nargs, handle ret);

      A hook that will be invoked after calling the bound function and
      converting its return value to a Python object, but only if the
      bound function returned normally.

      *args* stores the Python object arguments, with the same semantics
      as in ``precall()``, except that arguments that participated in
      implicit conversions will have had their ``args[i]`` pointer updated
      to reflect the new Python object that the implicit conversion produced.
      *nargs* is the number of arguments, passed as a ``std::integral_constant``
      in the same way as for ``precall()``.

      *ret* is the bound function's return value. If the bound function returned
      normally but its C++ return value could not be converted to a Python
      object, then ``postcall()`` will execute with *ret* set to null,
      and the Python error indicator might or might not be set to explain why.

      If the bound function did not return normally -- either because its
      Python object arguments couldn't be converted to the appropriate C++
      types, or because the C++ function threw an exception -- then
      ``postcall()`` **will not execute**. If you need some cleanup logic to
      run even in such cases, your ``precall()`` can add a capsule object to the
      cleanup list; its destructor will run eventually, but with no promises
      as to when. A :cpp:struct:`nb::call_guard <call_guard>` might be a
      better choice.

      ``postcall()`` may choose to throw a C++ exception. If it does,
      the result of the wrapped function will be destroyed,
      and the exception will be raised in its place, as if the bound function
      had thrown it just before returning.

   Here is an example policy to demonstrate.
   ``nb::call_policy<returns_references_to<I>>()`` behaves like
   :cpp:class:`nb::keep_alive\<0, I\>() <keep_alive>`, except that the
   return value is a treated as a list of objects rather than a single one.

   .. code-block:: cpp

      template <size_t I>
      struct returns_references_to {
          static void precall(PyObject **, size_t, nb::detail::cleanup_list *) {}

          template <size_t N>
          static void postcall(PyObject **args,
                               std::integral_constant<size_t, N>,
                               nb::handle ret) {
              static_assert(I > 0 && I < N,
                            "I in returns_references_to<I> must be in the "
                            "range [1, number of C++ function arguments]");
              if (!nb::isinstance<nb::sequence>(ret)) {
                  throw std::runtime_error("return value should be a sequence");
              }
              for (nb::handle nurse : ret) {
                  nb::detail::keep_alive(nurse.ptr(), args[I]);
              }
          }
      };

   For a more complex example (binding an object that uses trivially-copyable
   callbacks), see ``tests/test_callbacks.cpp`` in the nanobind source
   distribution.

.. _class_binding_annotations:

Class binding annotations
-------------------------

The following annotations can be specified using the variable-length ``Extra``
parameter of the constructor :cpp:func:`class_::class_`.

Besides the below options, also refer to the :cpp:class:`sig` which is
usable in both function and class bindings. It can be used to override class
declarations in generated :ref:`stubs <stubs>`,

.. cpp:struct:: is_final

   Indicate that a type cannot be subclassed.

.. cpp:struct:: dynamic_attr

   Indicate that instances of a type require a Python dictionary to support the dynamic addition of attributes.

.. cpp:struct:: is_weak_referenceable

   Indicate that instances of a type require a weak reference list so that they
   can be referenced by the Python ``weakref.*`` types.

.. cpp:struct:: is_generic

   If present, nanobind will add a ``__class_getitem__`` function to the newly
   created type that permits constructing *parameterized* versions (e.g.,
   ``MyType[int]``). The implementation of this function is equivalent to

   .. code-block:: python

      def __class_getitem__(cls, value):
          import types
          return types.GenericAlias(cls, value)

   See the section on :ref:`creating generic types <typing_generics_creating>`
   for an example.

   This feature is only supported on Python 3.9+. Nanobind will ignore
   the attribute in Python 3.8 builds.

.. cpp:struct:: template <typename T> supplement

   Indicate that ``sizeof(T)`` bytes of memory should be set aside to
   store supplemental data in the type object. See :ref:`Supplemental
   type data <supplement>` for more information.

.. cpp:struct:: type_slots

   .. cpp:function:: type_slots(PyType_Slot * value)

   nanobind uses the ``PyType_FromSpec`` Python C API interface to construct
   types. In certain advanced use cases, it may be helpful to append additional
   type slots during type construction. This class binding annotation can be
   used to accomplish this. The provided list should be followed by a
   zero-initialized ``PyType_Slot`` element. See :ref:`Customizing type creation
   <typeslots>` for more information about this feature.

.. cpp:struct:: template <typename T> intrusive_ptr

   nanobind provides a custom interface for intrusive reference-counted C++
   types that nicely integrate with Python reference counting. See the
   :ref:`separate section <intrusive>` on this topic. This annotation
   marks a type as compatible with this interface.

   .. cpp:function:: intrusive_ptr(void (* set_self_py)(T*, PyObject*) noexcept)

      Declares a callback that will be invoked when a C++ instance is first
      cast into a Python object.


.. _enum_binding_annotations:

Enum binding annotations
------------------------

The following annotations can be specified using the variable-length
``Extra`` parameter of the constructor :cpp:func:`enum_::enum_`.

.. cpp:struct:: is_arithmetic

   Indicate that the enumeration supports arithmetic operations.  This enables
   both unary (``-``, ``~``, ``abs()``) and binary (``+``, ``-``, ``*``,
   ``//``, ``&``, ``|``, ``^``, ``<<``, ``>>``) operations with operands of
   either enumeration or numeric types.

   The result will be as if the operands were first converted to integers. (So
   ``Shape(2) + Shape(1) == 3`` and ``Shape(2) * 1.5 == 3.0``.) It is
   unspecified whether operations on mixed enum types (such as ``Shape.Circle +
   Color.Red``) are permissible.

   Passing this annotation changes the Python enumeration parent class to
   either :py:class:`enum.IntEnum` or :py:class:`enum.IntFlag`, depending on
   whether or not the flag enumeration attribute is also specified (see
   :cpp:class:`is_flag`).

.. cpp:struct:: is_flag

   Indicate that the enumeration supports bit-wise operations.  This enables the
   operators (``|``, ``&``, ``^``, and ``~``) with two enumerators as operands.

   The result has the same type as the operands, i.e., ``Shape(2) | Shape(1)``
   will be equivalent to ``Shape(3)``.

   Passing this annotation changes the Python enumeration parent class to
   either :py:class:`enum.IntFlag` or :py:class:`enum.Flag`, depending on
   whether or not the enumeration is also marked to support arithmetic
   operations (see :cpp:class:`is_arithmetic`).

Function binding
----------------

.. cpp:function:: object cpp_function(Func &&f, const Extra&... extra)

   Convert the function `f` into a Python callable. This function has
   a few overloads (not shown here) to separately deal with function/method
   pointers and lambda functions.

   The variable length `extra` parameter can be used to pass a docstring and
   other :ref:`function binding annotations <function_binding_annotations>`.

.. _class_binding:

Class binding
-------------

.. cpp:class:: template <typename T, typename... Ts> class_ : public object

   Binding helper class to expose a custom C++ type `T` (declared using either
   the ``class`` or ``struct`` keyword) in Python.

   The variable length parameter `Ts` is optional and  can be used to specify
   the base class of `T` and/or an alias needed to realize :ref:`trampoline
   classes <trampolines>`.

   When the type ``T`` was previously already registered (either within the
   same extension or another extension), the ``class_<..>`` declaration is
   redundant. nanobind will print a warning message in this case:

   .. code-block:: text

      RuntimeWarning: nanobind: type 'MyType' was already registered!

   The ``class_<..>`` instance will subsequently wrap the original type object
   instead of creating a new one.

   .. cpp:function:: template <typename... Extra> class_(handle scope, const char * name, const Extra &... extra)

      Bind the type `T` to the identifier `name` within the scope `scope`. The
      variable length `extra` parameter can be used to pass a docstring and
      other :ref:`class binding annotations <class_binding_annotations>`.

   .. cpp:function:: template <typename Func, typename... Extra> class_ &def(const char * name, Func &&f, const Extra &... extra)

      Bind the function `f` and assign it to the class member `name`.
      The variable length `extra` parameter can be used to pass a docstring and
      other :ref:`function binding annotations <function_binding_annotations>`.

      This function has two overloads (listed just below) to handle constructor
      binding declarations.

      **Example**:

      .. code-block:: cpp

         struct A {
             void f() { /*...*/ }
         };

         nb::class_<A>(m, "A")
             .def(nb::init<>()) // Bind the default constructor
             .def("f", &A::f);  // Bind the method A::f

   .. cpp:function:: template <typename... Args, typename... Extra> class_ &def(init<Args...> arg, const Extra &... extra)

      Bind a constructor. The variable length `extra` parameter can be used to
      pass a docstring and other :ref:`function binding annotations
      <function_binding_annotations>`.

   .. cpp:function:: template <typename Arg, typename... Extra> class_ &def(init_implicit<Arg> arg, const Extra &... extra)

      Bind a constructor that may be used for implicit type conversions. The
      constructor must take a single argument of an unspecified type `Arg`.

      When nanobind later tries to dispatch a function call requiring an
      argument of type `T` while `Arg` was actually provided, it will run this
      constructor to perform the necessary conversion.

      The variable length `extra` parameter can be used to pass a docstring and
      other :ref:`function binding annotations <function_binding_annotations>`.

      This constructor generates more compact code than a separate call to
      :cpp:func:`implicitly_convertible`, but is otherwise equivalent.

   .. cpp:function:: template <typename Func, typename... Extra> class_ &def(new_<Func> arg, const Extra &... extra)

      Bind a C++ factory function as a Python object constructor (``__new__``).
      This is an advanced feature; prefer :cpp:struct:`nb::init\<..\> <init>`
      where possible. See the discussion of :ref:`customizing object creation
      <custom_new>` for more details.

   .. cpp:function:: template <typename C, typename D, typename... Extra> class_ &def_rw(const char * name, D C::* p, const Extra &...extra)

      Bind the field `p` and assign it to the class member `name`. nanobind
      constructs a ``property`` object with *read-write* access (hence the
      ``rw`` suffix) to do so.

      Every access from Python will read from or write to the C++ field while
      performing a suitable conversion (using :ref:`type casters
      <type_casters>`, :ref:`bindings <bindings>`, or :ref:`wrappers
      <wrappers>`) as determined by its type.

      The variable length `extra` parameter can be used to pass a docstring and
      other :ref:`function binding annotations <function_binding_annotations>`
      that are forwarded to the anonymous functions used to construct the
      property.
      Use the :cpp:struct:`nb::for_getter <for_getter>` and
      :cpp:struct:`nb::for_setter <for_setter>` to pass annotations
      specifically to the setter or getter part.

      **Example**:

      .. code-block:: cpp

         struct A { int value; };

         nb::class_<A>(m, "A")
             .def_rw("value", &A::value); // Enable mutable access to the field A::value

   .. cpp:function:: template <typename C, typename D, typename... Extra> class_ &def_ro(const char * name, D C::* p, const Extra &...extra)

      Bind the field `p` and assign it to the class member `name`. nanobind
      constructs a ``property`` object with *read only* access (hence the
      ``ro`` suffix) to do so.

      Every access from Python will read the C++ field while performing a
      suitable conversion (using :ref:`type casters <type_casters>`,
      :ref:`bindings <bindings>`, or :ref:`wrappers <wrappers>`) as determined
      by its type.

      The variable length `extra` parameter can be used to pass a docstring and
      other :ref:`function binding annotations <function_binding_annotations>`
      that are forwarded to the anonymous functions used to construct the
      property.

      **Example**:

      .. code-block:: cpp

         struct A { int value; };

         nb::class_<A>(m, "A")
             .def_ro("value", &A::value);  // Enable read-only access to the field A::value

   .. cpp:function:: template <typename Getter, typename Setter, typename... Extra> class_ &def_prop_rw(const char * name, Getter &&getter, Setter &&setter, const Extra &...extra)

      Construct a *mutable* (hence the ``rw`` suffix) Python ``property`` and
      assign it to the class member `name`. Every read access will call the
      function ``getter``  with the `T` instance, and every write access will
      call the ``setter`` with the `T` instance and value to be assigned.

      The variable length `extra` parameter can be used to pass a docstring and
      other :ref:`function binding annotations <function_binding_annotations>`.
      Use the :cpp:struct:`nb::for_getter <for_getter>` and
      :cpp:struct:`nb::for_setter <for_setter>` to pass annotations
      specifically to the setter or getter part.

      Note that this function implicitly assigns the
      :cpp:enumerator:`rv_policy::reference_internal` return value policy to
      `getter` (as opposed to the usual
      :cpp:enumerator:`rv_policy::automatic`). Provide an explicit return value
      policy as part of the `extra` argument to override this.

      **Example**: the example below uses `def_prop_rw` to expose a C++
      setter/getter pair as a more "Pythonic" property:

      .. code-block:: cpp

          class A {
          public:
              A(int value) : m_value(value) { }
              void set_value(int value) { m_value = value; }
              int value() const { return m_value; }
          private:
              int m_value;
          };

          nb::class_<A>(m, "A")
              .def(nb::init<int>())
              .def_prop_rw("value",
                  [](A &t) { return t.value() ; },
                  [](A &t, int value) { t.set_value(value); });

   .. cpp:function:: template <typename Getter, typename... Extra> class_ &def_prop_ro(const char * name, Getter &&getter, const Extra &...extra)

      Construct a *read-only* (hence the ``ro`` suffix) Python ``property`` and
      assign it to the class member `name`. Every read access will call the
      function ``getter``  with the `T` instance.

      The variable length `extra` parameter can be used to pass a docstring and
      other :ref:`function binding annotations <function_binding_annotations>`.

      Note that this function implicitly assigns the
      :cpp:enumerator:`rv_policy::reference_internal` return value policy to
      `getter` (as opposed to the usual
      :cpp:enumerator:`rv_policy::automatic`). Provide an explicit return value
      policy as part of the `extra` argument to override this.

      **Example**: the example below uses `def_prop_ro` to expose a C++ getter
      as a more "Pythonic" property:

      .. code-block:: cpp

          class A {
          public:
              A(int value) : m_value(value) { }
              int value() const { return m_value; }
          private:
              int m_value;
          };

          nb::class_<A>(m, "A")
              .def(nb::init<int>())
              .def_prop_ro("value",
                  [](A &t) { return t.value() ; });

   .. cpp:function:: template <typename Func, typename... Extra> class_ &def_static(const char * name, Func &&f, const Extra &... extra)

      Bind the *static* function `f` and assign it to the class member `name`.
      The variable length `extra` parameter can be used to pass a docstring and
      other :ref:`function binding annotations <function_binding_annotations>`.

      **Example**:

      .. code-block:: cpp

         struct A {
             static void f() { /*...*/ }
         };

         nb::class_<A>(m, "A")
             .def_static("f", &A::f);  // Bind the static method A::f

   .. cpp:function:: template <typename D, typename... Extra> class_ &def_rw_static(const char * name, D* p, const Extra &...extra)

      Bind the *static* field `p` and assign it to the class member `name`. nanobind
      constructs a class ``property`` object with *read-write* access (hence the
      ``rw`` suffix) to do so.

      Every access from Python will read from or write to the static C++ field
      while performing a suitable conversion (using :ref:`type casters
      <type_casters>`, :ref:`bindings <bindings>`, or :ref:`wrappers
      <wrappers>`) as determined by its type.

      The variable length `extra` parameter can be used to pass a docstring and
      other :ref:`function binding annotations <function_binding_annotations>`
      that are forwarded to the anonymous functions used to construct the
      property
      Use the :cpp:struct:`nb::for_getter <for_getter>` and
      :cpp:struct:`nb::for_setter <for_setter>` to pass annotations
      specifically to the setter or getter part.

      **Example**:

      .. code-block:: cpp

         struct A { inline static int value = 5; };

         nb::class_<A>(m, "A")
             // Enable mutable access to the static field A::value
             .def_rw_static("value", &A::value);

   .. cpp:function:: template <typename D, typename... Extra> class_ &def_ro_static(const char * name, D* p, const Extra &...extra)

      Bind the *static* field `p` and assign it to the class member `name`.
      nanobind constructs a class ``property`` object with *read-only* access
      (hence the ``ro`` suffix) to do so.

      Every access from Python will read the static C++ field while performing
      a suitable conversion (using :ref:`type casters <type_casters>`,
      :ref:`bindings <bindings>`, or :ref:`wrappers <wrappers>`) as determined
      by its type.

      The variable length `extra` parameter can be used to pass a docstring and
      other :ref:`function binding annotations <function_binding_annotations>`
      that are forwarded to the anonymous functions used to construct the
      property

      **Example**:

      .. code-block:: cpp

         struct A { inline static int value = 5; };

         nb::class_<A>(m, "A")
             // Enable read-only access to the static field A::value
             .def_ro_static("value", &A::value);

   .. cpp:function:: template <typename Getter, typename Setter, typename... Extra> class_ &def_prop_rw_static(const char * name, Getter &&getter, Setter &&setter, const Extra &...extra)

      Construct a *mutable* (hence the ``rw`` suffix) Python ``property`` and
      assign it to the class member `name`. Every read access will call the
      function ``getter``  with `T`'s Python type object, and every write access will
      call the ``setter`` with `T`'s Python type object and value to be assigned.

      The variable length `extra` parameter can be used to pass a docstring and
      other :ref:`function binding annotations <function_binding_annotations>`.
      Use the :cpp:struct:`nb::for_getter <for_getter>` and
      :cpp:struct:`nb::for_setter <for_setter>` to pass annotations
      specifically to the setter or getter part.

      Note that this function implicitly assigns the
      :cpp:enumerator:`rv_policy::reference` return value policy to
      `getter` (as opposed to the usual
      :cpp:enumerator:`rv_policy::automatic`). Provide an explicit return value
      policy as part of the `extra` argument to override this.

      **Example**: the example below uses `def_prop_rw_static` to expose a
      static C++ setter/getter pair as a more "Pythonic" property:

      .. code-block:: cpp

         class A {
         public:
            static void set_value(int value) { s_value = value; }
            static int value() { return s_value; }
         private:
            inline static int s_value = 5;
         };

         nb::class_<A>(m, "A")
             .def_prop_rw_static("value",
                 [](nb::handle /*unused*/) { return A::value() ; },
                 [](nb::handle /*unused*/, int value) { A::set_value(value); });

   .. cpp:function:: template <typename Getter, typename... Extra> class_ &def_prop_ro_static(const char * name, Getter &&getter, const Extra &...extra)

      Construct a *read-only* (hence the ``ro`` suffix) Python ``property`` and
      assign it to the class member `name`. Every read access will call the
      function ``getter``  with `T`'s Python type object.

      The variable length `extra` parameter can be used to pass a docstring and
      other :ref:`function binding annotations <function_binding_annotations>`.

      Note that this function implicitly assigns the
      :cpp:enumerator:`rv_policy::reference` return value policy to
      `getter` (as opposed to the usual
      :cpp:enumerator:`rv_policy::automatic`). Provide an explicit return value
      policy as part of the `extra` argument to override this.

      **Example**: the example below uses `def_prop_ro_static` to expose a
      static C++ getter as a more "Pythonic" property:

      .. code-block:: cpp

         class A {
         public:
            static int value() { return s_value; }
         private:
            inline static int s_value = 5;
         };

         nb::class_<A>(m, "A")
             .def_prop_ro_static("value",
                 [](nb::handle /*unused*/) { return A::value() ; });

   .. cpp:function:: template <detail::op_id id, detail::op_type ot, typename L, typename R, typename... Extra> class_ &def(const detail::op_<id, ot, L, R> &op, const Extra&... extra)

      This interface provides convenient syntax sugar to replace relatively
      lengthy method bindings with shorter operator bindings. To use it, you
      will need an extra include directive:

      .. code-block:: cpp

         #include <nanobind/operators.h>

      Below is an example type with three arithmetic operators in C++ (unary
      negation and 2 binary subtraction overloads) along with corresponding
      bindings.

      **Example**:

      .. code-block:: cpp

         struct A {
            float value;

            A operator-() const { return { -value }; }
            A operator-(const A &o) const { return { value - o.value }; }
            A operator-(float o) const { return { value - o }; }
         };

         nb::class_<A>(m, "A")
             .def(nb::init<float>())
             .def(-nb::self)
             .def(nb::self - nb::self)
             .def(nb::self - float());


      Bind an arithmetic or comparison operator expressed in short-hand form (e.g., ``.def(nb::self + nb::self)``).

   .. cpp:function:: template <detail::op_id id, detail::op_type ot, typename L, typename R, typename... Extra> class_ &def_cast(const detail::op_<id, ot, L, R> &op, const Extra&... extra)

      Like the above ``.def()`` variant, but furthermore cast the result of the operation back to `T`.


.. cpp:class:: template <typename T> enum_ : public class_<T>

   Class binding helper for scoped and unscoped C++ enumerations.

   .. cpp:function:: template <typename... Extra> NB_INLINE enum_(handle scope, const char * name, const Extra &...extra)

      Bind the enumeration of type `T` to the identifier `name` within the
      scope `scope`. The variable length `extra` parameter can be used to pass
      a docstring and other :ref:`enum binding annotations
      <enum_binding_annotations>` (currently, only :cpp:class:`is_arithmetic` is supported).

   .. cpp:function:: enum_ &value(const char * name, T value, const char * doc = nullptr)

      Add the entry `value` to the enumeration using the identifier `name`,
      potentially with a docstring provided via `doc` (optional).

   .. cpp:function:: enum_ &export_values()

      Export all entries of the enumeration into the parent scope.

.. cpp:class:: template <typename T> exception : public object

   Class binding helper for declaring new Python exception types

   .. cpp:function:: exception(handle scope, const char * name, handle base = PyExc_Exception)

      Create a new exception type identified by `name` that derives from
      `base`, and install it in `scope`. The constructor also calls
      :cpp:func:`register_exception_translator()` to register a new exception
      translator that converts caught C++ exceptions of type `T` into the
      newly created Python equivalent.

.. cpp:struct:: template <typename... Args> init

   nanobind uses this simple helper class to capture the signature of a
   constructor. It is only meant to be used in binding declarations done via
   :cpp:func:`class_::def()`.

   Sometimes, it is necessary to bind constructors that don't exist in the
   underlying C++ type (meaning that they are specific to the Python bindings).
   Because `init` only works for existing C++ constructors, this requires
   a manual workaround noting that

   .. code-block:: cpp

      nb::class_<MyType>(m, "MyType")
          .def(nb::init<const char*, int>());

   is syntax sugar for the following lower-level implementation using
   "`placement new <https://en.wikipedia.org/wiki/Placement_syntax>`_":

   .. code-block:: cpp

      nb::class_<MyType>(m, "MyType")
          .def("__init__",
               [](MyType* t, const char* arg0, int arg1) {
                   new (t) MyType(arg0, arg1);
               });

   The provided lambda function will be called with a pointer to uninitialized
   memory that has already been allocated (this memory region is co-located
   with the Python object for reasons of efficiency). The lambda function can
   then either run an in-place constructor and return normally (in which case
   the instance is assumed to be correctly constructed) or fail by raising an
   exception.

.. cpp:struct:: template <typename Arg> init_implicit

   See :cpp:class:`init` for detail on binding constructors. The main
   difference between :cpp:class:`init`  and `init_implicit` is that the latter
   only supports constructors taking a single argument `Arg`, and that it marks
   the constructor as usable for implicit conversions from `Arg`.

   Sometimes, it is necessary to bind implicit conversion-capable constructors
   that don't exist in the underlying C++ type (meaning that they are specific
   to the Python bindings). This can be done manually noting that

   .. code-block:: cpp

      nb::class_<MyType>(m, "MyType")
          .def(nb::init_implicit<const char*>());

   can be replaced by the lower-level code

   .. code-block:: cpp

       nb::class_<MyType>(m, "MyType")
           .def("__init__",
                [](MyType* t, const char* arg0) {
                    new (t) MyType(arg0);
                });

       nb::implicitly_convertible<const char*, MyType>();

.. cpp:struct:: template <typename Func> new_

   This is a small helper class that indicates to :cpp:func:`class_::def()`
   that a particular lambda or static method provides a Python object
   constructor (``__new__``) for the class being bound. Normally, you would
   use :cpp:class:`init` instead if possible, in order to cooperate with
   nanobind's usual object creation process. Using :cpp:class:`new_`
   replaces that process entirely. This is principally useful when some
   C++ type of interest can only provide pointers to its instances,
   rather than allowing them to be constructed directly.

   Like :cpp:class:`init`, the only use of a :cpp:class:`new_` object is
   as an argument to :cpp:func:`class_::def()`.

   Example use:

   .. code-block:: cpp

      class MyType {
      private:
          MyType();
      public:
          static std::shared_ptr<MyType> create();
          int value = 0;
      };

      nb::class_<MyType>(m, "MyType")
          .def(nb::new_(&MyType::create));

   Given this example code, writing ``MyType()`` in Python would
   produce a Python object wrapping the result of ``MyType::create()``
   in C++. If multiple calls to ``create()`` return pointers to the
   same C++ object, these will turn into references to the same Python
   object as well.

   See the discussion of :ref:`customizing Python object creation <custom_new>`
   for more information.


GIL Management
--------------

These two `RAII
<https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization>`_ helper
classes acquire and release the *Global Interpreter Lock* (GIL) in a given
scope. The :cpp:struct:`gil_scoped_release` helper is often combined with the
:cpp:struct:`call_guard`, as in

.. code-block:: cpp

    m.def("expensive", &expensive, nb::call_guard<nb::gil_scoped_release>());

This releases the interpreter lock while `expensive` is running, which permits
running it in parallel from multiple Python threads.

.. cpp:struct:: gil_scoped_acquire

   .. cpp:function:: gil_scoped_acquire()

      Acquire the GIL

   .. cpp:function:: ~gil_scoped_acquire()

      Release the GIL

.. cpp:struct:: gil_scoped_release

   .. cpp:function:: gil_scoped_release()

      Release the GIL (**must** be currently held)

      In :ref:`free-threaded extensions <free-threaded>`, this operation also
      temporarily releases all :ref:`argument locks <argument-locks>` held by
      the current thread.

   .. cpp:function:: ~gil_scoped_release()

      Reacquire the GIL

Free-threading
--------------

Nanobind provides abstractions to implement *additional* locking that is
needed to ensure the correctness of free-threaded Python extensions.

.. cpp:struct:: ft_mutex

   Object-oriented wrapper representing a `PyMutex
   <https://docs.python.org/3.13/c-api/init.html#c.PyMutex>`__. It can be
   slightly more efficient than OS/language-provided primitives (e.g.,
   ``std::thread``, ``pthread_mutex_t``) and should generally be preferred when
   adding critical sections to Python bindings.

   In Python builds *without* free-threading, this class does nothing. It has
   no attributes and the :cpp:func:`lock` and :cpp:func:`unlock` functions
   return immediately.

   .. cpp:function:: ft_mutex()

      Create a new (unlocked) mutex.

   .. cpp:function:: void lock()

      Acquire the mutex.

   .. cpp:function:: void unlock()

      Release the mutex.

.. cpp:struct:: ft_lock_guard

   This class provides a RAII lock guard analogous to ``std::lock_guard`` and
   ``std::unique_lock``.

   .. cpp:function:: ft_lock_guard(ft_mutex &mutex)

      Call :cpp:func:`mutex.lock() <ft_mutex::lock>` (no-op in non-free-threaded builds).

   .. cpp:function:: ~ft_lock_guard()

      Call :cpp:func:`mutex.unlock() <ft_mutex::unlock>` (no-op in non-free-threaded builds).

.. cpp:struct:: ft_object_guard

   This class provides a RAII guard that locks a single Python object within a
   local scope (in contrast to :cpp:class:`ft_lock_guard`, which locks a
   mutex).

   It is a thin wrapper around the Python `critical section API
   <https://docs.python.org/3.13/c-api/init.html#c.Py_BEGIN_CRITICAL_SECTION>`__.
   Please refer to the Python documentation for details on the semantics of
   this relaxed form of critical section (in particular, Python critical sections
   may release previously held locks).

   In Python builds *without* free-threading, this class does nothing---the
   constructor and destructor return immediately.

   .. cpp:function:: ft_object_guard(handle h)

      Lock the object ``h`` (no-op in non-free-threaded builds)

   .. cpp:function:: ~ft_object_guard()

      Unlock the object ``h`` (no-op in non-free-threaded builds)

.. cpp:struct:: ft_object2_guard

   This class provides a RAII guard that locks *two* Python object within a
   local scope (in contrast to :cpp:class:`ft_lock_guard`, which locks a
   mutex).

   It is a thin wrapper around the Python `critical section API
   <https://docs.python.org/3.13/c-api/init.html#c.Py_BEGIN_CRITICAL_SECTION2>`__.
   Please refer to the Python documentation for details on the semantics of
   this relaxed form of critical section (in particular, Python critical sections
   may release previously held locks).

   In Python builds *without* free-threading, this class does nothing---the
   constructor and destructor return immediately.

   .. cpp:function:: ft_object2_guard(handle h1, handle h2)

      Lock the objects ``h1`` and ``h2`` (no-op in non-free-threaded builds)

   .. cpp:function:: ~ft_object2_guard()

      Unlock the objects ``h1`` and ``h2`` (no-op in non-free-threaded builds)

Low-level type and instance access
----------------------------------

nanobind exposes a low-level interface to provide fine-grained control over
the sequence of steps that instantiates a Python object wrapping a C++
instance. A thorough explanation of these features is provided in a
:ref:`separate section <lowlevel>`.

Type objects
^^^^^^^^^^^^

.. cpp:function:: bool type_check(handle h)

   Returns ``true`` if ``h`` is a type that was previously bound via
   :cpp:class:`class_`.

.. cpp:function:: size_t type_size(handle h)

   Assuming that `h` represents a bound type (see :cpp:func:`type_check`),
   return its size in bytes.

.. cpp:function:: size_t type_align(handle h)

   Assuming that `h` represents a bound type (see :cpp:func:`type_check`),
   return its alignment in bytes.

.. cpp:function:: const std::type_info& type_info(handle h)

   Assuming that `h` represents a bound type (see :cpp:func:`type_check`),
   return its C++ RTTI record.

.. cpp:function:: template <typename T> T &type_supplement(handle h)

   Return a reference to supplemental data stashed in a type object.
   The type ``T`` must exactly match the type specified in the
   :cpp:class:`nb::supplement\<T\> <supplement>` annotation used when
   creating the type; no type check is performed, and invalid supplement
   accesses may crash the interpreter. Also refer to
   :cpp:class:`nb::supplement\<T\> <supplement>`.

.. cpp:function:: str type_name(handle h)

   Return the full (module-qualified) name of a type object as a Python string.

.. cpp:function:: void * type_get_slot(handle h, int slot_id)

   On Python 3.10+, this function is a simple wrapper around the Python C API
   function ``PyType_GetSlot`` that provides stable API-compatible access to
   type object members. On Python 3.9 and earlier, the official function did
   not work on non-heap types. The nanobind version consistently works on heap
   and non-heap types across Python versions.

Instances
^^^^^^^^^

The documentation below refers to two per-instance flags with the following meaning:

- *ready*: is the instance fully constructed? nanobind will not permit passing
  the instance to a bound C++ function when this flag is unset.

- *destruct*: should nanobind call the C++ destructor when the instance is
  garbage-collected?

.. cpp:function:: bool inst_check(handle h)

   Returns ``true`` if `h` represents an instance of a type that was
   previously bound via :cpp:class:`class_`.

.. cpp:function:: template <typename T> T * inst_ptr(handle h)

   Assuming that `h` represents an instance of a type that was previously bound
   via :cpp:class:`class_`, return a pointer to the underlying C++ instance.

   The function *does not check* that `h` actually contains an instance with
   C++ type `T`.

.. cpp:function:: object inst_alloc(handle h)

   Assuming that `h` represents a type object that was previously created via
   :cpp:class:`class_` (see :cpp:func:`type_check`), allocate an unitialized
   object of type `h` and return it. The *ready* and *destruct* flags of the
   returned instance are both set to ``false``.

.. cpp:function:: object inst_alloc_zero(handle h)

   Assuming that `h` represents a type object that was previously created via
   :cpp:class:`class_` (see :cpp:func:`type_check`), allocate a zero-initialized
   object of type `h` and return it. The *ready* and *destruct* flags of the
   returned instance are both set to ``true``.

   This operation is equivalent to calling :cpp:func:`inst_alloc` followed by
   :cpp:func:`inst_zero`.

.. cpp:function:: object inst_reference(handle h, void * p, handle parent = handle())

   Assuming that `h` represents a type object that was previously created via
   :cpp:class:`class_` (see :cpp:func:`type_check`) create an object of type
   `h` that wraps an existing C++ instance `p`.

   The *ready* and *destruct* flags of the returned instance are respectively
   set to ``true`` and ``false``.

   This is analogous to casting a C++ object with return value policy
   :cpp:enumerator:`rv_policy::reference`.

   If a `parent` object is specified, the instance keeps this parent alive
   while the newly created object exists. This is analogous to casting a C++
   object with return value policy
   :cpp:enumerator:`rv_policy::reference_internal`.

.. cpp:function:: object inst_take_ownership(handle h, void * p)

   Assuming that `h` represents a type object that was previously created via
   :cpp:class:`class_` (see :cpp:func:`type_check`) create an object of type
   `h` that wraps an existing C++ instance `p`.

   The *ready* and *destruct* flags of the returned instance are both set to
   ``true``.

   This is analogous to casting a C++ object with return value policy
   :cpp:enumerator:`rv_policy::take_ownership`.

.. cpp:function:: void inst_zero(handle h)

   Zero-initialize the contents of `h`. Sets the *ready* and *destruct* flags
   to ``true``.

.. cpp:function:: bool inst_ready(handle h)

   Query the *ready* flag of the instance `h`.

.. cpp:function:: std::pair<bool, bool> inst_state(handle h)

   Separately query the *ready* and *destruct* flags of the instance `h`.

.. cpp:function:: void inst_mark_ready(handle h)

   Simultaneously set the *ready* and *destruct* flags of the instance `h` to ``true``.

.. cpp:function:: void inst_set_state(handle h, bool ready, bool destruct)

   Separately set the *ready* and *destruct* flags of the instance `h`.

.. cpp:function:: void inst_destruct(handle h)

   Destruct the instance `h`. This entails calling the C++ destructor if the
   *destruct* flag is set and then setting the *ready* and *destruct* fields to
   ``false``.

.. cpp:function:: void inst_copy(handle dst, handle src)

   Copy-construct the contents of `src` into `dst` and set the *ready* and
   *destruct* flags of `dst` to ``true``.

   `dst` should be an uninitialized instance of the same type. Note that
   setting the *destruct* flag may be problematic if `dst` is an offset into an
   existing object created using :cpp:func:`inst_reference` (the destructor
   will be called multiple times in this case). If so, you must use
   :cpp:func:`inst_set_state` to disable the flag following the call to
   :cpp:func:`inst_copy`.

   *New in nanobind v2.0.0*: The function is a no-op when ``src`` and ``dst``
   refer to the same object.

.. cpp:function:: void inst_move(handle dst, handle src)

   Analogous to :cpp:func:`inst_copy`, except that the move constructor
   is used instead of the copy constructor.

.. cpp:function:: void inst_replace_copy(handle dst, handle src)

   Destruct the contents of `dst` (even if the *destruct* flag is ``false``).
   Next, copy-construct the contents of `src` into `dst` and set the *ready*
   flag of ``dst``. The value of the *destruct* flag is subsequently set to its
   value prior to the call.

   This operation is useful to replace the contents of one instance with that
   of another regardless of whether `dst` has been created using
   :cpp:func:`inst_alloc`, :cpp:func:`inst_reference`, or
   :cpp:func:`inst_take_ownership`.

   *New in nanobind v2.0.0*: The function is a no-op when ``src`` and ``dst``
   refer to the same object.

.. cpp:function:: void inst_replace_move(handle dst, handle src)

   Analogous to :cpp:func:`inst_replace_copy`, except that the move constructor
   is used instead of the copy constructor.

.. cpp:function:: str inst_name(handle h)

   Return the full (module-qualified) name of the instance's type object as a
   Python string.

Global flags
------------

.. cpp:function:: bool leak_warnings() noexcept

   Returns whether nanobind warns if any nanobind instances, types, or
   functions are still alive when the Python interpreter shuts down.

.. cpp:function:: bool implicit_cast_warnings() noexcept

   Returns whether nanobind warns if an implicit conversion was not successful.

.. cpp:function:: void set_leak_warnings(bool value) noexcept

   By default, nanobind loudly complains when any nanobind instances, types, or
   functions are still alive when the Python interpreter shuts down. Call this
   function to disable or re-enable leak warnings.

.. cpp:function:: void set_implicit_cast_warnings(bool value) noexcept

   By default, nanobind loudly complains when it attempts to perform an
   implicit conversion, and when that conversion is not successful. Call this
   function to disable or re-enable the warnings.

.. cpp:function:: inline bool is_alive() noexcept

   The function returns ``true`` when nanobind is initialized and ready for
   use. It returns ``false`` when the Python interpreter has shut down, causing
   the destruction various nanobind-internal data structures. Having access to
   this liveness status can be useful to avoid operations that are illegal in
   the latter context.

Miscellaneous
-------------

.. cpp:function:: str repr(handle h)

   Return a stringified version of the provided Python object.
   Equivalent to ``repr(h)`` in Python.

.. cpp:function:: void print(handle value, handle end = handle(), handle file = handle())

   Invoke the Python ``print()`` function to print the object `value`. If desired,
   a line ending `end` and file handle `file` can be specified.

.. cpp:function:: void print(const char * str, handle end = handle(), handle file = handle())

   Invoke the Python ``print()`` function to print the null-terminated C-style
   string `str` that is encoded using UTF-8 encoding.  If desired, a line
   ending `end` and file handle `file` can be specified.

.. cpp:function:: iterator iter(handle h)

   Equivalent to ``iter(h)`` in Python.

.. cpp:function:: object none()

   Return an object representing the value ``None``.

.. cpp:function:: dict builtins()

   Return the ``__builtins__`` dictionary.

.. cpp:function:: dict globals()

   Return the ``globals()`` dictionary.

.. cpp:function:: Py_hash_t hash(handle h)

   Hash the given argument like ``hash()`` in pure Python. The type of the
   return value (``Py_hash_t``) is an implementation-specific signed integer
   type.

.. cpp:function:: template <typename Source, typename Target> void implicitly_convertible()

   Indicate that the type `Source` is implicitly convertible into `Target`
   (which must refer to a type that was previously bound via
   :cpp:class:`class_`).

   *Note*: the :cpp:struct:`init_implicit` interface generates more compact
   code and should be preferred, i.e., use

   .. code-block:: cpp

      nb::class_<Target>(m, "Target")
          .def(nb::init_implicit<Source>());

   instead of

   .. code-block:: cpp

      nb::class_<Target>(m, "Target")
          .def(nb::init<Source>());

      nb::implicitly_convertible<Source, Target>();

   The function is provided for reasons of compatibility with pybind11, and as
   an escape hatch to enable use cases where :cpp:struct:`init_implicit`
   is not available (e.g., for custom binding-specific constructors that don't
   exist in `Target` type).

.. cpp:class:: template <typename T, typename... Ts> typed

    This helper class provides an interface to parameterize generic types to
    improve generated Python function signatures (e.g., to turn ``list`` into
    ``list[MyType]``).

    Consider the following binding that iterates over a Python list.

    .. code-block:: cpp

       m.def("f", [](nb::list l) {
           for (handle h : l) {
               // ...
           }
       });

    Suppose that ``f`` expects a list of ``MyType`` objects, which is not clear
    from the signature. To make this explicit, use the ``nb::typed<T, Ts...>``
    wrapper to pass additional type parameters. This has no effect besides
    clarifying the signature---in particular, nanobind does *not* insert
    additional runtime checks!

    .. code-block:: cpp

       m.def("f", [](nb::typed<nb::list, MyType> l) {
           for (nb::handle h : l) {
               // ...
           }
       });



C++ API Reference (Extras)
==========================

.. cpp:namespace:: nanobind

Operator overloading
--------------------

The following optional include directive imports the special value :cpp:var:`self`.

.. code-block:: cpp

   #include <nanobind/operators.h>

The underlying type exposes various C++ operators that enable a shorthand
notation to bind operators to python. See the :ref:`operator overloading
<operator_overloading>` example in the main documentation for details.


.. cpp:class:: detail::self_t

   This is an internal class that should be accessed through the singleton
   :cpp:var:`self` value.

   It supports the overloaded operators listed below. Depending on whether
   :cpp:var:`self` is the left or right argument of a binary operation,
   the binding will map to different Python methods as shown below.

   .. list-table::
      :header-rows: 1
      :widths: 50 50

      * - C++ operator
        - Python method (left or right)
      * - ``operator-``
        - ``__sub__``, ``__rsub__``
      * - ``operator+``
        - ``__add__``, ``__radd__``
      * - ``operator*``
        - ``__mul__``, ``__rmul__``
      * - ``operator/``
        - ``__truediv__``, ``__rtruediv__``
      * - ``operator%``
        - ``__mod__``, ``__rmod__``
      * - ``operator<<``
        - ``__lshift__``, ``__rlshift__``
      * - ``operator>>``
        - ``__rshift__``, ``__rrshift__``
      * - ``operator&``
        - ``__and__``, ``__rand__``
      * - ``operator^``
        - ``__xor__``, ``__rxor__``
      * - ``operator|``
        - ``__or__``, ``__ror__``
      * - ``operator>``
        - ``__gt__``, ``__lt__``
      * - ``operator>=``
        - ``__ge__``, ``__le__``
      * - ``operator<``
        - ``__lt__``, ``__gt__``
      * - ``operator<=``
        - ``__le__``, ``__ge__``
      * - ``operator==``
        - ``__eq__``
      * - ``operator!=``
        - ``__ne__``
      * - ``operator+=``
        - ``__iadd__``
      * - ``operator-=``
        - ``__isub__``
      * - ``operator*=``
        - ``__mul__``
      * - ``operator/=``
        - ``__itruediv__``
      * - ``operator%=``
        - ``__imod__``
      * - ``operator<<=``
        - ``__ilrshift__``
      * - ``operator>>=``
        - ``__ilrshift__``
      * - ``operator&=``
        - ``__iand__``
      * - ``operator^=``
        - ``__ixor__``
      * - ``operator|=``
        - ``__ior__``
      * - ``operator-`` (unary)
        - ``__neg__``
      * - ``operator+`` (unary)
        - ``__pos__``
      * - ``operator~``  (unary)
        - ``__invert__``
      * - ``operator!``  (unary)
        - ``__bool__`` (with extra negation)
      * - ``nb::abs(..)``
        - ``__abs__``
      * - ``nb::hash(..)``
        - ``__hash__``

.. cpp:var:: detail::self_t self

Trampolines
-----------

The following macros to implement trampolines that forward virtual function
calls to Python require an additional include directive:

.. code-block:: cpp

   #include <nanobind/trampoline.h>

See the section on :ref:`trampolines <trampolines>` for further detail.

.. c:macro:: NB_TRAMPOLINE(base, size)

   Install a trampoline in an alias class to enable dispatching C++ virtual
   function calls to a Python implementation. Refer to the documentation on
   :ref:`trampolines <trampolines>` to see how this macro can be used.

.. c:macro:: NB_OVERRIDE(func, ...)

   Dispatch the call to a Python method named ``"func"`` if it is overloaded on
   the Python side, and forward the function arguments specified in the
   variable length argument ``...``. Otherwise, call the C++ implementation
   `func` in the base class.

   Refer to the documentation on :ref:`trampolines <trampolines>` to see how
   this macro can be used.

.. c:macro:: NB_OVERRIDE_PURE(func, ...)

   Dispatch the call to a Python method named ``"func"`` if it is overloaded on
   the Python side, and forward the function arguments specified in the
   variable length argument ``...``. Otherwise, raise an exception. This macro
   should be used when the C++ function is pure virtual.

   Refer to the documentation on :ref:`trampolines <trampolines>` to see how
   this macro can be used.

.. c:macro:: NB_OVERRIDE_NAME(name, func, ...)

   Dispatch the call to a Python method named ``name`` if it is overloaded on
   the Python side, and forward the function arguments specified in the
   variable length argument ``...``. Otherwise, call the C++ function `func` in
   the base class.

   This function differs from :c:macro:`NB_OVERRIDE() <NB_OVERRIDE>` in that
   C++ and Python functions can be named differently (e.g., ``operator+`` and
   ``__add__``). Refer to the documentation on :ref:`trampolines <trampolines>`
   to see how this macro can be used.

.. c:macro:: NB_OVERRIDE_PURE_NAME(name, func, ...)

   Dispatch the call to a Python method named ``name`` if it is overloaded on
   the Python side, and forward the function arguments specified in the
   variable length argument ``...``. Otherwise, raise an exception. This macro
   should be used when the C++ function is pure virtual.

   This function differs from :c:macro:`NB_OVERRIDE_PURE() <NB_OVERRIDE_PURE>`
   in that C++ and Python functions can be named differently (e.g.,
   ``operator+`` and ``__add__``). Although the C++ base implementation cannot
   be called, its name is still important since nanobind uses it to infer the
   return value type. Refer to the documentation on :ref:`trampolines
   <trampolines>` to see how this macro can be used.

.. _vector_bindings:

STL vector bindings
-------------------

The following function can be used to expose ``std::vector<...>`` variants
in Python. It is not part of the core nanobind API and requires an additional
include directive:

.. code-block:: cpp

   #include <nanobind/stl/bind_vector.h>

.. cpp:function:: template <typename Vector, rv_policy Policy = rv_policy::automatic_reference, typename... Args> class_<Vector> bind_vector(handle scope, const char * name, Args &&...args)

   Bind the STL vector-derived type `Vector` to the identifier `name` and
   place it in `scope` (e.g., a :cpp:class:`module_`). The variable argument
   list can be used to pass a docstring and other :ref:`class binding
   annotations <class_binding_annotations>`.

   The type includes the following methods resembling ``list``:

   .. list-table::
      :header-rows: 1
      :widths: 50 50

      * - Signature
        - Documentation
      * - ``__init__(self)``
        - Default constructor
      * - ``__init__(self, arg: Vector)``
        - Copy constructor
      * - ``__init__(self, arg: typing.Sequence)``
        - Construct from another sequence type
      * - ``__len__(self) -> int``
        - Return the number of elements
      * - ``__repr__(self) -> str``
        - Generate a string representation
      * - ``__contains__(self, arg: Value)``
        - Check if the vector contains ``arg``
      * - ``__eq__(self, arg: Vector)``
        - Check if the vector is equal to ``arg``
      * - ``__ne__(self, arg: Vector)``
        - Check if the vector is not equal to ``arg``
      * - ``__bool__(self) -> bool``
        - Check whether the vector is empty
      * - ``__iter__(self) -> iterator``
        - Instantiate an iterator to traverse the elements
      * - ``__getitem__(self, arg: int) -> Value``
        - Return an element from the list (supports negative indexing)
      * - ``__setitem__(self, arg0: int, arg1: Value)``
        - Assign an element in the list (supports negative indexing)
      * - ``__delitem__(self, arg: int)``
        - Delete an item from the list (supports negative indexing)
      * - ``__getitem__(self, arg: slice) -> Vector``
        - Slice-based getter
      * - ``__setitem__(self, arg0: slice, arg1: Value)``
        - Slice-based assignment
      * - ``__delitem__(self, arg: slice)``
        - Slice-based deletion
      * - ``clear(self)``
        - Remove all items from the list
      * - ``append(self, arg: Value)``
        - Append a list item
      * - ``insert(self, arg0: int, arg1: Value)``
        - Insert a list item (supports negative indexing)
      * - ``pop(self, index: int = -1)``
        - Pop an element at position ``index`` (the end by default)
      * - ``extend(self, arg: Vector)``
        - Extend ``self`` by appending elements from ``arg``.
      * - ``count(self, arg: Value)``
        - Count the number of times that ``arg`` is contained in the vector
      * - ``remove(self, arg: Value)``
        - Remove all occurrences of ``arg``.

   In contrast to ``std::vector<...>``, all bound functions perform range
   checks to avoid undefined behavior. When the type underlying the vector is
   not comparable or copy-assignable, some of these functions will not be
   generated.

   The binding operation is a no-op if the vector type has already been
   registered with nanobind.

   .. warning::

      While this function creates a type resembling a Python ``list``, it has a
      major caveat: the item accessor ``__getitem__`` copies the accessed
      element by default (the bottom of this paragraph explains how this copy
      can be avoided).

      Consequently, writes to elements may not propagate in the expected way.
      Consider the following C++ bindings:

      .. code-block:: cpp

         struct A {
             int value;
         };

         nb::class_<A>(m, "A")
             .def(nb::init<int>())
             .def_rw("value", &A::value);

         nb::bind_vector<std::vector<A>>(m, "VecA");

      On the Python end, they yield the following surprising behavior:

      .. code-block:: python

         from my_ext import A, VecA

         va = VecA()
         va.append(A(123))
         va[0].value = 456
         assert va[0].value == 456 # <-- assertion fails!

      To actually modify ``va``, another write is needed.

      .. code-block:: python

         v = va[0]
         v.value = 456
         va[0] = v

      This may seem like a strange design, so it is worth explaining why the
      implementation works in this way.

      The key issue is that any particular value (e.g., ``va[0]``) lies within
      a memory buffer managed by the ``std::vector``. It is not safe for
      nanobind to refer to objects within this buffer using their absolute or
      relative memory address. For example, inserting an element at position 0
      will rearrange the buffer's contents and shift all subsequent ``A``
      instances. If nanobind ``A`` objects could be "views" into the
      ``std::vector``, then an insertion would cause the contents of unrelated
      ``A`` Python objects to change unexpectedly. Insertion may also require
      reallocation of the buffer, invalidating all current addresses, and this
      could lead to undefined behavior (use-after-free) if nanobind did not
      make a copy.

      There are three situations in which the surprising behavior is avoided:

      1. If the modification of the array is performed using in-place
         operations like

         .. code-block:: python

            v[i] += 5

         In-place operators automatically perform an array assignment, causing
         the issue to disappear. This means that if you work with a vector type
         like ``std::vector<int>`` or ``std::vector<std::string>`` with an
         immutable element type like ``int`` or ``str`` on the Python end, it
         will behave completely naturally in Python.

      2. If the array contains STL shared pointers (e.g.,
         ``std::vector<std::shared_ptr<T>>``), the added
         indirection and ownership tracking removes the need for extra copies.

      3. If the array contains pointers to reference-counted objects (e.g.,
         ``std::vector<ref<T>>`` via the :cpp:class:`ref` wrapper) and ``T``
         uses the intrusive reference counting approach explained :ref:`here
         <intrusive>`, the added indirection and ownership tracking removes the
         need for extra copies.

         (It is usually unsafe to use this class to bind pointer-valued
         vectors ``std::vector<T*>`` when ``T`` does not use intrusive
         reference counting, because then there is nothing to prevent the Python
         objects returned by ``__getitem__`` from outliving the C++ ``T``
         objects that they point to. But if you are able to guarantee through
         other means that the ``T`` objects will live long enough, the intrusive
         reference counting is not strictly required.)

   .. note::

      Previous versions of nanobind (before 2.0) and pybind11 return Python
      objects from ``__getitem__`` that wrap *references* (i.e., views),
      meaning that they are only safe to use until the next insertion or
      deletion in the vector they were drawn from. As discussed above, any use
      after that point could **corrupt memory or crash your program**, which is
      why reference semantics are no longer the default.

      If you truly need the unsafe reference semantics, and if you
      can guarantee that all use of your bindings will respect
      the memory layout and reference-invalidation rules of the
      underlying C++ container type, you can request the old behavior
      by passing a second template argument of
      :cpp:enumerator:`rv_policy::reference_internal` to
      :cpp:func:`bind_vector`. This will override nanobind's usual
      choice of :cpp:enumerator:`rv_policy::copy` for ``__getitem__``.

      .. code-block:: cpp

         nb::bind_vector<std::vector<MyType>,
                         nb::rv_policy::reference_internal>(m, "ExampleVec");

      Again, please avoid this if at all possible.
      It is *very* easy to cause problems if you're not careful, as the
      following example demonstrates.

      .. code-block:: python

         def looks_fine_but_crashes(vec: ext.ExampleVec) -> None:
             # Trying to remove all the elements too much older than the last:
             last = vec[-1]
             # Even being careful to iterate backwards so we visit each
             # index only once...
             for idx in range(len(vec) - 2, -1, -1):
                 if last.timestamp - vec[idx].timestamp > 5:
                     del vec[idx]
                     # Oops! After the first deletion, 'last' now refers to
                     # uninitialized memory.


.. _map_bindings:

STL map bindings
----------------

The following function can be used to expose ``std::map<...>`` or
``std::unordered_map<...>`` variants in Python. It is not part of the core
nanobind API and requires an additional include directive:

.. code-block:: cpp

   #include <nanobind/stl/bind_map.h>

.. cpp:function:: template <typename Map, rv_policy Policy = rv_policy::automatic_reference, typename... Args> class_<Map> bind_map(handle scope, const char * name, Args &&...args)

   Bind the STL map-derived type `Map` (ordered or unordered) to the identifier
   `name` and place it in `scope` (e.g., a :cpp:class:`module_`). The variable
   argument list can be used to pass a docstring and other :ref:`class binding
   annotations <class_binding_annotations>`.

   The type includes the following methods resembling ``dict``:

   .. list-table::
      :header-rows: 1
      :widths: 50 50

      * - Signature
        - Documentation
      * - ``__init__(self)``
        - Default constructor
      * - ``__init__(self, arg: Map)``
        - Copy constructor
      * - ``__init__(self, arg: dict)``
        - Construct from a Python dictionary
      * - ``__len__(self) -> int``
        - Return the number of elements
      * - ``__repr__(self) -> str``
        - Generate a string representation
      * - ``__contains__(self, arg: Key)``
        - Check if the map contains ``arg``
      * - ``__eq__(self, arg: Map)``
        - Check if the map is equal to ``arg``
      * - ``__ne__(self, arg: Map)``
        - Check if the map is not equal to ``arg``
      * - ``__bool__(self) -> bool``
        - Check whether the map is empty
      * - ``__iter__(self) -> iterator``
        - Instantiate an iterator to traverse the set of map keys
      * - ``__getitem__(self, arg: Key) -> Value``
        - Return an element from the map
      * - ``__setitem__(self, arg0: Key, arg1: Value)``
        - Assign an element in the map
      * - ``__delitem__(self, arg: Key)``
        - Delete an item from the map
      * - ``clear(self)``
        - Remove all items from the list
      * - ``update(self, arg: Map)``
        - Update the map with elements from ``arg``.
      * - ``keys(self, arg: Map) -> Map.KeyView``
        - Returns an iterable view of the map's keys
      * - ``values(self, arg: Map) -> Map.ValueView``
        - Returns an iterable view of the map's values
      * - ``items(self, arg: Map) -> Map.ItemView``
        - Returns an iterable view of the map's items

   The binding operation is a no-op if the map type has already been
   registered with nanobind.

   The binding routine ideally expects the involved types to be:

   - copy-constructible
   - copy-assignable
   - equality-comparable

   If not all of these properties are available, then a subset of the above
   methods will be omitted. Please refer to ``bind_map.h`` for details on the
   logic.

   .. warning::

      While this function creates a type resembling a Python ``dict``, it has a
      major caveat: the item accessor ``__getitem__`` copies the accessed
      element by default.

      Please refer to the :ref:`STL vector bindings <vector_bindings>` for a
      discussion of the problem and possible solutions. Everything applies
      equally to the map case.

   .. note::

      Unlike ``std::vector``, the ``std::map`` and ``std::unordered_map``
      containers are *node-based*, meaning their elements do have a
      consistent address for as long as they're stored in the map.
      (Note that this is generally *not* true of third-party containers
      with similar interfaces, such as ``absl::flat_hash_map``.)

      If you are binding a node-based container type, and you want
      ``__getitem__`` to return a reference to the accessed element
      rather than copying it, it is *somewhat* safer than it would
      be with :cpp:func:`bind_vector` to use the unsafe workaround
      discussed there:

      .. code-block:: cpp

         nb::bind_map<std::map<std::string, SomeValue>,
                      nb::rv_policy::reference_internal>(m, "ExampleMap");

      With a node-based container, the only situation where a reference
      returned from ``__getitem__`` would be invalidated is if the individual
      element that it refers to were removed from the map. Unlike with
      ``std::vector``, additions and removals of *other* elements would
      not present a danger.

      It is still easy to cause problems if you're not careful, though:

      .. code-block:: python

         def unsafe_pop(map: ext.ExampleMap, key: str) -> ext.SomeValue:
             value = map[key]
             del map[key]
             # Oops! `value` now points to a dangling element. Anything you
             # do with it now is liable to crash the interpreter.
             return value  # uh-oh...


Unique pointer deleter
----------------------

The following *deleter* should be used to gain maximal flexibility in combination with
``std::unique_ptr<..>``. It requires the following additional include directive:

.. code-block:: cpp

   #include <nanobind/stl/unique_ptr.h>

See the two documentation sections on unique pointers for further detail
(:ref:`#1 <unique_ptr>`, :ref:`#2 <unique_ptr_adv>`).

.. cpp:struct:: template <typename T> deleter

   .. cpp:function:: deleter() = default

      Create a deleter that destroys the object using a ``delete`` expression.

   .. cpp:function:: deleter(handle h)

      Create a deleter that destroys the object by reducing the Python reference count.

   .. cpp:function:: bool owned_by_python() const

      Check if the object is owned by Python.

   .. cpp:function:: bool owned_by_cpp() const

      Check if the object is owned by C++.

   .. cpp:function:: void operator()(void * p) noexcept

      Destroy the object at address `p`.

.. _iterator_bindings:

Iterator bindings
-----------------

The following functions can be used to expose existing C++ iterators in
Python. They are not part of the core nanobind API and require an additional
include directive:

.. code-block:: cpp

   #include <nanobind/make_iterator.h>

.. cpp:function:: template <rv_policy Policy = rv_policy::automatic_reference, typename Iterator, typename Sentinel, typename... Extra> auto make_iterator(handle scope, const char * name, Iterator first, Sentinel last, Extra &&...extra)

   Create a Python iterator wrapping the C++ iterator represented by the range
   ``[first, last)``. The `Extra` parameter can be used to pass additional
   function binding annotations.

   This function lazily creates a new Python iterator type identified by
   `name`, which is stored in the given `scope`. Usually, some kind of
   :cpp:class:`keep_alive` annotation is needed to tie the lifetime of the
   parent container to that of the iterator.

   The return value is a typed iterator (:cpp:class:`iterator` wrapped using
   :cpp:class:`typed`), whose template parameter is given by the type of
   ``*first``.

   Here is an example of what this might look like for a STL vector:

   .. code-block:: cpp

      using IntVec = std::vector<int>;

      nb::class_<IntVec>(m, "IntVec")
         .def("__iter__",
              [](const IntVec &v) {
                  return nb::make_iterator(nb::type<IntVec>(), "iterator",
                                           v.begin(), v.end());
              }, nb::keep_alive<0, 1>());

   .. note::

      Pre-2.0 versions of nanobind and pybind11 return *references* (views)
      into the underlying sequence.

      This is convenient when

      1. Iterated elements are used to modify the underlying container.

      2. Iterated elements should reflect separately made changes to
         the underlying container.

      But this strategy is *unsafe* if the allocated memory region or layout
      of the container could change (e.g., through insertion of removal of
      elements).

      Because of this, iterators now copy by default. There are two
      ways to still obtain references to the target elements:

      1. If the iterator is over STL shared pointers, the added indirection and
         ownership tracking removes the need for extra copies.

      2. If the iterator is over reference-counted objects (e.g., ``ref<T>``
         via the :cpp:class:`ref` wrapper) and ``T`` uses the intrusive
         reference counting approach explained :ref:`here <intrusive>`,
         the added indirection and ownership tracking removes the need
         for extra copies.

      If you truly need the unsafe reference semantics, and if you can
      guarantee that all use of your bindings will respect the memory layout
      and reference-invalidation rules of the underlying C++ container type,
      you can request the old behavior by passing
      :cpp:enumerator:`rv_policy::reference_internal` to the ``Policy``
      template argument of this function.


.. cpp:function:: template <rv_policy Policy = rv_policy::automatic_reference, typename Type, typename... Extra> auto make_iterator(handle scope, const char * name, Type &value, Extra &&...extra)

   This convenience wrapper calls the above :cpp:func:`make_iterator` variant with
   ``first`` and ``last`` set to ``std::begin(value)`` and ``std::end(value)``,
   respectively.

.. cpp:function:: template <rv_policy Policy = rv_policy::automatic_reference, typename Iterator, typename Sentinel, typename... Extra> iterator make_key_iterator(handle scope, const char * name, Iterator first, Sentinel last, Extra &&...extra)

   :cpp:func:`make_iterator` specialization for C++ iterators that return
   key-value pairs. `make_key_iterator` returns the first pair element to
   iterate over keys.

   The return value is a typed iterator (:cpp:class:`iterator` wrapped using
   :cpp:class:`typed`), whose template parameter is given by the type of
   ``(*first).first``.


.. cpp:function:: template <rv_policy Policy = rv_policy::automatic_reference, typename Iterator, typename Sentinel, typename... Extra> iterator make_value_iterator(handle scope, const char * name, Iterator first, Sentinel last, Extra &&...extra)

   :cpp:func:`make_iterator` specialization for C++ iterators that return
   key-value pairs. `make_value_iterator` returns the second pair element to
   iterate over values.

   The return value is a typed iterator (:cpp:class:`iterator` wrapped using
   :cpp:class:`typed`), whose template parameter is given by the type of
   ``(*first).second``.

N-dimensional array type
------------------------

The following type can be used to exchange n-dimension arrays with frameworks
like NumPy, PyTorch, Tensorflow, JAX, CuPy, and others. It requires an
additional include directive:

.. code-block:: cpp

   #include <nanobind/ndarray.h>

Detailed documentation including example code is provided in a :ref:`separate
section <ndarrays>`.

.. cpp:function:: bool ndarray_check(handle h) noexcept

   Test whether the Python object represents an ndarray.

   Objects with a ``__dlpack__`` attribute or objects that implement the buffer
   protocol are considered as ndarray objects. In addition, arrays from NumPy,
   PyTorch, TensorFlow and XLA are also regarded as ndarrays.

.. cpp:class:: template <typename... Args> ndarray

   .. cpp:type:: Scalar

      The scalar type underlying the array (or ``void`` if not specified)

   .. cpp:var:: static constexpr bool ReadOnly

      A ``constexpr`` Boolean value that is ``true`` if the ndarray template
      arguments (`Args... <Args>`) include the ``nb::ro`` annotation or a
      ``const``-qualified scalar type.

   .. cpp:var:: static constexpr char Order

      A ``constexpr`` character value set based on the ndarray template
      arguments (`Args... <Args>`). It equals

      - ``'C'`` if :cpp:class:`c_contig` is specified,
      - ``'F'`` if :cpp:class:`f_contig` is specified,
      - ``'A'`` if :cpp:class:`any_contig` is specified,
      - ``'\0'`` otherwise.

   .. cpp:var:: static constexpr int DeviceType

      A ``constexpr`` integer value set to the device type ID extracted from
      the ndarray template arguments (`Args... <Args>`), or
      :cpp:struct:`device::none::value <device::none>` when none was specified.

   .. cpp:type:: VoidPtr = std::conditional_t<ReadOnly, const void *, void *>

      A potentially ``const``-qualified ``void*`` pointer type used by some
      of the ``ndarray`` constructors.

   .. cpp:function:: ndarray() = default

      Create an invalid array.

   .. cpp:function:: template <typename... Args2> explicit ndarray(const ndarray<Args2...> &other)

      Reinterpreting constructor that wraps an existing nd-array (parameterized
      by `Args... <Args>`) into a new ndarray (parameterized by `Args2...
      <Args2>`). No copy or conversion is made.

      Dropping parameters is always safe. For example, a function that
      returns different array types could call it to convert ``ndarray<T>`` to
      ``ndarray<>``.  When adding constraints, the constructor is only safe to
      use following a runtime check to ensure that newly created array actually
      possesses the advertised properties.

   .. cpp:function:: ndarray(const ndarray &)

      Copy constructor. Increases the reference count of the referenced array.

   .. cpp:function:: ndarray(ndarray &&)

      Move constructor. Steals the referenced array without changing reference counts.

   .. cpp:function:: ~ndarray()

      Decreases the reference count of the referenced array and potentially destroy it.

   .. cpp:function:: ndarray& operator=(const ndarray &)

      Copy assignment operator. Increases the reference count of the referenced array.
      Decreases the reference count of the previously referenced array and potentially destroy it.

   .. cpp:function:: ndarray& operator=(ndarray &&)

      Move assignment operator. Steals the referenced array without changing reference counts.
      Decreases the reference count of the previously referenced array and potentially destroy it.

   .. _ndarray_dynamic_constructor:

   .. cpp:function:: ndarray(VoidPtr data, const std::initializer_list<size_t> shape = { }, handle owner = { }, std::initializer_list<int64_t> strides = { }, dlpack::dtype dtype = nanobind::dtype<Scalar>(), int32_t device_type = DeviceType, int32_t device_id = 0, char order = Order)

      Create an array wrapping an existing memory allocation.

      Only the `data` parameter is strictly required, while some other
      parameters can be be inferred from static :cpp:class:`nb::ndarray\<...\>
      <ndarray>` template parameters.

      The parameters have the following meaning:

      - `data`: a CPU/GPU/.. pointer to the memory region storing the array
        data.

        When the array is parameterized by a ``const`` scalar type, or when it
        has a :cpp:class:`nb::ro <ro>` read-only annotation, a ``const``
        pointer can be passed here.

      - `shape`: an initializer list that simultaneously specifies the number
        of dimensions and the size along each axis. If left at its default
        ``{}``, the :cpp:class:`nb::shape <nanobind::shape>` template parameter
        will take precedence (if present).

      - `owner`: if provided, the array will hold a reference to this object
        until its destruction. This makes it possible to create zero-copy views
        into other data structures, while guaranteeing the memory safety of
        array accesses.

      - `strides`: an initializer list explaining the layout of the data in
        memory. Each entry denotes the number of elements to jump over to
        advance to the next item along the associated axis.

        `strides` must either have the same size as `shape` or be empty. In the
        latter case, strides are automatically computed according to the
        `order` parameter.

        Note that strides in nanobind express *element counts* rather than
        *byte counts*. This convention differs from other frameworks (e.g.,
        NumPy) and is a consequence of the underlying `DLPack
        <https://github.com/dmlc/dlpack>`_ protocol.

      - `dtype` describes the numeric data type of array elements (e.g.,
        floating point, signed/unsigned integer) and their bit depth.

        You can use the :cpp:func:`nb::dtype\<T\>() <nanobind::dtype>` function to obtain the right
        value for a given type.

      - `device_type` and `device_id` specify where the array data is stored.
        The `device_type` must be an enumerant like
        :cpp:class:`nb::device::cuda::value <device::cuda>`, while the meaning
        of the device ID is unspecified and platform-dependent.

        Note that the `device_id` is set to ``0`` by default and cannot be
        inferred by nanobind. If your extension creates arrays on multiple
        different compute accelerators, you *must* provide this parameter.

      - The `order` parameter denotes the coefficient order in memory and is only
        relevant when `strides` is empty. Specify ``'C'`` for C-style or ``'F'``
        for Fortran-style. When this parameter is not explicitly specified, the
        implementation uses the order specified as an ndarray template
        argument, or C-style order as a fallback.

      Both ``strides`` and ``shape`` will be copied by the constructor, hence
      the targets of these initializer lists do not need to remain valid
      following the constructor call.

      .. warning::

         The Python *global interpreter lock* (GIL) must be held when calling
         this function.

   .. cpp:function:: ndarray(VoidPtr data, size_t ndim, const size_t * shape, handle owner, const int64_t * strides = nullptr, dlpack::dtype dtype = nanobind::dtype<Scalar>(), int device_type = DeviceType, int device_id = 0, char order = Order)

      Alternative form of the above constructor, which accepts the `shape`
      and `strides` arguments using pointers instead of initializer lists.
      The number of dimensions must be specified via the `ndim` parameter
      in this case.

      See the previous constructor for details, the remaining behavior is
      identical.

   .. cpp:function:: dlpack::dtype dtype() const

      Return the data type underlying the array

   .. cpp:function:: size_t ndim() const

      Return the number of dimensions.

   .. cpp:function:: size_t size() const

      Return the size of the array (i.e. the product of all dimensions).

   .. cpp:function:: size_t itemsize() const

      Return the size of a single array element in bytes. The returned value
      is rounded up to the next full byte in case of bit-level representations
      (query :cpp:member:`dtype::bits` for bit-level granularity).

   .. cpp:function:: size_t nbytes() const

      Return the size of the entire array bytes. The returned value is rounded
      up to the next full byte in case of bit-level representations.

   .. cpp:function:: size_t shape(size_t i) const

      Return the size of dimension `i`.

   .. cpp:function:: int64_t stride(size_t i) const

      Return the stride (in number of elements) of dimension `i`.

   .. cpp:function:: const int64_t* shape_ptr() const

      Return a pointer to the shape array. Note that the return type is
      ``const int64_t*``, which may be unexpected as the scalar version
      :cpp:func:`shape()` casts its result to a ``size_t``.

      This is a consequence of the DLPack tensor representation that uses
      signed 64-bit integers for all of these fields.

   .. cpp:function:: const int64_t* stride_ptr() const

      Return pointer to the stride array.

   .. cpp:function:: bool is_valid() const

      Check whether the array is in a valid state.

   .. cpp:function:: int device_type() const

      ID denoting the type of device hosting the array. This will match the
      ``value`` field of a device class, such as :cpp:class:`device::cpu::value
      <device::cpu>` or :cpp:class:`device::cuda::value <device::cuda>`.

   .. cpp:function:: int device_id() const

      In a multi-device/GPU setup, this function returns the ID of the device
      storing the array.

   .. cpp:function:: Scalar * data() const

      Return a pointer to the array data.
      If :cpp:var:`ReadOnly` is true, a pointer-to-const is returned.

   .. cpp:function:: template <typename... Args2> auto& operator()(Args2... indices)

      Return a reference to the element stored at the provided index/indices.
      If :cpp:var:`ReadOnly` is true, a reference-to-const is returned.
      Note that ``sizeof...(Args2)`` must match :cpp:func:`ndim()`.

      This accessor is only available when the scalar type and array dimension
      were specified as template parameters.

      This function should only be used when the array storage is accessible
      through the CPU's virtual memory address space.

   .. cpp:function:: template <typename... Extra> auto view()

      Returns an nd-array view that is optimized for fast array access on the
      CPU. You may optionally specify additional ndarray constraints via the
      `Extra` parameter (though a runtime check should first be performed to
      ensure that the array possesses these properties).

      The returned view provides the operations ``data()``, ``ndim()``,
      ``shape()``, ``stride()``, and ``operator()`` following the conventions
      of the `ndarray` type.

   .. cpp:function:: auto cast(rv_policy policy = rv_policy::automatic_reference, handle parent = {})

      The expression ``array.cast(policy, parent)`` is almost equivalent to
      :cpp:func:`nb::cast(array, policy, parent) <cast>`.

      The main difference is that the return type of :cpp:func:`nb::cast
      <cast>` is :cpp:class:`nb::object <object>`, which renders as a rather
      non-descriptive ``object`` in Python bindings. The ``.cast()`` method
      returns a custom wrapper type that still derives from
      :cpp:class:`nb::object <object>`, but whose type signature in bindings
      reproduces that of the original nd-array.

Data types
^^^^^^^^^^

Nanobind uses the `DLPack <https://github.com/dmlc/dlpack>`_ ABI to represent
metadata describing n-dimensional arrays (even when they are exchanged using
the buffer protocol). Consequently, the set of possible dtypes is :ref:`more
restricted <dtype_restrictions>` than that of other nd-array libraries (e.g.,
NumPy). Relevant data structures are located in the ``nanobind::dlpack``
sub-namespace.


.. cpp:enum-class:: dlpack::dtype_code : uint8_t

   This enumeration characterizes the elementary array data type regardless of
   bit depth.

   .. cpp:enumerator:: Int = 0

      Signed integer format

   .. cpp:enumerator:: UInt = 1

      Unsigned integer format

   .. cpp:enumerator:: Float = 2

      IEEE-754 floating point format

   .. cpp:enumerator:: Bfloat = 4

      "Brain" floating point format

   .. cpp:enumerator:: Complex = 5

      Complex numbers parameterized by real and imaginary component

.. cpp:struct:: dlpack::dtype

   Represents the data type underlying an n-dimensional array. Use the
   :cpp:func:`dtype\<T\>() <::nanobind::dtype>` function to return a populated
   instance of this data structure given a scalar C++ arithmetic type.

   .. cpp:member:: uint8_t code = 0;

      This field must contain the value of one of the
      :cpp:enum:`dlpack::dtype_code` enumerants.

   .. cpp:member:: uint8_t bits = 0;

      Number of bits per entry (e.g., 32 for a C++ single precision ``float``)

   .. cpp:member:: uint16_t lanes = 0;

      Number of SIMD lanes (typically ``1``)

.. cpp:function:: template <typename T> dlpack::dtype dtype()

   Returns a populated instance of the :cpp:class:`dlpack::dtype` structure
   given a scalar C++ arithmetic type.

Array annotations
^^^^^^^^^^^^^^^^^

The :cpp:class:`ndarray\<..\> <ndarray>` class admits optional template
parameters. They constrain the type of array arguments that may be passed to a
function.

The following are supported:

Data type
+++++++++

The data type of the underlying scalar element. The following are supported.

- ``[u]int8_t`` up to ``[u]int64_t`` and other variations (``unsigned long long``, etc.)
- ``float``, ``double``
- ``bool``

Annotate the data type with ``const`` to indicate a read-only array. Note that
only the buffer protocol/NumPy interface considers ``const``-ness at the
moment; data exchange with other array libraries will ignore this annotation.

When the is unspecified (e.g., to accept arbitrary input arrays), the
:cpp:class:`ro` annotation can instead be used to denote read-only access:

.. cpp:class:: ro

   Indicate read-only access (use only when no data type is specified.)


nanobind does not support non-standard types as documented in the section on
:ref:`dtype limitations <dtype_restrictions>`.

Shape
+++++

.. cpp:class:: template <ssize_t... Is> shape

   Require the array to have ``sizeof...(Is)`` dimensions. Each entry of `Is`
   specifies a fixed size constraint for that specific dimension. An entry
   equal to ``-1`` indicates that *any* size should be accepted for this
   dimension.

   (An alias named ``nb::any`` representing ``-1`` was removed in nanobind 2).

.. cpp:class:: template <size_t N> ndim

   Alternative to the above that only constrains the array dimension.
   ``nb::ndim<2>`` is equivalent to ``nb::shape<-1, -1>``.

Contiguity
++++++++++

.. cpp:class:: c_contig

   Request that the array storage uses a C-contiguous representation.

.. cpp:class:: f_contig

   Request that the array storage uses a F (Fortran)-contiguous representation.

.. cpp:class:: any_contig

   Accept both C- and F-contiguous arrays.

If you prefer not to require contiguity, simply do not provide any of the
``*_contig`` template parameters listed above.

Device type
+++++++++++

.. cpp:class:: device

   The following helper classes can be used to constrain the device and
   address space of an array. Each class has a ``static constexpr int32_t
   value`` field that will then match up with
   :cpp:func:`ndarray::device_id()`.

   .. cpp:class:: cpu

      CPU heap memory

   .. cpp:class:: cuda

      NVIDIA CUDA device memory

   .. cpp:class:: cuda_host

      NVIDIA CUDA host-pinned memory

   .. cpp:class:: cuda_managed

      NVIDIA CUDA managed memory

   .. cpp:class:: vulkan

      Vulkan device memory

   .. cpp:class:: metal

      Apple Metal device memory

   .. cpp:class:: rocm

      AMD ROCm device memory

   .. cpp:class:: rocm_host

      AMD ROCm host memory

   .. cpp:class:: oneapi

      Intel OneAPI device memory

Framework
+++++++++

Framework annotations cause :cpp:class:`nb::ndarray <ndarray>` objects to
convert into an equivalent representation in one of the following frameworks:

.. cpp:class:: numpy

.. cpp:class:: tensorflow

.. cpp:class:: pytorch

.. cpp:class:: jax

.. cpp:class:: cupy

Eigen convenience type aliases
------------------------------

The following helper type aliases require an additional include directive:

.. code-block:: cpp

   #include <nanobind/eigen/dense.h>

.. cpp:type:: DStride = Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic>

   This type alias refers to an Eigen stride object that is sufficiently flexible
   so that can be easily called with NumPy arrays and array slices.

.. cpp:type:: template <typename T> DRef = Eigen::Ref<T, 0, DStride>

   This templated type alias creates an ``Eigen::Ref<..>`` with flexible strides for
   zero-copy data exchange between Eigen and NumPy.

.. cpp:type:: template <typename T> DMap = Eigen::Map<T, 0, DStride>

   This templated type alias creates an ``Eigen::Map<..>`` with flexible strides for
   zero-copy data exchange between Eigen and NumPy.

.. _chrono_conversions:

Timestamp and duration conversions
----------------------------------

nanobind supports bidirectional conversions of timestamps and
durations between their standard representations in Python
(:py:class:`datetime.datetime`, :py:class:`datetime.timedelta`) and in C++
(``std::chrono::time_point``, ``std::chrono::duration``).
A few unidirectional conversions from other Python types to these
C++ types are also provided and explained below.

These type casters require an additional include directive:

.. code-block:: cpp

   #include <nanobind/stl/chrono.h>

.. The rest of this section is adapted from pybind11/docs/advanced/cast/chrono.rst

An overview of clocks in C++11
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The C++11 standard defines three different clocks, and users can
define their own. Each ``std::chrono::time_point`` is defined relative
to a particular clock. When using the ``chrono`` type caster, you must be
aware that only ``std::chrono::system_clock`` is guaranteed to convert
to a Python :py:class:`~datetime.datetime` object; other clocks may convert to
:py:class:`~datetime.timedelta` if they don't represent calendar time.

The first clock defined by the standard is ``std::chrono::system_clock``.
This clock measures the current date and time, much like the Python
:py:func:`time.time` function. It can change abruptly due to
administrative actions, daylight savings time transitions, or
synchronization with an external time server. That makes this clock a
poor choice for timing purposes, but a good choice for wall-clock time.

The second clock defined by the standard is ``std::chrono::steady_clock``.
This clock ticks at a steady rate and is never adjusted, like
:py:func:`time.monotonic` in Python. That makes it excellent for timing
purposes, but the value in this clock does not correspond to the
current date and time. Often this clock will measure the amount of
time your system has been powered on. This clock will never be
the same clock as the system clock, because the system clock can
change but steady clocks cannot.

The third clock defined in the standard is ``std::chrono::high_resolution_clock``.
This clock is the clock that has the highest resolution out of all the
clocks in the system. It is normally an alias for either ``system_clock``
or ``steady_clock``, but can be its own independent clock. Due
to this uncertainty, conversions of time measured on the
``high_resolution_clock`` to Python produce platform-dependent types:
you'll get a :py:class:`~datetime.datetime` if ``high_resolution_clock`` is
an alias for ``system_clock`` on your system, or a :py:class:`~datetime.timedelta`
value otherwise.

Provided conversions
^^^^^^^^^^^^^^^^^^^^

The C++ types described in this section may be instantiated with any
precision. Conversions to a less-precise type will round towards zero.
Since Python's built-in date and time objects support only microsecond
precision, any precision beyond that on the C++ side will be lost when
converting to Python.

.. rubric:: C++ to Python

- ``std::chrono::system_clock::time_point`` → :py:class:`datetime.datetime`
    A system clock time will be converted to a Python
    :py:class:`~datetime.datetime` instance.  The result describes a time in the
    local timezone, but does not have any timezone information
    attached to it (it is a naive datetime object).

- ``std::chrono::duration`` → :py:class:`datetime.timedelta`
    A duration will be converted to a Python :py:class:`~datetime.timedelta`.
    Any precision beyond microseconds is lost by rounding towards zero.

- ``std::chrono::[other_clock]::time_point`` → :py:class:`datetime.timedelta`
    A time on any clock except the system clock will be converted to a Python
    :py:class:`~datetime.timedelta`, which measures the number of seconds between
    the clock's epoch and the time point of interest.

.. rubric:: Python to C++

- :py:class:`datetime.datetime` or :py:class:`datetime.date` or :py:class:`datetime.time` → ``std::chrono::system_clock::time_point``
    A Python date, time, or datetime object can be converted into a
    system clock timepoint.  A :py:class:`~datetime.time` with no date
    information is treated as that time on January 1, 1970. A
    :py:class:`~datetime.date` with no time information is treated as midnight
    on that date. **Any timezone information is ignored.**

- :py:class:`datetime.timedelta` → ``std::chrono::duration``
    A Python time delta object can be converted into a duration
    that describes the same number of seconds (modulo precision limitations).

- :py:class:`datetime.timedelta` → ``std::chrono::[other_clock]::time_point``
    A Python time delta object can be converted into a timepoint on a
    clock other than the system clock. The resulting timepoint will be
    that many seconds after the target clock's epoch time.

- ``float`` → ``std::chrono::duration``
    A floating-point value can be converted into a duration. The input is
    treated as a number of seconds, and fractional seconds are supported
    to the extent representable.

- ``float`` → ``std::chrono::[other_clock]::time_point``
    A floating-point value can be converted into a timepoint on a
    clock other than the system clock. The input is treated as a
    number of seconds, and fractional seconds are supported to the
    extent representable. The resulting timepoint will be that many
    seconds after the target clock's epoch time.


Evaluating Python expressions from strings
------------------------------------------

The following functions can be used to evaluate Python functions and
expressions. They require an additional include directive:

.. code-block:: cpp

   #include <nanobind/eval.h>

Detailed documentation including example code is provided in a :ref:`separate
section <utilities_eval>`.

.. cpp:enum-class:: eval_mode

   This enumeration specifies how the content of a string should be
   interpreted. Used in Py_CompileString().

   .. cpp:enumerator:: eval_expr = Py_eval_input

      Evaluate a string containing an isolated expression

   .. cpp:enumerator:: eval_single_statement = Py_single_input

      Evaluate a string containing a single statement. Returns \c None

   .. cpp:enumerator:: eval_statements = Py_file_input

      Evaluate a string containing a sequence of statement. Returns \c None

.. cpp:function:: template <eval_mode start = eval_expr, size_t N> object eval(const char (&s)[N], handle global = handle(), handle local = handle())

   Evaluate the given Python code in the given global/local scopes, and return
   the value.

.. cpp:function:: inline void exec(const str &expr, handle global = handle(), handle local = handle())

   Execute the given Python code in the given global/local scopes.

Intrusive reference counting helpers
------------------------------------

The following functions and classes can be used to augment user-provided
classes with intrusive reference counting that greatly simplifies shared
ownership in larger C++/Python binding projects.

This functionality requires the following include directives:

.. code-block:: cpp

   #include <nanobind/intrusive/counter.h>
   #include <nanobind/intrusive/ref.h>

These headers reference several functions, whose implementation must be
provided. You can do so by including the following file from a single ``.cpp``
file of your project:

.. code-block:: cpp

   #include <nanobind/intrusive/counter.inl>

The functionality in these files consist of the following classes and
functions:

.. cpp:class:: intrusive_counter

   Simple atomic reference counter that can optionally switch over to
   Python-based reference counting.

   The various copy/move assignment/constructors intentionally don't transfer
   the reference count. This is so that the contents of classes containing an
   ``intrusive_counter`` can be copied/moved without disturbing the reference
   counts of the associated instances.

   .. cpp:function:: intrusive_counter() noexcept = default

      Initialize with a reference count of zero.

   .. cpp:function:: intrusive_counter(const intrusive_counter &o)

      Copy constructor, which produces a zero-initialized counter.
      Does *not* copy the reference count from `o`.

   .. cpp:function:: intrusive_counter(intrusive_counter &&o)

      Move constructor, which produces a zero-initialized counter.
      Does *not* copy the reference count from `o`.

   .. cpp:function:: intrusive_counter &operator=(const intrusive_counter &o)

      Copy assignment operator. Does *not* copy the reference count from `o`.

   .. cpp:function:: intrusive_counter &operator=(intrusive_counter &&o)

      Move assignment operator. Does *not* copy the reference count from `o`.

   .. cpp:function:: void inc_ref() const noexcept

      Increase the reference count. When the counter references an object
      managed by Python, the operation calls ``Py_INCREF()`` to increase
      the reference count of the Python object instead.

      The :cpp:func:`inc_ref() <nanobind::inc_ref>` top-level function
      encapsulates this logic for subclasses of :cpp:class:`intrusive_base`.

   .. cpp:function:: bool dec_ref() const noexcept

      Decrease the reference count. When the counter references an object
      managed by Python, the operation calls ``Py_DECREF()`` to decrease
      the reference count of the Python object instead.

      When the C++-managed reference count reaches zero, the operation returns
      ``true`` to signal to the caller that it should use a *delete expression*
      to destroy the instance.

      The :cpp:func:`dec_ref() <nanobind::dec_ref>` top-level function
      encapsulates this logic for subclasses of :cpp:class:`intrusive_base`.

   .. cpp:function:: void set_self_py(PyObject * self)

      Set the Python object associated with this instance. This operation
      is usually called by nanobind when ownership is transferred to the
      Python side.

      Any references from prior calls to
      :cpp:func:`intrusive_counter::inc_ref()` are converted into Python
      references by calling ``Py_INCREF()`` repeatedly.

   .. cpp:function:: PyObject * self_py()

      Return the Python object associated with this instance (or ``nullptr``).

.. cpp:class:: intrusive_base

   Simple polymorphic base class for a intrusively reference-counted object
   hierarchy. The member functions expose corresponding functionality of
   :cpp:class:`intrusive_counter`.

   .. cpp:function:: void inc_ref() const noexcept

      See :cpp:func:`intrusive_counter::inc_ref()`.

   .. cpp:function:: bool dec_ref() const noexcept

      See :cpp:func:`intrusive_counter::dec_ref()`.

   .. cpp:function:: void set_self_py(PyObject * self)

      See :cpp:func:`intrusive_counter::set_self_py()`.

   .. cpp:function:: PyObject * self_py()

      See :cpp:func:`intrusive_counter::self_py()`.

.. cpp:function:: void intrusive_init(void (* intrusive_inc_ref_py)(PyObject * ) noexcept, void (* intrusive_dec_ref_py)(PyObject * ) noexcept)

   Function to register reference counting hooks with the intrusive reference
   counter class. This allows its implementation to not depend on Python.

   You would usually call this function as follows from the initialization
   routine of a Python extension:

   .. code-block:: cpp

      NB_MODULE(my_ext, m) {
          nb::intrusive_init(
              [](PyObject * o) noexcept {
                  nb::gil_scoped_acquire guard;
                  Py_INCREF(o);
              },
              [](PyObject * o) noexcept {
                  nb::gil_scoped_acquire guard;
                  Py_DECREF(o);
              });

          // ...
      }

.. cpp:function:: inline void inc_ref(intrusive_base * o) noexcept

   Reference counting helper function that calls ``o->inc_ref()`` if ``o`` is
   not equal to ``nullptr``.

.. cpp:function:: inline void dec_ref(intrusive_base * o) noexcept

   Reference counting helper function that calls ``o->dec_ref()`` if ``o`` is
   not equal to ``nullptr`` and ``delete o`` when the reference count reaches
   zero.

.. cpp:class:: template <typename T> ref

   RAII scoped reference counting helper class

   :cpp:class:`ref\<T\> <ref>` is a simple RAII wrapper class that encapsulates a
   pointer to an instance with intrusive reference counting.

   It takes care of increasing and decreasing the reference count as needed and
   deleting the instance when the count reaches zero.

   For this to work, compatible functions :cpp:func:`inc_ref()` and
   :cpp:func:`dec_ref()` must be defined before including the file
   ``nanobind/intrusive/ref.h``. Default implementations for subclasses of the
   type :cpp:class:`intrusive_base` are already provided as part of the file
   ``counter.h``.

   .. cpp:function:: ref() = default

      Create a null reference

   .. cpp:function:: ref(T * ptr)

      Create a reference from a pointer. Increases the reference count of the
      object (if not ``nullptr``).

   .. cpp:function:: ref(const ref &r)

      Copy a reference. Increase the reference count of the object (if not
      ``nullptr``).

   .. cpp:function:: ref(ref &&r) noexcept

      Move a reference. Object reference counts are unaffected by this operation.

   .. cpp:function:: ~ref()

      Destroy a reference. Decreases the reference count of the object (if not
      ``nullptr``).

   .. cpp:function:: ref& operator=(ref &&r) noexcept

      Move-assign another reference into this one.

   .. cpp:function:: ref& operator=(const ref &r)

      Copy-assign another reference into this one.

   .. cpp:function:: ref& operator=(const T * ptr)

      Overwrite this reference with a pointer to another object

   .. cpp:function:: void reset()

      Clear the reference and reduces the reference count of the object (if not
      ``nullptr``)

   .. cpp:function:: bool operator==(const ref &r) const

      Compare this reference with another reference (pointer equality)

   .. cpp:function:: bool operator!=(const ref &r) const

      Compare this reference with another reference (pointer inequality)

   .. cpp:function:: bool operator==(const T * ptr) const

      Compare this reference with another object (pointer equality)

   .. cpp:function:: bool operator!=(const T * ptr) const

      Compare this reference with another object (pointer inequality)

   .. cpp:function:: T * operator->()

      Access the object referenced by this reference

   .. cpp:function:: const T * operator->() const

      Access the object referenced by this reference (const version)

   .. cpp:function:: T& operator*()

      Return a C++ reference to the referenced object

   .. cpp:function:: const T& operator*() const

      Return a C++ reference to the referenced object (const version)

   .. cpp:function:: T* get()

      Return a C++ pointer to the referenced object

   .. cpp:function:: const T* get() const

      Return a C++ pointer to the referenced object (const version)

Typing
------

The following functions for typing-related functionality require an additional
include directive:

.. code-block:: cpp

   #include <nanobind/typing.h>

.. cpp:function:: template <typename... Args> object type_var(Args&&... args)

   Create a `type variable
   <https://docs.python.org/3/library/typing.html#typing.TypeVar>`__ (i.e., an
   instance of ``typing.TypeVar``). All arguments of the original Python
   construction are supported, e.g.:

   .. code-block:: cpp

        m.attr("T") = nb::type_var("T",
                                   "contravariant"_a = true,
                                   "covariant"_a = false,
                                   "bound"_a = nb::type<MyClass>());


.. cpp:function:: template <typename... Args> object type_var_tuple(Args&&... args)

   Analogousto :cpp:func:`type_var`, create a `type variable tuple
   <https://docs.python.org/3/library/typing.html#typing.TypeVarTuple>`__
   (i.e., an instance of ``typing.TypeVarTuple``).

.. cpp:function:: object any_type()

   Convenience wrapper, which returns ``typing.Any``.
