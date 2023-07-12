cc_library(
    name = "compiler",
    srcs = [
        "compiler.cc",
        "function.cc",
    ],
    hdrs = [
        "compiler.h",
        "function.h",
    ],
    deps = [
        "//base:coro",
        "//compiler/arm64",
        "//compiler/common",
        "//compiler/x64",
        "//core:ast",
        "//third_party/absl/container:flat_hash_map",
    ],
)

cc_test(
    name = "compiler_test",
    size = "small",
    srcs = ["compiler_test.cc"],
    deps = [
        ":compiler",
        "//base:stream",
        "//parser",
        "//testing:wat",
        "//third_party/gtest:gtest_main",
    ],
)
