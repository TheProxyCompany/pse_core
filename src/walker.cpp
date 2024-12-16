#include "walker.h"
#include "state_machine.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>
#include <typeinfo>
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/intrusive/counter.h>
#include <nanobind/intrusive/ref.h>

namespace nb = nanobind;

// Constructor
Walker::Walker(nb::ref<StateMachine> state_machine,
               std::optional<State> current_state)
    : state_machine_(state_machine),
      current_state_(current_state.value_or(state_machine->start_state_)),
      consumed_character_count_(0),
      _accepts_more_input_(false)
{
  target_state_ = std::nullopt;
  transition_walker_ = nullptr;
  remaining_input_ = std::nullopt;
  _raw_value_ = std::nullopt;
}

nb::ref<Walker> Walker::clone() const
{
  return new Walker(*this);
}

// Property-like getters
nb::object Walker::get_current_value() const
{
  auto raw_val = get_raw_value();
  if (raw_val)
  {
    return parse_value(raw_val);
  }
  return nb::none();
}

std::optional<std::string> Walker::get_raw_value() const
{
  if (_raw_value_)
  {
    return _raw_value_;
  }

  if (accepted_history_.empty() && !transition_walker_)
  {
    return std::nullopt;
  }

  std::ostringstream oss;
  for (const auto &walker : accepted_history_)
  {
    auto val = walker->get_raw_value();
    if (val)
    {
      oss << *val;
    }
  }
  if (transition_walker_)
  {
    auto val = transition_walker_->get_raw_value();
    if (val)
    {
      oss << *val;
    }
  }

  std::string result = oss.str();
  if (!result.empty())
  {
    return result;
  }
  return std::nullopt;
}

Walker::VisitedEdge Walker::current_edge() const
{
  return std::make_tuple(current_state_, target_state_, get_raw_value());
}

std::vector<nb::ref<Walker>> Walker::consume_token(const std::string &token)
{
  return state_machine_->advance(nb::ref<Walker>(this), token);
}

bool Walker::can_accept_more_input() const
{
  if (transition_walker_ && transition_walker_->can_accept_more_input())
  {
    return true;
  }
  auto it = state_machine_->state_graph_.find(current_state_);
  if (it == state_machine_->state_graph_.end())
  {
    return _accepts_more_input_;
  }
  bool has_current_edges = it->second.size() > 0;
  return _accepts_more_input_ || has_current_edges;
}

bool Walker::is_within_value() const
{
  if (transition_walker_)
  {
    return transition_walker_->is_within_value();
  }
  return consumed_character_count_ > 0;
}

// Default implementations
bool Walker::should_start_transition(const std::string &token)
{
  if (transition_walker_)
  {
    return transition_walker_->should_start_transition(token);
  }

  if (explored_edges_.count(current_edge()) > 0)
  {
    _accepts_more_input_ = false;
    return false;
  }

  return true;
}

bool Walker::should_complete_transition() const
{
  if (transition_walker_)
  {
    return transition_walker_->should_complete_transition();
  }
  return true;
}

bool Walker::accepts_any_token() const
{
  if (transition_walker_)
  {
    return transition_walker_->accepts_any_token();
  }
  return false;
}

std::vector<std::string> Walker::get_valid_continuations(int depth) const
{
  if (!transition_walker_ || depth > 10)
  {
    return {};
  }
  return transition_walker_->get_valid_continuations(depth + 1);
}

bool Walker::has_reached_accept_state() const { return false; }

std::optional<nb::ref<Walker>>
Walker::start_transition(nb::ref<Walker> transition_walker,
                         const std::optional<std::string> &token,
                         std::optional<State> start_state,
                         std::optional<State> target_state)
{
  if (token && !transition_walker->should_start_transition(*token))
  {
    return std::nullopt;
  }

  if (target_state_ == target_state && transition_walker_ &&
      transition_walker_->can_accept_more_input())
  {
    return std::nullopt;
  }

  auto clone = this->clone();
  clone->current_state_ = start_state.value_or(clone->current_state_);
  clone->target_state_ = target_state;

  if (clone->transition_walker_ &&
      clone->transition_walker_->has_reached_accept_state())
  {
    clone->accepted_history_.push_back(clone->transition_walker_);
  }

  clone->transition_walker_ = transition_walker;
  return clone;
}

