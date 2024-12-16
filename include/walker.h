#pragma once

#include "state_machine.h"
#include <nanobind/nanobind.h>
#include <nanobind/intrusive/counter.h>
#include <nanobind/intrusive/ref.h>
#include <optional>
#include <set>
#include <string>
#include <tsl/htrie_set.h>
#include <tuple>
#include <vector>
#include <any>

namespace nb = nanobind;

class Walker : public nb::intrusive_base
{
public:
    using State = StateMachine::State;
    using VisitedEdge = std::tuple<State, std::optional<State>, std::optional<std::string>>;

    nb::ref<StateMachine> state_machine_;
    std::vector<nb::ref<Walker>> accepted_history_;
    std::set<VisitedEdge> explored_edges_;
    State current_state_;
    std::optional<State> target_state_;
    nb::ref<Walker> transition_walker_;
    size_t consumed_character_count_;
    std::optional<std::string> remaining_input_;
    std::optional<std::string> _raw_value_;
    bool _accepts_more_input_;

    Walker(nb::ref<StateMachine> state_machine, std::optional<State> current_state = std::nullopt);
    virtual ~Walker() = default;

    virtual std::vector<nb::ref<Walker>> consume_token(const std::string &token);
    virtual bool can_accept_more_input() const;
    virtual bool is_within_value() const;

    virtual bool should_start_transition(const std::string &token);
    virtual bool should_complete_transition() const;
    virtual bool has_reached_accept_state() const;

    virtual bool accepts_any_token() const;
    virtual std::vector<std::string> get_valid_continuations(int depth = 0) const;
    virtual std::set<std::string> find_valid_prefixes(const tsl::htrie_set<char> &trie);

    virtual nb::object parse_value(const std::optional<std::string> &value) const;

    virtual nb::ref<Walker> clone() const;

    std::optional<nb::ref<Walker>> start_transition(
        nb::ref<Walker> transition_walker,
        const std::optional<std::string> &token = std::nullopt,
        std::optional<State> start_state = std::nullopt,
        std::optional<State> target_state = std::nullopt);

    std::tuple<std::optional<nb::ref<Walker>>, bool> complete_transition(nb::ref<Walker> transition_walker);

    std::vector<nb::ref<Walker>> branch(const std::optional<std::string> &token = std::nullopt);

    VisitedEdge current_edge() const;

    virtual nb::object get_current_value() const;
    virtual std::optional<std::string> get_raw_value() const;

    virtual bool operator==(const Walker &other) const;

    virtual std::string to_string() const;

private:
    std::string _format_current_edge() const;
};
