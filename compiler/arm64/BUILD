cc_library(
    name = "arm64",
    srcs = [
        "call_convention.cc",
        "compiler.cc",
    ],
    hdrs = [
        "call_convention.h",
        "compiler.h",
        "register_tracker.h",
        "runtime_stack.h",
    ],
    visibility = [
        "//compiler:__pkg__",
    ],
    deps = [
        "//base:align",
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
    size = "small",
    srcs = [
        "register_tracker_test.cc",
    ],
    deps = [
        ":arm64",
        "//third_party/absl/container:flat_hash_set",
        "//third_party/gtest:gtest_main",
    ],
)

cc_test(
    name = "runtime_stack_test",
    size = "small",
    srcs = [
        "runtime_stack_test.cc",
    ],
    deps = [
        ":arm64",
        "//third_party/gtest:gtest_main",
    ],
)
