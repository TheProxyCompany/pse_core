#pragma once
#include <nanobind/trampoline.h>
#include "state_machine.h"

class PyStateMachine : public StateMachine
{
    NB_TRAMPOLINE(StateMachine, 7);

    std::shared_ptr<Walker> get_new_walker(std::optional<State> state = std::nullopt) override
    {
        NB_OVERRIDE(get_new_walker, state);
    }

    std::vector<std::shared_ptr<Walker>> get_walkers(
        std::optional<State> state = std::nullopt) override
    {
        NB_OVERRIDE(get_walkers, state);
    }

    std::vector<Edge> get_edges(State state) override
    {
        NB_OVERRIDE(get_edges, state);
    }

    std::vector<std::tuple<std::shared_ptr<Walker>, State, State>> get_transitions(
        std::shared_ptr<Walker> walker,
        std::optional<State> state = std::nullopt) override
    {
        NB_OVERRIDE(get_transitions, walker, state);
    }

    std::vector<std::shared_ptr<Walker>> advance(
        std::shared_ptr<Walker> walker,
        const std::string &token) override
    {
        NB_OVERRIDE(advance, walker, token);
    }

    std::vector<std::shared_ptr<Walker>> branch_walker(
        std::shared_ptr<Walker> walker,
        std::optional<std::string> token = std::nullopt) override
    {
        NB_OVERRIDE(branch_walker, walker, token);
    }

    bool operator==(const StateMachine &other) const override
    {
        NB_OVERRIDE(operator==, other);
    }

    std::string to_string() const override
    {
        NB_OVERRIDE(to_string);
    }

    std::string repr() const override
    {
        NB_OVERRIDE(repr);
    }
};
