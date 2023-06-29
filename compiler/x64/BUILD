cc_library(
    name = "x64",
    srcs = [
        "register_tracker.cc",
        "runtime_stack.cc",
    ],
    hdrs = [
        "register_tracker.h",
        "runtime_stack.h",
    ],
    deps = [
        "//core:value",
        "//third_party/absl/container:fixed_array",
        "//third_party/asmjit",
    ],
)

cc_test(
  name = "register_tracker_test",
  srcs = [
    "register_tracker_test.cc",
  ],
  deps = [
    "//third_party/gtest:gtest_main",
        "//third_party/absl/container:flat_hash_set",
    ":x64",
  ],
)
