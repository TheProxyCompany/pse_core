#pragma once
#include "walker.h"
#include <nanobind/trampoline.h>

class PyWalker : public Walker {
public:
    // NB_TRAMPOLINE macro defines the interface
    NB_TRAMPOLINE(Walker, 4);

    // Pure virtual methods
    std::vector<std::shared_ptr<Walker>> consume_token(const std::string& token) override {
        NB_OVERRIDE_PURE(consume_token, token);
    }

    std::shared_ptr<Walker> clone() const override {
        NB_OVERRIDE_PURE(clone);
    }

    bool can_accept_more_input() const override {
        NB_OVERRIDE_PURE(can_accept_more_input);
    }

    bool is_within_value() const override {
        NB_OVERRIDE_PURE(is_within_value);
    }

    // Virtual methods with default implementations
    bool should_start_transition(const std::string& token) override {
        NB_OVERRIDE(should_start_transition, token);
    }

    bool should_complete_transition() const override {
        NB_OVERRIDE(should_complete_transition);
    }

    bool accepts_any_token() const override {
        NB_OVERRIDE(accepts_any_token);
    }

    std::vector<std::string> get_valid_continuations(int depth) const override {
        NB_OVERRIDE(get_valid_continuations, depth);
    }

    bool has_reached_accept_state() const override {
        NB_OVERRIDE(has_reached_accept_state);
    }
};
