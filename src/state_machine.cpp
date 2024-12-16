#include "state_machine.h"
#include "walker.h"
#include "accepted_state.h"
#include <algorithm>
#include <deque>

namespace nb = nanobind;

using Edge = StateMachine::Edge;
using State = StateMachine::State;
using StateGraph = StateMachine::StateGraph;

StateMachine::StateMachine(
    StateGraph &&state_graph,
    State start_state,
    std::vector<State> &&end_states,
    bool is_optional,
    bool is_case_sensitive)
    : state_graph_(std::move(state_graph)),
      start_state_(std::move(start_state)),
      end_states_(std::move(end_states)),
      is_optional_(is_optional),
      is_case_sensitive_(is_case_sensitive) {}

nb::ref<Walker> StateMachine::get_new_walker(std::optional<State> state)
{
    auto w = new Walker(nb::ref<StateMachine>(this), state);
    return nb::ref<Walker>(w);
}

std::vector<nb::ref<Walker>> StateMachine::get_walkers(std::optional<State> state)
{
    auto initial_walker = get_new_walker(state.value_or(start_state_));
    if (!state_graph_.empty())
    {
        return branch_walker(initial_walker);
    }
    return {initial_walker};
}

std::vector<Edge> StateMachine::get_edges(State state) const
{
    auto it = state_graph_.find(state);
    if (it == state_graph_.end())
    {
        return {};
    }
    return it->second;
}

std::vector<std::tuple<nb::ref<Walker>, State, State>>
StateMachine::get_transitions(nb::ref<Walker> walker, std::optional<State> state) const
{
    std::vector<std::tuple<nb::ref<Walker>, State, State>> result;

    State current_state = state.value_or(walker->current_state_);

    for (auto &[edge, target_state] : get_edges(current_state))
    {
        auto transition_walkers = edge->get_walkers();
        for (const auto &transition : transition_walkers)
        {
            result.emplace_back(transition, current_state, target_state);
        }

        if (edge->is_optional_ &&
            std::find(end_states_.begin(), end_states_.end(), target_state) == end_states_.end() &&
            walker->can_accept_more_input())
        {

            auto next_transitions = get_transitions(walker, target_state);
            result.insert(result.end(), next_transitions.begin(), next_transitions.end());
        }
    }

    return result;
}

std::vector<nb::ref<Walker>> StateMachine::branch_walker(nb::ref<Walker> walker, std::optional<std::string> token)
{
    std::vector<nb::ref<Walker>> result;
    std::optional<std::string> input_token = token.has_value() ? token : walker->remaining_input_;

    auto transitions = get_transitions(walker);
    for (const auto &[transition, start_state, target_state] : transitions)
    {
        auto branched_walker = walker->start_transition(transition, input_token, start_state, target_state);

        if (branched_walker)
        {
            result.push_back(*branched_walker);
            continue;
        }

        if (transition->state_machine_->is_optional() &&
            std::find(end_states_.begin(), end_states_.end(), target_state) != end_states_.end() &&
            input_token.has_value())
        {

            if (!walker->remaining_input_)
            {
                walker->remaining_input_ = token;
            }

            auto accepted = new AcceptedState(walker);
            result.push_back(accepted);
        }
    }

    return result;
}

