#pragma once

#include <tsl/htrie_set.h>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <nanobind/nanobind.h>
#include <nanobind/intrusive/counter.h>
#include <nanobind/intrusive/ref.h>

// Forward declaration
class Walker;

namespace nb = nanobind;

class StateMachine : public nb::intrusive_base
{
public:
  using State = std::variant<int, std::string>;
  using Edge = std::pair<nb::ref<StateMachine>, State>;
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

  bool is_optional() const { return is_optional_; }
  void is_optional(bool value) { is_optional_ = value; }

  bool is_case_sensitive() const { return is_case_sensitive_; }
  void is_case_sensitive(bool value) { is_case_sensitive_ = value; }

  virtual nb::ref<Walker> get_new_walker(std::optional<State> state = std::nullopt);
  virtual std::vector<nb::ref<Walker>> get_walkers(std::optional<State> state = std::nullopt);
  virtual std::vector<Edge> get_edges(State state) const;
  virtual std::vector<std::tuple<nb::ref<Walker>, State, State>> get_transitions(nb::ref<Walker> walker, std::optional<State> state = std::nullopt) const;
  virtual std::vector<nb::ref<Walker>> branch_walker(nb::ref<Walker> walker, std::optional<std::string> token = std::nullopt);
  virtual std::vector<nb::ref<Walker>> advance(nb::ref<Walker> walker, const std::string &token) const;

  virtual bool operator==(const StateMachine &other) const;
  virtual std::string to_string() const;

  std::string get_name() const
  {
    nb::object obj = nb::find(this);
    nb::object cls = nb::getattr(obj, "__class__");
    nb::object name_obj = nb::getattr(cls, "__name__");
    std::string type_name = nb::str(name_obj).c_str();
    return type_name;
  }

  /**
   * @brief Advance multiple walkers with a token, optionally using a vocabulary DAWG
   * @param walkers The walkers to advance
   * @param token The token to advance with
   * @param vocab vocabulary to validate against
   * @return Vector of pairs containing the token and resulting walker
   */
  static std::vector<std::pair<std::string, nb::ref<Walker>>> advance_all(
      std::vector<nb::ref<Walker>> &walkers,
      const std::string &token,
      const tsl::htrie_set<char> &vocab);

  /**
   * @brief Advance multiple walkers with a token, optionally using a vocabulary DAWG
   * @param walkers The walkers to advance
   * @param token The token to advance with
   * @return Vector of pairs containing the token and resulting walker
   */
  static std::vector<std::pair<std::string, nb::ref<Walker>>> advance_all(
      std::vector<nb::ref<Walker>> &walkers,
      const std::string &token);

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
