#pragma once
#include <nanobind/trampoline.h>
#include "state_machine.h"
#include "walker.h"

class PyStateMachine : public StateMachine
{
    NB_TRAMPOLINE(StateMachine, 8);

    nb::ref<Walker> get_new_walker(std::optional<State> state = std::nullopt) override
    {
        NB_OVERRIDE(get_new_walker, state);
    }

    std::vector<nb::ref<Walker>> get_walkers(std::optional<State> state = std::nullopt) override
    {
        NB_OVERRIDE(get_walkers, state);
    }

    std::vector<Edge> get_edges(State state) const override
    {
        NB_OVERRIDE(get_edges, state);
    }

    std::vector<std::tuple<nb::ref<Walker>, State, State>> get_transitions(
        nb::ref<Walker> walker, std::optional<State> state = std::nullopt) const override
    {
        NB_OVERRIDE(get_transitions, walker, state);
    }

    std::vector<nb::ref<Walker>> advance(nb::ref<Walker> walker, const std::string &token) const override
    {
        NB_OVERRIDE(advance, walker, token);
    }

    std::vector<nb::ref<Walker>> branch_walker(nb::ref<Walker> walker, std::optional<std::string> token = std::nullopt) override
    {
        NB_OVERRIDE(branch_walker, walker, token);
    }

    bool operator==(const StateMachine &other) const override
    {
        NB_OVERRIDE_NAME("__eq__", operator==, other);
    }

    std::string to_string() const override
    {
        NB_OVERRIDE_NAME("__repr__", to_string);
    }
};
