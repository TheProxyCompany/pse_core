#pragma once

#include "walker.h"
#include <memory>
#include <string>
#include <vector>

class AcceptedState : public Walker
{
public:
    // Constructor taking a walker that has reached an accepted state
    AcceptedState(Walker walker);

    // Override clone to return a clone of the accepted walker
    Walker clone() const override;

    // Override can_accept_more_input to delegate to accepted walker
    bool can_accept_more_input() const override;

    // Override has_reached_accept_state to always return true
    bool has_reached_accept_state() const override;

    // Override is_within_value to always return false
    bool is_within_value() const override;

    // Override should_start_transition
    bool should_start_transition(const std::string& token) override;

    // Override consume_token to delegate to accepted walker
    std::vector<Walker> consume_token(const std::string& token) const override;

    // Override equality operator
    bool operator==(const Walker &other) const override;

    // Override string representation
    std::string __repr__() const override;

    Walker accepted_walker_;
};
