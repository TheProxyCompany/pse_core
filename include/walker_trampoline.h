#pragma once
#include "walker.h"
#include <nanobind/trampoline.h>

class PyWalker : public Walker
{
public:
    // NB_TRAMPOLINE macro defines the interface
    NB_TRAMPOLINE(Walker, 15);

    // Pure virtual methods
    nb::ref<Walker> clone() const override
    {
        NB_OVERRIDE(clone);
    }

    std::vector<nb::ref<Walker>> consume_token(const std::string &token) override
    {
        NB_OVERRIDE(consume_token, token);
    }

    bool can_accept_more_input() const override
    {
        NB_OVERRIDE(can_accept_more_input);
    }

    bool is_within_value() const override
    {
        NB_OVERRIDE(is_within_value);
    }

    bool should_start_transition(const std::string &token) override
    {
        NB_OVERRIDE(should_start_transition, token);
    }

    bool should_complete_transition() const override
    {
        NB_OVERRIDE(should_complete_transition);
    }

    bool has_reached_accept_state() const override
    {
        NB_OVERRIDE(has_reached_accept_state);
    }

    bool accepts_any_token() const override
    {
        NB_OVERRIDE(accepts_any_token);
    }

    std::vector<std::string> get_valid_continuations(int depth = 0) const override
    {
        NB_OVERRIDE(get_valid_continuations, depth);
    }

    std::set<std::string> find_valid_prefixes(const tsl::htrie_set<char> &trie) override
    {
        NB_OVERRIDE(find_valid_prefixes, trie);
    }

    nb::object parse_value(const std::optional<std::string> &value) const override
    {
        NB_OVERRIDE(parse_value, value);
    }

    nb::object get_current_value() const override
    {
        NB_OVERRIDE(get_current_value);
    }

    std::optional<std::string> get_raw_value() const override
    {
        NB_OVERRIDE(get_raw_value);
    }

    std::string to_string() const override
    {
        NB_OVERRIDE_NAME("__repr__", to_string);
    }
};
