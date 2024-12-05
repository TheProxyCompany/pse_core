#pragma once

#include "acceptor.h"
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tsl/htrie_set.h>
#include <tuple>
#include <vector>
#include <any>

class Walker : public std::enable_shared_from_this<Walker> {
public:
    using State = Acceptor::State;
    using VisitedEdge = std::tuple<State, std::optional<State>, std::optional<std::string>>;

    Walker(std::shared_ptr<Acceptor> acceptor, std::optional<State> current_state = std::nullopt);
    virtual ~Walker() = default;

    // Pure virtual methods that must be implemented by subclasses
    virtual std::vector<std::shared_ptr<Walker>> consume_token(const std::string& token) = 0;
    virtual bool can_accept_more_input() const = 0;
    virtual bool is_within_value() const = 0;

    // Virtual methods with default implementations
    virtual bool should_start_transition(const std::string& token);
    virtual bool should_complete_transition() const;
    virtual bool accepts_any_token() const;
    virtual std::vector<std::string> get_valid_continuations(int depth = 0) const;
    virtual bool has_reached_accept_state() const;
    virtual std::set<std::string> find_valid_prefixes(const tsl::htrie_set<char>& trie);
    virtual std::shared_ptr<Walker> clone() const = 0;

    // Non-virtual methods
    std::shared_ptr<Walker> start_transition(
        std::shared_ptr<Walker> transition_walker,
        const std::optional<std::string>& token = std::nullopt,
        std::optional<State> start_state = std::nullopt,
        std::optional<State> target_state = std::nullopt
    );

    std::tuple<std::shared_ptr<Walker>, bool> complete_transition(
        std::shared_ptr<Walker> transition_walker
    );

    std::vector<std::shared_ptr<Walker>> branch(
        const std::optional<std::string>& token = std::nullopt
    );

    // Property-like getters
    std::any current_value() const;
    std::optional<std::string> raw_value() const;
    VisitedEdge current_edge() const;

    // Comparison operators
    bool operator==(const Walker& other) const;

    // String representation
    virtual std::string to_string() const;
    virtual std::string repr() const;

    const std::shared_ptr<Acceptor>& get_acceptor() const { return acceptor_; }
    const std::vector<std::shared_ptr<Walker>>& get_accepted_history() const { return accepted_history_; }
    const std::set<VisitedEdge>& get_explored_edges() const { return explored_edges_; }
    const State& get_current_state() const { return current_state_; }
    const std::optional<State>& get_target_state() const { return target_state_; }
    const std::shared_ptr<Walker>& get_transition_walker() const { return transition_walker_; }
    size_t get_consumed_character_count() const { return consumed_character_count_; }
    const std::optional<std::string>& get_remaining_input() const { return remaining_input_; }

protected:
    std::shared_ptr<Acceptor> acceptor_;
    std::vector<std::shared_ptr<Walker>> accepted_history_;
    std::set<VisitedEdge> explored_edges_;

    State current_state_;
    std::optional<State> target_state_;
    std::shared_ptr<Walker> transition_walker_;

    size_t consumed_character_count_;
    std::optional<std::string> remaining_input_;

    std::optional<std::string> _raw_value_;
    bool _accepts_more_input_;

private:
    std::any _parse_value(const std::optional<std::string>& value) const;
    std::string _format_current_edge() const;
};
