#include "walker.h"
#include "acceptor.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <numeric>
#include <sstream>
#include <typeinfo>

// Constructor
Walker::Walker(std::shared_ptr<Acceptor> acceptor,
               std::optional<State> current_state)
    : acceptor_(std::move(acceptor)),
      current_state_(current_state.value_or(acceptor_->start_state())),
      consumed_character_count_(0),
      _accepts_more_input_(false)
{
  // Initialize other optional members
  target_state_ = std::nullopt;
  transition_walker_ = nullptr;
  remaining_input_ = std::nullopt;
  _raw_value_ = std::nullopt;
}

// Property-like getters
std::any Walker::current_value() const
{
  auto raw_val = raw_value();
  if (raw_val)
  {
    return parse_value(raw_val);
  }
  return {};
}

std::optional<std::string> Walker::raw_value() const
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
    auto val = walker->raw_value();
    if (val)
    {
      oss << *val;
    }
  }
  if (transition_walker_)
  {
    auto val = transition_walker_->raw_value();
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
  return std::make_tuple(current_state_, target_state_, raw_value());
}

// // Clone method
// std::shared_ptr<Walker> Walker::clone() const {
//   auto cloned_walker = std::make_shared<Walker>(*this);
//   // Deep copy the accepted_history_ and explored_edges_
//   cloned_walker->accepted_history_ = accepted_history_;
//   cloned_walker->explored_edges_ = explored_edges_;
//   return cloned_walker;
// }

// Abstract methods (to be implemented by subclasses)
// consume_token, can_accept_more_input, is_within_value are pure virtual and
// remain unimplemented here

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

bool Walker::accepts_any_token() const { return false; }

std::vector<std::string> Walker::get_valid_continuations(int depth) const
{
  if (!transition_walker_ || depth > 10)
  {
    return {};
  }
  return transition_walker_->get_valid_continuations(depth + 1);
}

bool Walker::has_reached_accept_state() const { return false; }

// Start transition
std::shared_ptr<Walker>
Walker::start_transition(std::shared_ptr<Walker> transition_walker,
                         const std::optional<std::string> &token,
                         std::optional<State> start_state,
                         std::optional<State> target_state)
{
  if (token && !transition_walker->should_start_transition(*token))
  {
    return nullptr;
  }

  if (target_state_ == target_state && transition_walker_ &&
      transition_walker_->can_accept_more_input())
  {
    return nullptr;
  }

  auto clone = this->clone();
  clone->current_state_ = start_state.value_or(clone->current_state_);
  clone->target_state_ = target_state;

  if (clone->transition_walker_ &&
      clone->transition_walker_->has_reached_accept_state())
  {
    clone->accepted_history_.push_back(clone->transition_walker_);
  }

  clone->transition_walker_ = std::move(transition_walker);
  return clone;
}

// Complete transition
std::tuple<std::shared_ptr<Walker>, bool>
Walker::complete_transition(std::shared_ptr<Walker> transition_walker)
{
  auto clone = this->clone();
  clone->transition_walker_ = std::move(transition_walker);

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
      return std::make_tuple(nullptr, false);
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

    if (std::find(clone->acceptor_->end_states().begin(),
                  clone->acceptor_->end_states().end(),
                  clone->current_state_) != clone->acceptor_->end_states().end())
    {
      return std::make_tuple(clone, true);
    }
  }

  return std::make_tuple(clone, false);
}

// Branch method
std::vector<std::shared_ptr<Walker>>
Walker::branch(const std::optional<std::string> &token)
{
  std::vector<std::shared_ptr<Walker>> result;

  if (transition_walker_)
  {
    std::vector<std::shared_ptr<Walker>> transition_walkers;
    if (transition_walker_->can_accept_more_input())
    {
      transition_walkers = transition_walker_->branch(token);
    }

    for (const auto &new_transition_walker : transition_walkers)
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

  // Extend result with acceptor's branch_walker output
  auto branched_walkers = acceptor_->branch_walker(shared_from_this(), token);
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
std::any Walker::parse_value(const std::optional<std::string> &value) const
{
  if (!value)
  {
    return {};
  }

  const std::string &val = *value;

  // Try to parse as float
  try
  {
    double float_value = std::stod(val);
    if (std::floor(float_value) == float_value)
    {
      return static_cast<int64_t>(float_value);
    }
    return float_value;
  }
  catch (const std::exception &)
  {
    // Not a float
  }

  // Try to parse as JSON (requires JSON library)
  // Here we'll assume JSON parsing is implemented elsewhere
  // For illustration purposes, we'll skip it

  // Return the original string
  return val;
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
  if (raw_value() != other.raw_value())
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

  // Compare acceptor
  if (acceptor_ != other.acceptor_)
  {
    return false;
  }

  return true;
}

// String representation
std::string Walker::to_string() const
{
  if (transition_walker_)
  {
    return acceptor_->to_string() + ".Walker(" +
           transition_walker_->to_string() + ")";
  }
  return repr();
}

// Helper method to format current edge
std::string Walker::_format_current_edge() const
{
  std::string target_state_str;
  if (target_state_)
  {
    target_state_str =
        "--> (" + Acceptor::state_to_string(*target_state_) + ")";
  }

  auto accumulated_value = raw_value();
  std::string accumulated_value_str = target_state_str;
  if (accumulated_value)
  {
    accumulated_value_str = "--" + *accumulated_value + target_state_str;
  }

  return "Current edge: (" + Acceptor::state_to_string(current_state_) + ") " +
         accumulated_value_str;
}

// Representation method
std::string Walker::repr() const
{
  // Build header with status indicators
  const std::string prefix = has_reached_accept_state() ? "✅ " : "";
  const std::string suffix = _accepts_more_input_ ? " 🔄" : "";
  const std::string header = prefix +
                             (acceptor_ ? typeid(acceptor_).name() : "null") +
                             ".Walker" + suffix;

  std::vector<std::string> info_parts;

  // Format state info
  if (std::holds_alternative<int>(current_state_) &&
      std::get<int>(current_state_) != 0)
  {
    std::string state_info =
        "State: " + Acceptor::state_to_string(current_state_);
    if (target_state_ && current_state_ != *target_state_)
    {
      state_info += " ➔ " + Acceptor::state_to_string(*target_state_);
    }
    info_parts.push_back(state_info);
  }

  // Format history info
  if (!accepted_history_.empty())
  {
    std::vector<std::string> history_values;
    for (const auto &w : accepted_history_)
    {
      const auto val = w->current_value();
      if (val.has_value())
      {
        try
        {
          const std::string str_val = std::any_cast<std::string>(val);
          history_values.push_back(str_val);
        }
        catch (const std::bad_any_cast &)
        {
          // Handle other types if needed
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

  // Add current edge
  info_parts.push_back(_format_current_edge());

  // Format remaining input
  if (remaining_input_)
  {
    info_parts.push_back("Remaining input: " + *remaining_input_);
  }

  // Format transition info
  if (transition_walker_)
  {
    std::string transition_repr = transition_walker_->repr();
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
      info_parts.push_back("Transition:\n" + indented_transition);
    }
  }

  // Build the final output
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

  // Multiline output
  std::ostringstream oss;
  oss << header << " {\n";
  for (const auto &part : info_parts)
  {
    oss << "  " << part << "\n";
  }
  oss << "}";
  return oss.str();
}
