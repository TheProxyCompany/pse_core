.. _ownership:

.. cpp:namespace:: nanobind

Object ownership
================

Python and C++ don't manage the lifetime and storage of objects in the same
way. Consequently, two questions arise whenever an object crosses the language
barrier:

- Who actually *owns* this object? C++? Python? Both?!

- Can we safely determine when it is no longer needed?

This is important: we *must* exclude the possibility that Python destroys an
object that is still being used by C++ (or vice versa).

The :ref:`previous section <exchange>` introduced three ways of exchanging
information between C++ and Python: :ref:`type casters <type_casters>`,
:ref:`bindings <bindings>`, and :ref:`wrappers <wrappers>`.
It is specifically
:ref:`bindings <bindings>` for which these two questions must be answered.

.. _ownership_problem:

A problematic example
---------------------

Consider the following problematic example to see what can go wrong:

.. code-block:: cpp

   #include <nanobind/nanobind.h>
   namespace nb = nanobind;

   struct Data { };
   Data data; // Data global variable & function returning a pointer to it
   Data *get_data() { return &data; }

   NB_MODULE(my_ext, m) {
       nb::class_<Data>(m, "Data");

       // KABOOM, calling this function will crash the Python interpreter
       m.def("get_data", &get_data);
   }

The bound function ``my_ext.get_data()`` returns a Python object of type
``my_ext.Data`` that wraps the pointer ``&data`` and takes ownership of it.

When Python eventually garbage collects the object, nanobind will try to free
the (non-heap-allocated) C++ instance via ``operator delete``, causing a
segmentation fault.

To avoid this problem, we can

1. **Provide more information**: the problem was that nanobind *incorrectly*
   transferred ownership of a C++ instance to the Python side. To fix this, we
   can add add a :ref:`return value policy <rvp>` annotation that clarifies
   what to do with the return value.

2. **Make ownership transfer explicit**: C++ types passed via :ref:`unique
   pointers <unique_ptr>` (``std::unique_ptr<T>``) make the ownership transfer
   explicit in the type system, which would have revealed the problem in this
   example.

3. **Switch to shared ownership**: C++ types passed via :ref:`shared pointers
   <shared_ptr>` (``std::shared_ptr<T>``), or which use :ref:`intrusive
   reference counting <intrusive_intro>` can be shared by C++ and Python. The
   whole issue disappears because ownership transfer is no longer needed.

The remainder of this section goes through each of these options.

.. _rvp:

Return value policies
---------------------

nanobind provides several *return value policy* annotations that can be
passed to :func:`module_::def`, :func:`class_::def`, and :func:`cpp_function`.
The default policy is :cpp:enumerator:`rv_policy::automatic`, which is usually
a reasonable default (but not in this case!).

In the :ref:`problematic example <ownership_problem>`, the policy
:cpp:enumerator:`rv_policy::reference` should have been specified explicitly so
that the global instance is only *referenced* without any implied transfer of
ownership, i.e.:

.. code-block:: cpp

    m.def("get_data", &get_data, nb::rv_policy::reference);

On the other hand, this is not the right policy for many other situations,
where ignoring ownership could lead to resource leaks. As a developer using
this library, it is important that you familiarize yourself with the different
options below. In particular, the following policies are available:

- :cpp:enumerator:`rv_policy::take_ownership`:
  Create a thin Python object wrapper around the returned C++ instance without
  making a copy and transfer ownership to Python. When the
  Python wrapper is eventually garbage collected, nanobind will call the C++
  ``delete`` operator to free the C++ instance.

  In the example below, a function uses this policy to transfer ownership of a
  heap-allocated C++ instance to Python:

  .. code-block:: cpp

     m.def("make_data", []{ return new Data(); }, nb::rv_policy::take_ownership);

  The return value policy declaration could actually have been omitted here
  because :cpp:enumerator:`take_ownership <rv_policy::take_ownership>` is the
  default for *pointer return values* (see :cpp:enumerator:`automatic
  <rv_policy::automatic>`).

