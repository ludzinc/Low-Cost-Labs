void debounce(int test, int *state, int *counter) {
  if ((*state) != test) {
    (*counter)++;
    if ((*counter) >= debounceCount) {
      (*counter) = 0;
      (*state) = test;
    }
  } else {
    (*counter) = 0;
  }
}