std::vector<nb::ref<Walker>> StateMachine::advance(nb::ref<Walker> walker, const std::string &token) const
{
    std::vector<nb::ref<Walker>> result;
    std::deque<std::pair<nb::ref<Walker>, std::string>> queue;
    queue.push_back({walker, token});

    auto handle_blocked_transition = [&](nb::ref<Walker> blocked_walker, const std::string &current_token)
    {
        std::vector<nb::ref<Walker>> branched_walkers;

        for (auto branched_walker : blocked_walker->branch(current_token))
        {
            if (branched_walker->should_start_transition(current_token))
            {
                branched_walkers.push_back(branched_walker);
            }
            else if (branched_walker->has_reached_accept_state())
            {
                result.push_back(branched_walker);
                return;
            }
        }

        for (const auto &new_walker : branched_walkers)
        {
            queue.push_back({new_walker, current_token});
        }

        if (branched_walkers.empty() && blocked_walker->remaining_input_)
        {
            result.push_back(blocked_walker);
        }
    };

    while (!queue.empty())
    {
        auto [current_walker, current_token] = queue.front();
        queue.pop_front();

        if (!current_walker->transition_walker_ ||
            !current_walker->should_start_transition(current_token))
        {
            handle_blocked_transition(current_walker, current_token);
            continue;
        }

        for (auto transition : current_walker->transition_walker_->consume_token(current_token))
        {
            auto [new_walker, is_accepted] = current_walker->complete_transition(transition);

            if (!new_walker)
            {
                continue;
            }

            if (is_accepted)
            {
                new_walker = new AcceptedState(*new_walker);
            }

            if ((*new_walker)->remaining_input_)
            {
                queue.push_back({*new_walker, *(*new_walker)->remaining_input_});
            }
            else
            {
                result.push_back(*new_walker);
            }
        }
    }

    return result;
}

std::vector<std::pair<std::string, nb::ref<Walker>>> StateMachine::advance_all(
    std::vector<nb::ref<Walker>> &walkers,
    const std::string &token,
    const tsl::htrie_set<char> &vocab)
{

    std::vector<std::pair<std::string, nb::ref<Walker>>> results;

    for (auto &walker : walkers)
    {
        auto advanced_walkers = walker->consume_token(token);

        for (auto &advanced_walker : advanced_walkers)
        {
            if (!advanced_walker->remaining_input_)
            {
                results.emplace_back(token, advanced_walker);
                continue;
            }

            size_t prefix_len = token.size() - advanced_walker->remaining_input_->size();
            std::string prefix = token.substr(0, prefix_len);

            if (!prefix.empty() && vocab.find(prefix) != vocab.end())
            {
                advanced_walker->remaining_input_ = prefix;

                if (!advanced_walker->transition_walker_ &&
                    advanced_walker->can_accept_more_input())
                {
                    auto next_walkers = advanced_walker->branch();
                    for (const auto &next_walker : next_walkers)
                    {
                        results.emplace_back(prefix, next_walker);
                    }
                }
                else
                {
                    results.emplace_back(prefix, advanced_walker);
                }
            }
        }
    }

    return results;
}

std::vector<std::pair<std::string, nb::ref<Walker>>> StateMachine::advance_all(
    std::vector<nb::ref<Walker>> &walkers,
    const std::string &token)
{

    std::vector<std::pair<std::string, nb::ref<Walker>>> results;

    for (auto &walker : walkers)
    {
        auto advanced_walkers = walker->consume_token(token);

        for (auto &advanced_walker : advanced_walkers)
        {
            if (!advanced_walker->remaining_input_)
            {
                results.emplace_back(token, advanced_walker);
                continue;
            }
        }
    }

    return results;
}

bool StateMachine::operator==(const StateMachine &other) const
{
    return state_graph_ == other.state_graph_;
}

std::string StateMachine::to_string() const
{
    std::string result = "StateMachine";

    if (state_graph_.empty())
    {
        return result + "(empty)";
    }

    result += "(graph={\n";
    for (const auto &[state, transitions] : state_graph_)
    {
        result += "    " + std::visit([](auto &&arg) -> std::string
                                      {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int>) {
                return std::to_string(arg);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return "'" + arg + "'";
            }
            return ""; }, state) +
                  ": [";

        bool first = true;
        for (const auto &[state_machine, target_state] : transitions)
        {
            if (!first)
            {
                result += ", ";
            }
            first = false;

            result += "(" + state_machine->to_string() + ", " +
                      std::visit([](auto &&arg) -> std::string
                                 {
                          using T = std::decay_t<decltype(arg)>;
                          if constexpr (std::is_same_v<T, int>) {
                              return std::to_string(arg);
                          } else if constexpr (std::is_same_v<T, std::string>) {
                              return "'" + arg + "'";
                          }
                          return ""; }, target_state) +
                      ")";
        }
        result += "],\n";
    }
    result += "})";
    return result;
}