- :cpp:enumerator:`rv_policy::copy`:
  Copy-construct a new Python object from the C++ instance. The copy will be
  owned by Python, while C++ retains ownership of the original.

  In the example below, a function uses this policy to return a reference to a
  C++ instance. The owner and lifetime of such a reference may not be clear, so
  the safest route is to make a copy.

  .. code-block:: cpp

     struct A {
        B &b() { /* .. unknown code .. */ }
     };

     nb::class_<A>(m, "A")
        .def("b", &A::b, nb::rv_policy::copy);

  The return value policy declaration could actually have been omitted here
  because :cpp:enumerator:`copy <rv_policy::copy>` is the default for *lvalue
  reference* return values (see :cpp:enumerator:`automatic
  <rv_policy::automatic>`).

- :cpp:enumerator:`rv_policy::move`:
  Move-construct a new Python object from the C++ instance. The new object will
  be owned by Python, while C++ retains ownership of the original (whose
  contents were likely invalidated by the move operation).

  In the example below, a function uses this policy to return a C++ instance by
  value. The :cpp:enumerator:`copy <rv_policy::copy>` operation mentioned above
  would also be safe to use, but move construction has the potential of being
  significantly more efficient.

  .. code-block:: cpp

     struct A {
        B b() { return B(...); }
     };

     nb::class_<A>(m, "A")
        .def("b", &A::b, nb::rv_policy::move);

  The return value policy declaration could actually have been omitted here
  because :cpp:enumerator:`move <rv_policy::move>` is the default for *functions
  that return by value* (see :cpp:enumerator:`automatic
  <rv_policy::automatic>`).

- :cpp:enumerator:`rv_policy::reference`:
  Create a thin Python object wrapper around the returned C++ instance without
  making a copy, but *do not transfer ownership to Python*. nanobind will never
  call the C++ ``delete`` operator, even when the wrapper expires.
  The C++ side is responsible for destructing the C++ instance.

  This return value policy is *dangerous* and should be used cautiously.
  Undefined behavior will ensue when the C++ side deletes the instance while it
  is still being used by Python. If you need to use this policy, combine it with
  a :cpp:struct:`keep_alive` function binding annotation to manage the lifetime.
  Or use the simple and safe :cpp:enumerator:`reference_internal
  <rv_policy::reference_internal>` alternative described next.

  Below is an example use of this return value policy to reference a
  global variable that does not need ownership and lifetime management.

  .. code-block:: cpp

     Data data; // This is a global variable

     m.def("get_data", []{ return &data; }, nb::rv_policy::reference)

- :cpp:enumerator:`rv_policy::reference_internal`: A policy for *methods* that
  expose an internal field. The lifetime of the field must match that of the
  parent object.

  The policy resembles :cpp:enumerator:`reference <rv_policy::reference>` in
  that it creates creates a thin Python object wrapper around the returned C++
  field without making a copy, and without transferring ownership to Python.

  Furthermore, it ensures that the instance owning the field (implicit
  ``this``/``self`` argument) cannot be garbage collected while an object
  representing the field is alive.

  The example below uses this policy to implement a *getter* that permits
  mutable access to an internal field.

  .. code-block:: cpp

      struct MyClass {
      public:
          MyField &field() { return m_field; }

      private:
          MyField m_field;
      };

      nb::class_<MyClass>(m, "MyClass")
         .def("field", &MyClass::field, nb::rv_policy::reference_internal);

  More advanced variations of this scheme are also possible using combinations
  of :cpp:enumerator:`reference <rv_policy::reference>` and the
  :cpp:struct:`keep_alive` function binding annotation.

- :cpp:enumerator:`rv_policy::none`: This is the most conservative policy: it
  simply refuses the cast unless the C++ instance already has a corresponding
  Python object, in which case the question of ownership becomes moot.

- :cpp:enumerator:`rv_policy::automatic`: This is the default return value
  policy, which falls back to :cpp:enumerator:`take_ownership
  <rv_policy::automatic>` when the return value is a pointer,
  :cpp:enumerator:`move <rv_policy::move>` when it is a rvalue reference, and
  :cpp:enumerator:`copy <rv_policy::copy>` when it is a lvalue reference.

- :cpp:enumerator:`rv_policy::automatic_reference`: This policy matches
  :cpp:enumerator:`automatic <rv_policy::automatic>` but falls back to
  :cpp:enumerator:`reference <rv_policy::refernece>` when the return value is a
  pointer. It is the default for function arguments when calling Python
  functions from C++ code via :cpp:func:`detail::api::operator()`. You probably
  won't need to use this policy in your own code.

.. _unique_ptr:

