#include "accepted_state.h"
#include "state_machine.h"
#include "state_machine_trampoline.h"
#include "walker.h"
#include "walker_trampoline.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/unordered_map.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <nanobind/intrusive/counter.inl>

namespace nb = nanobind;
using namespace nb::literals;

// dummy init function for Py_tp_init slot
static PyObject *dummy_init(PyObject *, PyObject *, PyObject *)
{
    PyErr_SetString(PyExc_RuntimeError, "This should never be called!");
    return nullptr;
}

extern "C"
{
    int statemachine_tp_traverse(PyObject *self, visitproc visit, void *arg);
    int statemachine_tp_clear(PyObject *self);
    int walker_tp_traverse(PyObject *self, visitproc visit, void *arg);
    int walker_tp_clear(PyObject *self);
    int acceptedstate_tp_traverse(PyObject *self, visitproc visit, void *arg);
    int acceptedstate_tp_clear(PyObject *self);
}

// define type slots for StateMachine
static PyType_Slot state_machine_slots[] = {
    {Py_tp_init, (void *)dummy_init},
    {Py_tp_traverse, (void *)statemachine_tp_traverse},
    {Py_tp_clear, (void *)statemachine_tp_clear},
    {0, nullptr}};

// define type slots for Walker
static PyType_Slot walker_slots[] = {
    {Py_tp_init, (void *)dummy_init},
    {Py_tp_traverse, (void *)walker_tp_traverse},
    {Py_tp_clear, (void *)walker_tp_clear},
    {0, nullptr}};

// define type slots for AcceptedState if it forms cycles
static PyType_Slot acceptedstate_slots[] = {
    {Py_tp_init, (void *)dummy_init},
    {Py_tp_traverse, (void *)acceptedstate_tp_traverse},
    {Py_tp_clear, (void *)acceptedstate_tp_clear},
    {0, nullptr}};

NB_MODULE(_core, m)
{
     m.doc() = R"pbdoc(
        PSE Core C++ Bindings
        ---------------------

        .. currentmodule:: _core

        .. autosummary::
           :toctree: _generate

           StateMachine
           Walker
    )pbdoc";

     nb::intrusive_init(
         [](PyObject *o) noexcept
         {
             nb::gil_scoped_acquire guard;
             Py_INCREF(o);
         },
         [](PyObject *o) noexcept
         {
             nb::gil_scoped_acquire guard;
             Py_DECREF(o);
         });

     // Bind the StateMachine class with trampoline support
     nb::class_<StateMachine, PyStateMachine>(
         m, "StateMachine",
         nb::intrusive_ptr<StateMachine>([](StateMachine *o, PyObject *po) noexcept
                                         { o->set_self_py(po); }),
         nb::type_slots(state_machine_slots))
         .def(nb::init<StateMachine::StateGraph, StateMachine::State, std::vector<StateMachine::State>, bool, bool>(),
              nb::arg("state_graph") = StateMachine::StateGraph(),
              nb::arg("start_state") = 0,
              nb::arg("end_states") = std::vector<StateMachine::State>{"$"},
              nb::arg("is_optional") = false,
              nb::arg("is_case_sensitive") = true)
         .def_rw("state_graph", &StateMachine::state_graph_)
         .def_rw("start_state", &StateMachine::start_state_)
         .def_rw("end_states", &StateMachine::end_states_)
         .def_rw("is_optional", &StateMachine::is_optional_)
         .def_rw("is_case_sensitive", &StateMachine::is_case_sensitive_)
         .def("get_new_walker", &StateMachine::get_new_walker, nb::arg("state") = nb::none())
         .def("get_walkers", &StateMachine::get_walkers, nb::arg("state") = nb::none())
         .def("get_edges", &StateMachine::get_edges, nb::arg("state"))
         .def("get_transitions", &StateMachine::get_transitions, nb::arg("walker"), nb::arg("state") = nb::none())
         .def("advance", &StateMachine::advance, nb::arg("walker"), nb::arg("token"))
         .def("branch_walker", &StateMachine::branch_walker, nb::arg("walker"), nb::arg("token") = nb::none())
         .def_static("advance_all", &StateMachine::advance_all)
         .def("__eq__", &StateMachine::operator==)
         .def("__str__", &StateMachine::to_string)
         .def("__repr__", &StateMachine::__repr__);

     // Bind the Walker class with trampoline support
     nb::class_<Walker, PyWalker>(
         m, "Walker",
         nb::intrusive_ptr<Walker>([](Walker *o, PyObject *po) noexcept
                                   { o->set_self_py(po); }),
         nb::type_slots(walker_slots))
         .def(
             nb::init<nb::ref<StateMachine>, std::optional<StateMachine::State>>(),
             "state_machine"_a, "current_state"_a = nb::none())

         // Properties
         .def("get_current_value", &Walker::get_current_value)
         .def("get_raw_value", &Walker::get_raw_value)

         // Read/Write properties
         .def_rw("state_machine", &Walker::state_machine_)
         .def_rw("accepted_history", &Walker::accepted_history_)
         .def_rw("explored_edges", &Walker::explored_edges_)
         .def_rw("current_state", &Walker::current_state_)
         .def_rw("target_state", &Walker::target_state_)
         .def_rw("transition_walker", &Walker::transition_walker_)
         .def_rw("consumed_character_count", &Walker::consumed_character_count_)
         .def_rw("remaining_input", &Walker::remaining_input_, "remaining_input"_a = nb::none())
         .def_rw("_raw_value", &Walker::_raw_value_)
         .def_rw("_accepts_more_input", &Walker::_accepts_more_input_)

         // Pure virtual methods
         .def("clone", &Walker::clone)

         // Virtual methods with default implementations
         .def("consume_token", &Walker::consume_token)
         .def("can_accept_more_input", &Walker::can_accept_more_input)
         .def("is_within_value", &Walker::is_within_value)
         .def("should_start_transition", &Walker::should_start_transition)
         .def("should_complete_transition", &Walker::should_complete_transition)
         .def("accepts_any_token", &Walker::accepts_any_token)
         .def("get_valid_continuations", &Walker::get_valid_continuations,
              "depth"_a = 0)
         .def("has_reached_accept_state", &Walker::has_reached_accept_state)

         // Non-virtual methods
         .def("start_transition", &Walker::start_transition, "transition_walker"_a,
              "token"_a = nb::none(), "start_state"_a = nb::none(),
              "target_state"_a = nb::none())
         .def("complete_transition", &Walker::complete_transition)
         .def("branch", &Walker::branch, "token"_a = nb::none())

         // Special methods
         .def("__eq__", &Walker::operator==)
         .def("__str__", &Walker::to_string)
         .def("__repr__", &Walker::__repr__);

     nb::class_<AcceptedState, Walker>(
         m, "AcceptedState",
         nb::intrusive_ptr<AcceptedState>([](AcceptedState *o, PyObject *po) noexcept
                                          { o->set_self_py(po); }),
         nb::type_slots(acceptedstate_slots))
         .def(nb::init<nb::ref<Walker>>())
         .def("clone", &AcceptedState::clone)
         .def("can_accept_more_input", &AcceptedState::can_accept_more_input)
         .def("has_reached_accept_state", &AcceptedState::has_reached_accept_state)
         .def("is_within_value", &AcceptedState::is_within_value)
         .def("should_start_transition", &AcceptedState::should_start_transition)
         .def("consume_token", &AcceptedState::consume_token)
         .def("__eq__", &AcceptedState::operator==)
         .def("__repr__", &AcceptedState::__repr__);
}
