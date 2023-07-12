package(
  default_visibility = ["//visibility:public"],
)

cc_library(
  name = "module",
    srcs = [
        "module.cc",
    ],
    hdrs = [
        "module.h",
    ],
    deps = [
        "//core:ast",
        "//third_party/absl/container:flat_hash_map",
    ],
)

cc_library(
    name = "compiler",
    srcs = [
        "compiler.cc",
    ],
    hdrs = [
        "compiler.h",
    ],
    deps = [
        "//base:coro",
        "//base:assert",
        "//compiler/arm64",
        "//compiler/common",
        "//compiler/x64",
        "//core:ast",
        ":module",
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
