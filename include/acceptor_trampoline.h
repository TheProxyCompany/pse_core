#pragma once
#include <nanobind/trampoline.h>
#include "acceptor.h"

class PyAcceptor : public Acceptor
{
    NB_TRAMPOLINE(Acceptor, 8); // 8 virtual methods that can be overridden

    std::type_info const &walker_class() const override
    {
        NB_OVERRIDE_PURE(walker_class);
    }

    std::vector<std::shared_ptr<Walker>> get_walkers(
        std::optional<State> state = std::nullopt) override
    {
        NB_OVERRIDE_PURE(get_walkers, state);
    }

    std::vector<std::pair<std::shared_ptr<Walker>, State>> get_transitions(
        std::shared_ptr<Walker> walker) override
    {
        NB_OVERRIDE_PURE(get_transitions, walker);
    }

    std::vector<std::shared_ptr<Walker>> advance(
        std::shared_ptr<Walker> walker,
        const std::string &token) override
    {
        NB_OVERRIDE_PURE(advance, walker, token);
    }

    std::vector<std::shared_ptr<Walker>> branch_walker(
        std::shared_ptr<Walker> walker,
        std::optional<std::string> token = std::nullopt) override
    {
        NB_OVERRIDE_PURE(branch_walker, walker, token);
    }

    bool operator==(const Acceptor &other) const override
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
