#pragma once

#include "walker_forward_header.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class StateMachine
{
public:
  using State = std::variant<int, std::string>;
  using Edge = std::pair<std::shared_ptr<StateMachine>, State>;
  using VisitedEdge = std::tuple<State, std::optional<State>, std::optional<std::string>>;
  using StateGraph = std::unordered_map<State, std::vector<Edge>>;

  StateGraph state_graph_;
  State start_state_;
  std::vector<State> end_states_;
  bool is_optional_;
  bool is_case_sensitive_;

  StateMachine(StateGraph &&state_graph = StateGraph(),
               State start_state = 0, std::vector<State> &&end_states = {"$"},
               bool is_optional = false, bool is_case_sensitive = true);

  virtual ~StateMachine() = default;

  const State &start_state() const { return start_state_; }
  void start_state(const State &value) { start_state_ = value; }

  const std::vector<State> &end_states() const { return end_states_; }
  void end_states(const std::vector<State> &value) { end_states_ = value; }

  const StateGraph &state_graph() const { return state_graph_; }
  void state_graph(const StateGraph &value) { state_graph_ = value; }

  bool is_optional() const { return is_optional_; }
  void is_optional(bool value) { is_optional_ = value; }

  bool is_case_sensitive() const { return is_case_sensitive_; }
  void is_case_sensitive(bool value) { is_case_sensitive_ = value; }

  virtual std::shared_ptr<Walker> get_new_walker(std::optional<State> state = std::nullopt) = 0;
  virtual std::vector<std::shared_ptr<Walker>> get_walkers(std::optional<State> state = std::nullopt) = 0;
  virtual std::vector<Edge> get_edges(State state) = 0;
  virtual std::vector<std::pair<std::shared_ptr<Walker>, State>> get_transitions(std::shared_ptr<Walker> walker) = 0;
  virtual std::vector<std::shared_ptr<Walker>> advance(std::shared_ptr<Walker> walker, const std::string &token) = 0;
  virtual std::vector<std::shared_ptr<Walker>> branch_walker(std::shared_ptr<Walker> walker, std::optional<std::string> token = std::nullopt) = 0;

  virtual bool operator==(const StateMachine &other) const;
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
                        } }, state);
  }
};
