#pragma once

#include "state_machine.h"
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tsl/htrie_set.h>
#include <tuple>
#include <vector>
#include <any>

class Walker : public std::enable_shared_from_this<Walker>
{
public:
    using State = StateMachine::State;
    using VisitedEdge = std::tuple<State, std::optional<State>, std::optional<std::string>>;

    std::shared_ptr<StateMachine> state_machine_;
    std::vector<std::shared_ptr<Walker>> accepted_history_;
    std::set<VisitedEdge> explored_edges_;
    State current_state_;
    std::optional<State> target_state_;
    std::shared_ptr<Walker> transition_walker_;
    size_t consumed_character_count_;
    std::optional<std::string> remaining_input_;
    std::optional<std::string> _raw_value_;
    bool _accepts_more_input_;

    Walker(std::shared_ptr<StateMachine> state_machine, std::optional<State> current_state = std::nullopt);
    virtual ~Walker() = default;

    // Pure virtual methods
    virtual std::shared_ptr<Walker> clone() const = 0;

    // Core token handling
    virtual std::vector<std::shared_ptr<Walker>> consume_token(const std::string &token);
    virtual bool can_accept_more_input() const;
    virtual bool is_within_value() const;

    // Transition control
    virtual bool should_start_transition(const std::string &token);
    virtual bool should_complete_transition() const;
    virtual bool has_reached_accept_state() const;

    // Token validation and suggestions
    virtual bool accepts_any_token() const;
    virtual std::vector<std::string> get_valid_continuations(int depth = 0) const;
    virtual std::set<std::string> find_valid_prefixes(const tsl::htrie_set<char> &trie);

    // Value parsing
    virtual std::any parse_value(const std::optional<std::string> &value) const;

    // Non-virtual methods
    std::shared_ptr<Walker> start_transition(
        std::shared_ptr<Walker> transition_walker,
        const std::optional<std::string> &token = std::nullopt,
        std::optional<State> start_state = std::nullopt,
        std::optional<State> target_state = std::nullopt);

    std::tuple<std::shared_ptr<Walker>, bool> complete_transition(
        std::shared_ptr<Walker> transition_walker);

    std::vector<std::shared_ptr<Walker>> branch(
        const std::optional<std::string> &token = std::nullopt);

    // Property-like getters
    std::any current_value() const;
    std::optional<std::string> raw_value() const;
    VisitedEdge current_edge() const;

    // Comparison operators
    bool operator==(const Walker &other) const;

    // String representation
    virtual std::string to_string() const;
    virtual std::string repr() const;

    const std::shared_ptr<StateMachine> &state_machine() const { return state_machine_; }
    void state_machine(const std::shared_ptr<StateMachine> &value) { state_machine_ = value; }

    const std::vector<std::shared_ptr<Walker>> &accepted_history() const { return accepted_history_; }
    void accepted_history(const std::vector<std::shared_ptr<Walker>> &value) { accepted_history_ = value; }

    const std::set<VisitedEdge> &explored_edges() const { return explored_edges_; }
    void explored_edges(const std::set<VisitedEdge> &value) { explored_edges_ = value; }

    const State &current_state() const { return current_state_; }
    void current_state(const State &value) { current_state_ = value; }

    const std::optional<State> &target_state() const { return target_state_; }
    void target_state(const std::optional<State> &value) { target_state_ = value; }

    const std::shared_ptr<Walker> &transition_walker() const { return transition_walker_; }
    void transition_walker(const std::shared_ptr<Walker> &value) { transition_walker_ = value; }

    size_t consumed_character_count() const { return consumed_character_count_; }
    void consumed_character_count(size_t value) { consumed_character_count_ = value; }

    const std::optional<std::string> &remaining_input() const { return remaining_input_; }
    void remaining_input(const std::optional<std::string> &value) { remaining_input_ = value; }

private:
    std::string _format_current_edge() const;
};
