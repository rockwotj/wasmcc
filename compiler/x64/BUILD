cc_library(
    name = "x64",
    srcs = [
        "compiler.cc",
        "register_tracker.cc",
        "runtime_stack.cc",
    ],
    hdrs = [
        "compiler.h",
        "register_tracker.h",
        "runtime_stack.h",
    ],
    visibility = [
        "//compiler:__pkg__",
    ],
    deps = [
        "//compiler/common",
        "//core:ast",
        "//core:value",
        "//third_party/absl/container:fixed_array",
        "//third_party/absl/strings:str_format",
        "//third_party/asmjit",
    ],
)

cc_test(
    name = "register_tracker_test",
    srcs = [
        "register_tracker_test.cc",
    ],
    deps = [
        ":x64",
        "//third_party/absl/container:flat_hash_set",
        "//third_party/gtest:gtest_main",
    ],
)

cc_test(
    name = "runtime_stack_test",
    srcs = [
        "runtime_stack_test.cc",
    ],
    deps = [
        ":x64",
        "//third_party/gtest:gtest_main",
    ],
)