Unique pointers
---------------

Passing a STL unique pointer embodies an ownership transfer---a return value
policy annotation is therefore not needed. To bind functions that receive or
return ``std::unique_ptr<..>``, add the extra include directive

.. code-block:: cpp

   #include <nanobind/stl/unique_ptr.h>

.. note::

   While this this header file technically contains a :ref:`type caster
   <type_casters>`, it is *not* affected by their usual limitations (mandatory
   copy/conversion, inability to mutate function arguments).

**Example**: The following example binds two functions that create and consume
instances of a C++ type ``Data`` via unique pointers.

.. code-block:: cpp

   #include <nanobind/stl/unique_ptr.h>

   namespace nb = nanobind;

   NB_MODULE(my_ext, m) {
       struct Data { };
       nb::class_<Data>(m, "Data");
       m.def("create", []() { return std::make_unique<Data>(); });
       m.def("consume", [](std::unique_ptr<Data> x) { /* no-op */ });
   }

Calling a function taking a unique pointer from Python invalidates the passed
Python object. nanobind will refuse further use of it:

.. code-block:: pycon
   :emphasize-lines: 8,9

   Python 3.11.1 (main, Dec 23 2022, 09:28:24) [Clang 14.0.0 (clang-1400.0.29.202)] on darwin
   Type "help", "copyright", "credits" or "license" for more information.
   >>> import my_ext

   >>> x = my_ext.create()
   >>> my_ext.consume(x)

   >>> my_ext.consume(x)
   <stdin>:1: RuntimeWarning: nanobind: attempted to access an uninitialized instance of type 'my_ext.Data'!

   TypeError: consume(): incompatible function arguments. The following argument types are supported:
       1. consume(arg: my_ext.Data, /) -> None

   Invoked with types: my_ext.Data

We strongly recommend that you replace all use of ``std::unique_ptr<T>`` by
``std::unique_ptr<T, nb::deleter<T>>`` in your code. Without the latter type
declaration, which references a custom nanobind-provided deleter
:cpp:class:`nb::deleter\<T\> <deleter>`,  nanobind cannot transfer ownership of
objects constructed using :cpp:class:`nb::init\<...\> <init>` to C++ and will
refuse to do so with an error message. Further detail on this special case can
be found in the *advanced* :ref:`section <unique_ptr_adv>` on object ownership.

.. _shared_ownership:

Shared ownership
----------------

In a *shared ownership* model, an object can have multiple owners that each
register their claim by holding a *reference*. The system keeps track of the
total number of references and destroys the object once the count reaches zero.
Passing such an object in a function call shares ownership between the caller
and callee. nanobind makes this behavior seamless so that everything works
regardless of whether caller/callee are written in C++ or Python.

.. _shared_ptr:

Shared pointers
^^^^^^^^^^^^^^^

STL shared pointers (``std::shared_ptr<T>``) allocate a separate control block to
keep track of the reference count, which makes them very general but also slightly
less efficient than other alternatives.

nanobind's support for shared pointers requires an extra include directive:

.. code-block:: cpp

   #include <nanobind/stl/shared_ptr.h>

.. note::

   While this this header file technically contains a :ref:`type caster
   <type_casters>`, it is *not* affected by their usual limitations (mandatory
   copy/conversion, inability to mutate function arguments).

You don't need to specify a return value policy annotation when a function
returns a shared pointer.

nanobind's implementation of ``std::shared_ptr`` support typically
allocates a new ``shared_ptr`` control block each time a Python object
must be converted to ``std::shared_ptr<T>``. The new ``shared_ptr``
"owns" a reference to the Python object, and its deleter drops that
reference.  This has the advantage that the Python portion of the
object will be kept alive by its C++-side references (which is
important when implementing C++ virtual methods in Python), but it can
be inefficient when passing the same object back and forth between
Python and C++ many times, and it means that the ``use_count()``
method of ``std::shared_ptr`` will return a value that does not
capture all uses. Some of these problems can be mitigated by modifying
``T`` so that it inherits from ``std::enable_shared_from_this<T>``.
See the :ref:`advanced section <shared_ptr_adv>` on object ownership
for more details on the implementation.

