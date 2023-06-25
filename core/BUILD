package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "coro",
    hdrs = ["coro.h"],
    srcs = ["coro.cc"],
)

cc_test(
    name = "coro_test",
    size = "small",
    srcs = ["coro_test.cc"],
    deps = [
        ":coro",
        "//third_party/gtest:gtest_main",
    ],
)
