#include "accepted_state.h"

namespace nb = nanobind;

extern "C"
{
    int acceptedstate_tp_traverse(PyObject *self, visitproc visit, void *arg)
    {
        AcceptedState *as = nb::inst_ptr<AcceptedState>(self);
        Py_VISIT(as->accepted_walker_.get());
        Walker *w = (Walker *)as;
        Py_VISIT(w->state_machine_.get());
        for (auto &aw : w->accepted_history_)
        {
            Py_VISIT(aw.get());
        }
        if (w->transition_walker_)
        {
            Py_VISIT(w->transition_walker_.get());
        }

        return 0;
    }

    int acceptedstate_tp_clear(PyObject *self)
    {
        AcceptedState *as = nb::inst_ptr<AcceptedState>(self);
        as->accepted_walker_ = nullptr;

        // also clear walker references:
        Walker *w = (Walker *)as;
        w->state_machine_ = nullptr;
        w->accepted_history_.clear();
        w->transition_walker_ = nullptr;

        return 0;
    }
}

AcceptedState::AcceptedState(nb::ref<Walker> walker)
    : Walker(walker->state_machine(), walker->current_state())
{
    accepted_walker_ = walker;
    accepted_history_ = walker->accepted_history();
    explored_edges_ = walker->explored_edges();
    current_state_ = walker->current_state();
    target_state_ = walker->target_state();
    transition_walker_ = walker->transition_walker();
    consumed_character_count_ = walker->consumed_character_count();
    remaining_input_ = walker->remaining_input();
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

std::vector<nb::ref<Walker>> AcceptedState::consume_token(const std::string &token) const
{
    if (!can_accept_more_input())
    {
        return {};
    }
    return accepted_walker_->consume_token(token);
}

bool AcceptedState::operator==(const Walker &other) const
{
    return *accepted_walker_ == other;
}

std::string AcceptedState::__repr__() const
{
    return "✅ " + accepted_walker_->__repr__();
}
