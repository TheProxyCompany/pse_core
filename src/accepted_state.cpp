#include "accepted_state.h"

AcceptedState::AcceptedState(Walker walker)
    : Walker(walker.state_machine(), walker.current_state()),
      accepted_walker_(walker)
{
    accepted_history_ = walker.accepted_history();
    explored_edges_ = walker.explored_edges();
    current_state_ = walker.current_state();
    target_state_ = walker.target_state();
    transition_walker_ = walker.transition_walker();
    consumed_character_count_ = walker.consumed_character_count();
    remaining_input_ = walker.remaining_input();
    _raw_value_ = walker.get_raw_value();
}

Walker AcceptedState::clone() const
{
    return accepted_walker_.clone();
}

bool AcceptedState::can_accept_more_input() const
{
    return accepted_walker_.can_accept_more_input();
}

bool AcceptedState::has_reached_accept_state() const
{
    return true;
}

bool AcceptedState::is_within_value() const
{
    return false;
}

bool AcceptedState::should_start_transition(const std::string &token)
{
    if (!can_accept_more_input())
    {
        return false;
    }
    return accepted_walker_.should_start_transition(token);
}

std::vector<Walker> AcceptedState::consume_token(const std::string &token) const
{
    if (!can_accept_more_input())
    {
        return {};
    }
    return accepted_walker_.consume_token(token);
}

bool AcceptedState::operator==(const Walker &other) const
{
    return accepted_walker_ == other;
}

std::string AcceptedState::__repr__() const
{
    return "✅ " + accepted_walker_.__repr__();
}
