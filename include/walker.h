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

    StateMachine state_machine_;
    std::vector<nb::ref<Walker>> accepted_history_;
    std::set<VisitedEdge> explored_edges_;
    State current_state_;
    std::optional<State> target_state_;
    nb::ref<Walker> transition_walker_;
    size_t consumed_character_count_;
    std::optional<std::string> remaining_input_;
    std::optional<std::string> _raw_value_;
    bool _accepts_more_input_;

    Walker(StateMachine state_machine, std::optional<State> current_state = std::nullopt);
    virtual ~Walker() = default;

    virtual std::vector<Walker> consume_token(const std::string &token) const;
    virtual bool can_accept_more_input() const;
    virtual bool is_within_value() const;

    virtual bool should_start_transition(const std::string &token);
    virtual bool should_complete_transition() const;
    virtual bool has_reached_accept_state() const;

    virtual bool accepts_any_token() const;
    virtual std::vector<std::string> get_valid_continuations(int depth = 0) const;
    virtual std::set<std::string> find_valid_prefixes(const tsl::htrie_set<char> &trie);

    virtual nb::object parse_value(const std::optional<std::string> &value) const;

    virtual Walker clone() const;

    std::optional<Walker> start_transition(
        Walker transition_walker,
        const std::optional<std::string> &token = std::nullopt,
        std::optional<State> start_state = std::nullopt,
        std::optional<State> target_state = std::nullopt);

    std::tuple<std::optional<Walker>, bool> complete_transition(Walker transition_walker);

    std::vector<Walker> branch(const std::optional<std::string> &token = std::nullopt) const;

    VisitedEdge current_edge() const;

    virtual nb::object get_current_value() const;
    virtual std::optional<std::string> get_raw_value() const;

    virtual bool operator==(const Walker &other) const;

    virtual std::string to_string() const;
    virtual std::string __repr__() const;

    const StateMachine &state_machine() const { return state_machine_; }
    void state_machine(const StateMachine &value) { state_machine_ = value; }

    const std::vector<nb::ref<Walker>> &accepted_history() const { return accepted_history_; }
    void accepted_history(const std::vector<nb::ref<Walker>> &value) { accepted_history_ = value; }

    const std::set<VisitedEdge> &explored_edges() const { return explored_edges_; }
    void explored_edges(const std::set<VisitedEdge> &value) { explored_edges_ = value; }

    const State &current_state() const { return current_state_; }
    void current_state(const State &value) { current_state_ = value; }

    const std::optional<State> &target_state() const { return target_state_; }
    void target_state(const std::optional<State> &value) { target_state_ = value; }

    const nb::ref<Walker> &transition_walker() const { return transition_walker_; }
    void transition_walker(const nb::ref<Walker> &value) { transition_walker_ = value; }

    size_t consumed_character_count() const { return consumed_character_count_; }
    void consumed_character_count(size_t value) { consumed_character_count_ = value; }

    const std::optional<std::string> &remaining_input() const { return remaining_input_; }
    void remaining_input(const std::optional<std::string> &value) { remaining_input_ = value; }

private:
    std::string _format_current_edge() const;
};