nanobind has limited support for objects that inherit from
``std::enable_shared_from_this<T>`` to allow safe conversion of raw
pointers to shared pointers. The safest way to deal with these objects
is to always use ``std::make_shared<T>(...)`` when constructing them in C++,
and always pass them across the Python/C++ boundary wrapped in an explicit
``std::shared_ptr<T>``. If you do this, then there shouldn't be any
surprises. If you will be passing raw ``T*`` pointers around, then
read the :ref:`advanced section on object ownership <enable_shared_from_this>`
for additional caveats.

.. _intrusive_intro:

Intrusive reference counting
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Intrusive reference counting is the most flexible and efficient way of handling
shared ownership. The main downside is that you must adapt the base class of
your object hierarchy to the needs of nanobind.

The core idea is to define base class (e.g. ``Object``) common to all bound
types requiring shared ownership. That class contains a builtin atomic counter
(e.g., ``m_ref_count``) and a Python object pointer (e.g., ``m_py_object``).

.. code-block:: cpp

   class Object {
   ...
   private:
       mutable std::atomic<size_t> m_ref_count { 0 };
       PyObject *m_py_object = nullptr;
   };

The core idea is that such ``Object`` instances can either be managed by C++ or
Python. In the former case, the ``m_ref_count`` field keeps track of the number
of outstanding references. In the latter case, reference counting is handled by
Python, and the ``m_ref_count`` field remains unused.

This is actually little wasteful---nanobind therefore ships with a more
efficient reference counter sample implementation that supports both use cases
while requiring only ``sizeof(void*)`` bytes of storage:

.. code-block:: cpp

   #include <nanobind/intrusive/counter.h>

   class Object {
   ...
   private:
       intrusive_counter m_ref_count;
   };


Object ownership, continued
===========================

This section covers intrusive reference counting as an alternative to shared
pointers, and it explains the nitty-gritty details of how shared and unique
pointer conversion is implemented in nanobind.

.. _intrusive:

Intrusive reference counting
----------------------------

nanobind provides a custom intrusive reference counting solution that
completely solves the issue of shared C++/Python object ownership, while
avoiding the overheads and complexities of traditional C++ shared pointers
(``std::shared_ptr<T>``).

The main limitation is that it requires adapting the base class of an object
hierarchy according to the needs of nanobind, which may not always be possible.

Motivation
^^^^^^^^^^

Consider the following simple class with intrusive reference counting:

.. code-block:: cpp

   class Object {
   public:
       void inc_ref() const noexcept { ++m_ref_count; }

       void dec_ref() const noexcept {
           if (--m_ref_count == 0)
               delete this;
       }

   private:
       mutable std::atomic<size_t> m_ref_count { 0 };
   };

It contains an atomic counter that stores the number of references. When the
counter reaches zero, the object deallocates itself. Easy and efficient.

The advantage of over ``std::shared_ptr<T>`` is that no separate control block
must be allocated. Technical band-aids like ``std::enable_shared_from_this<T>``
can also be avoided, since the reference count is always found in the object
itself.

However, one issue that tends to arise when a type like ``Object`` is
wrapped using nanobind is that there are now *two* separate reference counts
referring to the same object: one in Python’s ``PyObject``, and one in
``Object``. This can lead to a problematic reference cycle:

- Python’s ``PyObject`` needs to keep the ``Object`` instance alive so that it
  can be safely passed to C++ functions.

- The C++ ``Object`` may in turn need to keep the ``PyObject`` alive. This
  is the case when a subclass uses *trampolines* (:c:macro:`NB_TRAMPOLINE`,
  :c:macro:`NB_OVERRIDE`) to catch C++ virtual function calls and
  potentially dispatch them to an overridden implementation in Python. In
  this case, the C++ instance needs to be able to perform a function call on
  its own Python object identity, which requires a reference.

The source of the problem is that there are *two* separate counters that try
to reason about the reference count of *one* instance, which leads to an
uncollectable inter-language reference cycle.

The solution
^^^^^^^^^^^^
We can solve the problem by using just one counter:

- if an instance lives purely on the C++ side, the ``m_ref_count``
  field is used to reason about the number of references.

- The first time that an instance is exposed to Python (by being
  created from Python, or by being returned from a bound C++ function),
  lifetime management switches over to Python.

The file `nanobind/intrusive/counter.h
<https://github.com/wjakob/nanobind/blob/master/include/nanobind/intrusive/counter.h>`_
includes an official sample implementation of this functionality. It contains an extra optimization to pack *either*
a reference counter or a pointer to a ``PyObject*`` into a single
``sizeof(void*)``-sized field.