// Complete transition
std::tuple<std::optional<nb::ref<Walker>>, bool>
Walker::complete_transition(nb::ref<Walker> transition_walker)
{
  auto clone = this->clone();
  clone->transition_walker_ = transition_walker;

  clone->remaining_input_ = clone->transition_walker_->remaining_input_;
  clone->transition_walker_->remaining_input_ = std::nullopt;

  clone->consumed_character_count_ +=
      clone->transition_walker_->consumed_character_count_;
  clone->explored_edges_.insert(clone->current_edge());

  if (!clone->should_complete_transition())
  {
    if (clone->can_accept_more_input())
    {
      return std::make_tuple(clone, false);
    }
    else
    {
      return std::make_tuple(std::nullopt, false);
    }
  }

  if (clone->target_state_ &&
      clone->transition_walker_->has_reached_accept_state())
  {
    clone->current_state_ = clone->target_state_.value();

    if (!clone->transition_walker_->can_accept_more_input())
    {
      clone->accepted_history_.push_back(clone->transition_walker_);
      clone->transition_walker_ = nullptr;
      clone->target_state_ = std::nullopt;
    }

    if (std::find(clone->state_machine_->end_states_.begin(),
                  clone->state_machine_->end_states_.end(),
                  clone->current_state_) != clone->state_machine_->end_states_.end())
    {
      return std::make_tuple(clone, true);
    }
  }

  return std::make_tuple(clone, false);
}

// Branch method
std::vector<nb::ref<Walker>>
Walker::branch(const std::optional<std::string> &token)
{
  std::vector<nb::ref<Walker>> result;

  if (transition_walker_)
  {
    std::vector<nb::ref<Walker>> transition_walkers;
    if (transition_walker_->can_accept_more_input())
    {
      transition_walkers = transition_walker_->branch(token);
    }

    for (auto &new_transition_walker : transition_walkers)
    {
      auto clone = this->clone();
      clone->transition_walker_ = new_transition_walker;
      result.push_back(clone);
    }

    if (transition_walkers.empty() &&
        !transition_walker_->has_reached_accept_state())
    {
      return result;
    }
  }

  auto branched_walkers = state_machine_->branch_walker(nb::ref<Walker>(this), token);
  result.insert(result.end(), branched_walkers.begin(), branched_walkers.end());
  return result;
}

// Find valid prefixes
std::set<std::string>
Walker::find_valid_prefixes(const tsl::htrie_set<char> &trie)
{
  std::set<std::string> valid_prefixes;
  std::set<std::string> seen;

  for (const auto &continuation : get_valid_continuations())
  {
    if (seen.count(continuation) > 0)
    {
      continue;
    }
    seen.insert(continuation);

    // Use trie to find tokens with the given prefix
    auto range = trie.equal_prefix_range(continuation);
    for (auto it = range.first; it != range.second; ++it)
    {
      valid_prefixes.insert(it.key());
    }
  }

  return valid_prefixes;
}

// Helper function to parse value
nb::object Walker::parse_value(const std::optional<std::string> &value) const
{
  if (!value)
  {
    return nb::none();
  }

  const std::string &val = *value;

  // Try to parse as float
  try
  {
    double float_value = std::stod(val);
    return nb::cast(float_value);
  }
  catch (const std::invalid_argument &)
  {
    // Not a float
  }

  return nb::cast(val);
}

