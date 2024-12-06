#include "acceptor.h"

Acceptor::Acceptor(
    StateGraph&& state_graph,
    State start_state,
    std::vector<State>&& end_states,
    bool is_optional,
    bool is_case_sensitive)
    : state_graph_(std::move(state_graph)),
      start_state_(std::move(start_state)),
      end_states_(std::move(end_states)),
      is_optional_(is_optional),
      is_case_sensitive_(is_case_sensitive) {}

bool Acceptor::operator==(const Acceptor& other) const {
    return state_graph_ == other.state_graph_;
}

std::string Acceptor::to_string() const {
    return this->repr();
}

std::string Acceptor::repr() const {
    std::string result = "Acceptor";

    if (state_graph_.empty()) {
        return result + "(empty)";
    }

    result += "(graph={\n";
    for (const auto& [state, transitions] : state_graph_) {
        result += "    " + std::visit([](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int>) {
                return std::to_string(arg);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return "'" + arg + "'";
            }
            return "";
        }, state) + ": [";

        bool first = true;
        for (const auto& [acceptor, target_state] : transitions) {
            if (!first) {
                result += ", ";
            }
            first = false;

            result += "(" + acceptor->repr() + ", ";
            result += std::visit([](auto&& arg) -> std::string {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int>) {
                    return std::to_string(arg);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    return "'" + arg + "'";
                }
                return "";
            }, target_state) + ")";
        }
        result += "],\n";
    }
    result += "})";
    return result;
}
