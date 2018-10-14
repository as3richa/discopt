#ifndef _BNB_H_
#define _BNB_H_

#include <queue>
#include <vector>
#include <functional>

template <typename State> bool bnb_optimize(const State initial, State* best) {
  if(!initial.feasible()) {
    return false;
  }

  bool solved = false;

  const auto comparator = [](const State& x, const State& y) {
    if(y.leaf() && !x.leaf()) {
      return true;
    }

    if(x.leaf() && !y.leaf()) {
      return false;
    }
    return (
      x.heuristicValue() < y.heuristicValue() ||
      (x.heuristicValue() == y.heuristicValue() && x < y)
    );
  };

  std::priority_queue<State, std::vector<State>, std::function<bool(const State&, const State&)>> pq(comparator);
  pq.push(initial);

  while(!pq.empty()) {
    const State st = pq.top();
    pq.pop();

    assert(st.feasible());

    // fprintf(stderr, "%d, %lld, %lld, %d, %lld\n", (int)pq.size(), st.value(), st.heuristicValue(), st.considered_, best->value());

    if(st.leaf()) {
      if(!solved || best->value() < st.value()) {
        solved = true;
        (*best) = st;
      }
      continue;
    }

    if(solved && st.heuristicValue() <= best->value()) {
      continue;
    }

    const State left = st.left();
    if(left.feasible()) {
      pq.push(left);
    }

    const State right = st.right();
    if(right.feasible()) {
      pq.push(right);
    }
  }

  return best;
}

#endif