// Comparison operator
bool Walker::operator==(const Walker &other) const
{
  // Compare current state
  if (current_state_ != other.current_state_)
  {
    return false;
  }

  // Compare target state
  if (target_state_ != other.target_state_)
  {
    return false;
  }

  // Compare raw value
  if (get_raw_value() != other.get_raw_value())
  {
    return false;
  }

  // Compare transition walker
  if ((transition_walker_ && !other.transition_walker_) ||
      (!transition_walker_ && other.transition_walker_))
  {
    return false;
  }
  if (transition_walker_ && other.transition_walker_ &&
      !(*transition_walker_ == *other.transition_walker_))
  {
    return false;
  }

  // Compare state_machine
  if (state_machine_ != other.state_machine_)
  {
    return false;
  }

  return true;
}

// Helper method to format current edge
std::string Walker::_format_current_edge() const
{
  std::string target_state_str;
  if (target_state_)
  {
    target_state_str =
        "--> (" + StateMachine::state_to_string(*target_state_) + ")";
  }

  auto accumulated_value = get_raw_value();
  std::string accumulated_value_str = target_state_str;
  if (accumulated_value)
  {
    accumulated_value_str = "--" + *accumulated_value + target_state_str;
  }

  return "Current edge: (" + StateMachine::state_to_string(current_state_) + ") " +
         accumulated_value_str;
}

// Representation method
std::string Walker::to_string() const
{
  const std::string prefix = has_reached_accept_state() ? "✅ " : "";
  const std::string suffix = _accepts_more_input_ ? " 🔄" : "";
  const std::string header = prefix +
                             state_machine_->get_name() + ".Walker" +
                             suffix;

  std::vector<std::string> info_parts;

  if (std::holds_alternative<int>(current_state_) &&
      std::get<int>(current_state_) != 0)
  {
    std::string state_info =
        "State: " + StateMachine::state_to_string(current_state_);
    if (target_state_ && current_state_ != *target_state_)
    {
      state_info += " ➔ " + StateMachine::state_to_string(*target_state_);
    }
    info_parts.push_back(state_info);
  }

  if (!accepted_history_.empty())
  {
    std::vector<std::string> history_values;
    for (auto &w : accepted_history_)
    {
      const auto val = w->get_current_value();
      if (val)
      {
        try
        {
          std::string str_val = nb::cast<std::string>(val);
          history_values.push_back(str_val);
        }
        catch (...)
        {
          // skip non-string
        }
      }
    }
    if (!history_values.empty())
    {
      std::ostringstream oss;
      for (size_t i = 0; i < history_values.size(); ++i)
      {
        if (i > 0)
          oss << ", ";
        oss << history_values[i];
      }
      info_parts.push_back("History: " + oss.str());
    }
  }

  info_parts.push_back(_format_current_edge());

  if (remaining_input_)
  {
    info_parts.push_back("Remaining input: " + *remaining_input_);
  }

  if (transition_walker_)
  {
    std::string transition_repr = transition_walker_->to_string();
    if (transition_repr.find('\n') == std::string::npos &&
        transition_repr.length() < 40)
    {
      info_parts.push_back("Transition: " + transition_repr);
    }
    else
    {
      std::string indented_transition = "  " + transition_repr;
      std::string indent = "  ";
      for (size_t pos = 0;
           (pos = indented_transition.find('\n', pos)) != std::string::npos;
           pos += indent.length() + 1)
      {
        indented_transition.replace(pos, 1, "\n" + indent);
      }
      info_parts.push_back("Transition:\n" + indent + indented_transition);
    }
  }

  std::string single_line =
      header + " (" +
      std::accumulate(info_parts.begin(), info_parts.end(), std::string(),
                      [](const std::string &a, const std::string &b)
                      {
                        return a.empty() ? b : a + ", " + b;
                      }) +
      ")";
  if (single_line.length() <= 80)
  {
    return single_line;
  }

  std::ostringstream oss;
  oss << header << " {\n";
  for (auto &part : info_parts)
  {
    oss << "  " << part << "\n";
  }
  oss << "}";
  return oss.str();
}
