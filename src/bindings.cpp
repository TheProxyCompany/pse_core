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

static int dummy_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    return PyBaseObject_Type.tp_init(self, args, kwds);
}

static PyType_Slot init_slots[] = {
    {Py_tp_init, (void *)dummy_init},
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

    nb::class_<StateMachine, PyStateMachine>(
        m, "StateMachine",
        nb::type_slots(init_slots),
        nb::intrusive_ptr<StateMachine>([](StateMachine *o, PyObject *po) noexcept
                                        { o->set_self_py(po); }))
        .def(
            nb::init<
                StateMachine::StateGraph,
                StateMachine::State,
                std::vector<StateMachine::State>,
                bool,
                bool>(),
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
        .def_static(
            "advance_all",
            nb::overload_cast<std::vector<nb::ref<Walker>> &, const std::string &>(&StateMachine::advance_all),
            "Advance multiple walkers with a token")
        .def_static(
            "advance_all",
            nb::overload_cast<std::vector<nb::ref<Walker>> &, const std::string &, const tsl::htrie_set<char> &>(&StateMachine::advance_all),
            "Advance multiple walkers with a token, validating against vocabulary")
        .def("__eq__", &StateMachine::operator==)
        .def("__repr__", &StateMachine::to_string);

    nb::class_<Walker, PyWalker>(
        m, "Walker",
        nb::type_slots(init_slots),
        nb::intrusive_ptr<Walker>([](Walker *o, PyObject *po) noexcept
                                  { o->set_self_py(po); }))
        .def(
            nb::init<nb::ref<StateMachine>, std::optional<StateMachine::State>>(),
            "state_machine"_a,
            "current_state"_a = nb::none())
        .def_prop_rw(
            "state_machine",
            [](Walker &w)
            { return w.state_machine_; },
            [](Walker &w, StateMachine *sm)
            { w.state_machine_ = sm; })
        .def_prop_rw(
            "_raw_value",
            [](const Walker &w) -> std::optional<std::string>
            { return w._raw_value_; },
            [](Walker &w, std::optional<std::string> v)
            { w._raw_value_ = v; },
            nb::arg().none())
        .def_rw("accepted_history", &Walker::accepted_history_)
        .def_rw("explored_edges", &Walker::explored_edges_)
        .def_rw("current_state", &Walker::current_state_)
        .def_rw("target_state", &Walker::target_state_)
        .def_rw("transition_walker", &Walker::transition_walker_)
        .def_rw("consumed_character_count", &Walker::consumed_character_count_)
        .def_rw("remaining_input", &Walker::remaining_input_, "remaining_input"_a = nb::none())
        .def_rw("_accepts_more_input", &Walker::_accepts_more_input_)

        .def("get_current_value", &Walker::get_current_value)
        .def("get_raw_value", &Walker::get_raw_value)
        .def("clone", &Walker::clone)
        .def("consume_token", &Walker::consume_token)
        .def("can_accept_more_input", &Walker::can_accept_more_input)
        .def("is_within_value", &Walker::is_within_value)
        .def("should_start_transition", &Walker::should_start_transition)
        .def("should_complete_transition", &Walker::should_complete_transition)
        .def("accepts_any_token", &Walker::accepts_any_token)
        .def("get_valid_continuations", &Walker::get_valid_continuations, "depth"_a = 0)
        .def("has_reached_accept_state", &Walker::has_reached_accept_state)
        .def("start_transition", &Walker::start_transition, "transition_walker"_a, "token"_a = nb::none(), "start_state"_a = nb::none(), "target_state"_a = nb::none())
        .def("complete_transition", &Walker::complete_transition)
        .def("branch", &Walker::branch, "token"_a = nb::none())
        .def("parse_value", &Walker::parse_value, "value"_a = nb::none())
        .def("__eq__", &Walker::operator==)
        .def("__repr__", &Walker::to_string);

    nb::class_<AcceptedState, Walker>(m, "AcceptedState")
        .def(nb::init<nb::ref<Walker>>())
        .def_prop_rw(
            "accepted_walker",
            [](AcceptedState &w)
            { return w.accepted_walker_; },
            [](AcceptedState &w, nb::ref<Walker> walker)
            { w.accepted_walker_ = walker; })
        .def("clone", &AcceptedState::clone)
        .def("can_accept_more_input", &AcceptedState::can_accept_more_input)
        .def("has_reached_accept_state", &AcceptedState::has_reached_accept_state)
        .def("is_within_value", &AcceptedState::is_within_value)
        .def("should_start_transition", &AcceptedState::should_start_transition)
        .def("consume_token", &AcceptedState::consume_token)
        .def("__eq__", &AcceptedState::operator==)
        .def("__repr__", &AcceptedState::to_string);
}
