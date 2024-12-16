#include "accepted_state.h"

AcceptedState::AcceptedState(nb::ref<Walker> walker)
    : Walker(walker->state_machine_, walker->current_state_),
      accepted_walker_(walker)
{
    accepted_history_ = walker->accepted_history_;
    explored_edges_ = walker->explored_edges_;
    current_state_ = walker->current_state_;
    target_state_ = walker->target_state_;
    transition_walker_ = walker->transition_walker_;
    consumed_character_count_ = walker->consumed_character_count_;
    remaining_input_ = walker->remaining_input_;
    _raw_value_ = walker->get_raw_value();
}

nb::ref<Walker> AcceptedState::clone() const
{
    return accepted_walker_->clone();
}

bool AcceptedState::can_accept_more_input() const
{
    return accepted_walker_->can_accept_more_input();
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
    return accepted_walker_->should_start_transition(token);
}

std::vector<nb::ref<Walker>> AcceptedState::consume_token(const std::string &token)
{
    if (!can_accept_more_input())
    {
        return {};
    }
    return accepted_walker_->consume_token(token);
}

bool AcceptedState::operator==(const Walker &other) const
{
    return accepted_walker_.get() == &other;
}

std::string AcceptedState::to_string() const
{
    return "✅ " + accepted_walker_->to_string();
}
