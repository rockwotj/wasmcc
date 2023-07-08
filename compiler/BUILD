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
        "//compiler/common",
        "//compiler/x64",
        "//compiler/arm64",
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
        "//testing:wat",
        "//parser",
        "//third_party/gtest:gtest_main",
    ],
)
