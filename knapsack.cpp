#include <cstdio>
#include <cassert>
#include <cmath>
#include <vector>
#include <algorithm>
#include <bitset>

#include "bnb.hpp"

using namespace std;

const int max_n = 10 * 1000;

typedef long long ll;

typedef struct {
  int index;
  ll weight;
  ll value;
} item_t;

typedef struct {
  int n;
  ll k;
  item_t* items;
  ll* cumulative_value;
  ll* cumulative_weight;
} context_t;

class State {
public:
  const context_t* context_;
  int considered_;
  ll value_;
  ll weight_;
  ll heuristic_value_;
  bitset<max_n> taken_;

  void compute_heuristic_value() {
    const int n = context_->n;
    const ll* cv = context_->cumulative_value;
    const ll* cw = context_->cumulative_weight;
    const item_t* items = context_->items;

    ll remaining = context_->k - weight_;

    const ll base_value = ((considered_ == 0) ? 0 : cv[considered_ - 1]);
    const ll base_weight = ((considered_ == 0) ? 0 : cw[considered_ - 1]);
    const int goal_weight = base_weight + remaining;

    const int right = (int)(std::upper_bound(cw + considered_, cw + n, goal_weight) - cw);

    double hv = value_;

    /* We can take every item in [considered_, right), plus some fraction of right */

    if(right > considered_) {
      hv += cv[right - 1] - base_value;
      remaining -= cw[right - 1] - base_weight;

      assert(remaining >= 0);
    }

    if(right < n) {
      assert(remaining < items[right].weight);
      hv += remaining * (double)items[right].value / items[right].weight;
    }

    heuristic_value_ = (ll)ceil(hv);
  }

public:

  State() { ; }

  State(const context_t* context):
    context_(context), considered_(0), value_(0), weight_(0) {
      compute_heuristic_value();
    }

  long long value() const {
    return value_;
  }

  long long heuristicValue() const {
    return heuristic_value_;
  }

  bool leaf() const {
    return (considered_ == context_->n);
  }

  bool feasible() const {
    return (weight_ <= context_->k);
  }

  State left() const {
    assert(!leaf());
    assert(feasible());

    State next = (*this);
    next.considered_ ++;
    next.compute_heuristic_value();
    return next;
  }

  State right() const {
    assert(!leaf());
    assert(feasible());

    const int index = considered_;
    const item_t item = context_->items[index];

    State next = (*this);
    next.considered_ ++;
    next.value_ += item.value;
    next.weight_ += item.weight;
    next.taken_.set(index);
    next.compute_heuristic_value();
    return next;
  }

  bool operator<(const State& other) const {
    return (considered_ < other.considered_);
  }

  void to_boolean_array(bool* result) {
    for(int i = 0; i < context_->n; i ++) {
      result[i] = taken_[i];
    }
  }
};

int solve_dp(int n, ll k, item_t* items, bool* take);
int solve_bnb(int n, ll k, item_t* items, bool* take);

int main(void) {
  int n;
  ll k;
  item_t items[max_n];

  scanf("%d %lld", &n, &k);
  assert(1 <= n && n <= max_n);

  for(int i = 0; i < n; i ++) {
    scanf("%lld %lld", &items[i].value, &items[i].weight);
    items[i].index = i;
  }

  bool take[max_n];
  int value;

  if(n * k <= 100 * 1000 * 1000) {
    fprintf(stderr, "n * k <= 1e8; using DP\n");
    value = solve_dp(n, k, items, take);
  } else {
    fprintf(stderr, "n * k > 1e8; using BnB\n");
    value = solve_bnb(n, k, items, take);
  }

  printf("%d 1\n", value);

  for(int i = 0; i < n; i ++) {
    printf("%d%c", (int)take[i], ((i == n - 1) ? '\n' : ' '));
  }

  return 0;
}

int solve_dp(int n, ll k, item_t* items, bool* take) {
  vector<vector<ll>> table(n + 1);

  for(int i = 0; i <= n; i ++) {
    table[i].resize(k + 1);
  }

  for(int i = 1; i <= n; i ++) {
    const item_t item = items[i - 1];

    for(int j = 1; j <= k; j ++) {
      table[i][j] = table[i - 1][j];

      if(item.weight <= j) {
        table[i][j] = max(table[i][j], table[i - 1][j - item.weight] + item.value);
      }
    }
  }

  for(int i = n, j = k; i > 0; i --) {
    if(table[i][j] == table[i - 1][j]) {
      take[i - 1] = false;
    } else {
      take[i - 1] = true;
      j -= items[i - 1].weight;
    }
  }

  return table[n][k];
}

int solve_bnb(int n, ll k, item_t* it, bool* take) {
  /* Copy given items into a local array for sorting */
  item_t items[max_n];
  memcpy(items, it, sizeof(item_t) * n);

  /* Sort in descending order of value density */
  const auto comparator = [](const item_t& x, const item_t& y) {
    return (double)x.value / x.weight > (double)y.value / y.weight;
  };
  sort(items, items + n, comparator);

  ll cumulative_value[max_n];
  ll cumulative_weight[max_n];

  for(int i = 0; i < n; i ++) {
    cumulative_value[i] = items[i].value;
    cumulative_weight[i] = items[i].weight;

    if(i > 0) {
      cumulative_value[i] += cumulative_value[i - 1];
      cumulative_weight[i] += cumulative_weight[i - 1];
    }
  }

  context_t context = {
    n, k,
    items,
    cumulative_value,
    cumulative_weight
  };

  const State initial(&context);

  State best;
  const bool solved = bnb_optimize(initial, &best);

  assert(solved);

  /* best records the optimal configuration with respect to the sorted
   * array of items; we need to transform this configuration to match
   * the original, unsorted array */
  bool buf[max_n];
  best.to_boolean_array(buf);

  for(int i = 0; i < n; i ++) {
    take[items[i].index] = buf[i];
  }

  return best.value();
}