The most basic interface, :cpp:class:`intrusive_counter` represents an atomic
counter that can be increased (via :cpp:func:`intrusive_counter::inc_ref()`) or
decreased (via :cpp:func:`intrusive_counter::dec_ref()`). When the counter
reaches zero, the object should be deleted, which ``dec_ref()`` indicates by
returning ``true``.

In addition to this simple counting mechanism, ownership of the object can also
be transferred to Python (via :cpp:func:`intrusive_counter::set_self_py()`). In
this case, subsequent calls to ``inc_ref()`` and ``dec_ref()`` modify the
reference count of the underlying Python object.

To incorporate intrusive reference counting into your own project, you would
usually add an :cpp:class:`intrusive_counter`-typed member to the base class of an object
hierarchy and expose it as follows:

.. code-block:: cpp

   #include <nanobind/intrusive/counter.h>

   class Object {
   public:
       void inc_ref() noexcept { m_ref_count.inc_ref(); }
       bool dec_ref() noexcept { return m_ref_count.dec_ref(); }

       // Important: must declare virtual destructor
       virtual ~Object() = default;

       void set_self_py(PyObject *self) noexcept {
           m_ref_count.set_self_py(self);
       }

   private:
       nb::intrusive_counter m_ref_count;
   };

   // Convenience function for increasing the reference count of an instance
   inline void inc_ref(Object *o) noexcept {
       if (o)
          o->inc_ref();
   }

   // Convenience function for decreasing the reference count of an instance
   // and potentially deleting it when the count reaches zero
   inline void dec_ref(Object *o) noexcept {
       if (o && o->dec_ref())
           delete o;
   }

Alternatively, you could also inherit from :cpp:class:`intrusive_base`, which
obviates the need for all of the above declarations:

.. code-block:: cpp

   class Object : public nb::intrusive_base {
   public:
       // ...
   };

The main change in the bindings is that the base class must specify a
:cpp:class:`nb::intrusive_ptr <intrusive_ptr>` annotation to inform an instance
that lifetime management has been taken over by Python. This annotation is
automatically inherited by all subclasses. In the linked example, this is done
via the ``Object::set_self_py()`` method that we can now call from the class
binding annotation:

.. code-block:: cpp

   nb::class_<Object>(
     m, "Object",
     nb::intrusive_ptr<Object>(
         [](Object *o, PyObject *po) noexcept { o->set_self_py(po); }));

Also, somewhere in your binding initialization code, you must register Python
reference counting hooks with the intrusive reference counter class. This
allows its implementation of the code in ``nanobind/intrusive/counter.h`` to
*not* depend on Python (this means that it can be used in projects where Python
bindings are an optional component).

.. code-block:: cpp

   nb::intrusive_init(
       [](PyObject *o) noexcept {
           nb::gil_scoped_acquire guard;
           Py_INCREF(o);
       },
       [](PyObject *o) noexcept {
           nb::gil_scoped_acquire guard;
           Py_DECREF(o);
       });

These ``counter.h`` include file references several functions that must be
compiled somewhere inside the project, which can be accomplished by including
the following file from a single ``.cpp`` file.

.. code-block:: cpp

   #include <nanobind/intrusive/counter.inl>

Having to call :cpp:func:`inc_ref()` and :cpp:func:`dec_ref()` many times to
perform manual reference counting in project code can quickly become tedious.
Nanobind also ships with a :cpp:class:`ref\<T\> <ref>` RAII helper class to
help with this.

.. code-block:: cpp

   #include <nanobind/intrusive/ref.h>

   void foo() {
       /// Assignment to ref<T> automatically increases the object's reference count
       ref<MyObject> x = new MyObject();

       // ref<T> can be used like a normal pointer
       x->func();

   } // <-- ref::~ref() calls dec_ref(), which deletes the now-unreferenced instance

When the file ``nanobind/intrusive/ref.h`` is included following
``nanobind/nanobind.h``, it also exposes a custom type caster to bind functions
taking or returning ``ref<T>``-typed values.

That's it. If you use this approach, any potential issues involving shared
pointers, return value policies, reference leaks with trampolines, etc., can
be avoided from the beginning.