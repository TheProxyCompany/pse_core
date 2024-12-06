#pragma once

#include "walker_forward_header.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

/**
 * @brief Base class for token acceptors.
 *
 * An acceptor constrains the acceptable tokens at a specific point
 * during parsing or generation. It manages multiple walkers representing
 * different valid states, enabling efficient traversal and minimizing
 * backtracking.
 */
class Acceptor
{
public:
  using State = std::variant<int, std::string>;
  using Edge = std::pair<std::shared_ptr<Acceptor>, State>;
  using VisitedEdge =
      std::tuple<State, std::optional<State>, std::optional<std::string>>;
  using StateGraph = std::unordered_map<State, std::vector<Edge>>;

  StateGraph state_graph_;
  State start_state_;
  std::vector<State> end_states_;
  bool is_optional_;
  bool is_case_sensitive_;

  /**
   * @brief Construct a new Acceptor
   *
   * @param state_graph The state transition graph
   * @param start_state The initial state
   * @param end_states Collection of accepting states
   * @param is_optional Whether the acceptor is optional
   * @param is_case_sensitive Whether the acceptor is case sensitive
   */
  Acceptor(StateGraph&& state_graph = StateGraph(),
           State start_state = 0, std::vector<State>&& end_states = {"$"},
           bool is_optional = false, bool is_case_sensitive = true);

  virtual ~Acceptor() = default;

  // Property getters and setters
  const State& start_state() const { return start_state_; }
  void start_state(const State& value) { start_state_ = value; }

  const std::vector<State>& end_states() const { return end_states_; }
  void end_states(const std::vector<State>& value) { end_states_ = value; }

  const StateGraph& state_graph() const { return state_graph_; }
  void state_graph(const StateGraph& value) { state_graph_ = value; }

  bool is_optional() const { return is_optional_; }
  void is_optional(bool value) { is_optional_ = value; }

  bool is_case_sensitive() const { return is_case_sensitive_; }
  void is_case_sensitive(bool value) { is_case_sensitive_ = value; }

  /**
   * @brief Get the walker class for this acceptor
   * @return Walker class type
   */
  virtual const std::type_info& walker_class() const = 0;

  /**
   * @brief Get walkers to traverse the acceptor
   * @param state Optional starting state
   * @return Vector of walker instances
   */
  virtual std::vector<std::shared_ptr<Walker>>
  get_walkers(std::optional<State> state = std::nullopt) = 0;

  /**
   * @brief Get transitions from the given walker
   * @param walker Walker to get transitions from
   * @return Vector of walker and state pairs
   */
  virtual std::vector<std::pair<std::shared_ptr<Walker>, State>>
  get_transitions(std::shared_ptr<Walker> walker) = 0;

  /**
   * @brief Advance walker with given input
   * @param walker Walker to advance
   * @param token Input token to process
   * @return Vector of updated walkers
   */
  virtual std::vector<std::shared_ptr<Walker>>
  advance(std::shared_ptr<Walker> walker, const std::string &token) = 0;

  /**
   * @brief Branch walker into multiple paths
   * @param walker Walker to branch
   * @param token Optional token to consider
   * @return Vector of branched walkers
   */
  virtual std::vector<std::shared_ptr<Walker>>
  branch_walker(std::shared_ptr<Walker> walker,
                std::optional<std::string> token = std::nullopt) = 0;

  virtual bool operator==(const Acceptor &other) const;
  virtual std::string to_string() const;
  virtual std::string repr() const;

  /**
   * @brief Convert a state to a string
   * @param state The state to convert
   * @return The string representation of the state
   */
  static std::string state_to_string(const State &state)
  {
    return std::visit([](auto &&arg) -> std::string
                      {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                          return std::to_string(arg);
                        } else if constexpr (std::is_same_v<T, std::string>) {
                          return arg;
                        } else {
                          return "";
                        }
                      }, state);
  }

// protected:
//   StateGraph state_graph_;
//   State start_state_;
//   std::vector<State> end_states_;
//   bool is_optional_;
//   bool is_case_sensitive_;
};
