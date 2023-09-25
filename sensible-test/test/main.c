#include "sensible-test.h"

int main(void) {
  struct test_config config = {
    .filter_str = "",
    .junit = true,
  };
  struct test_state state = test_state_new(config);
  test_group(&state, "group 1") {
    test_group(&state, "group 2") {
      test(&state, "one is one") {
        test_assert_eq(&state, 1, 1);
      }
    }
    test_group(&state, "group 2") {
      test(&state, "one is two") {
        test_assert_eq(&state, 1, 1);
      }
    }
  }
  test_state_finalize(&state);
  print_failures(&state);
}
