package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "coro",
    srcs = ["coro.cc"],
    hdrs = ["coro.h"],
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

cc_library(
    name = "bytes",
    hdrs = ["bytes.h"],
)

cc_library(
    name = "stream",
    hdrs = ["stream.h"],
    srcs = ["stream.cc"],
    deps = [":bytes"],
)

cc_test(
    name = "stream_test",
    size = "small",
    srcs = ["stream_test.cc"],
    deps = [
        ":stream",
        ":bytes",
        "//third_party/gtest:gtest_main",
    ],
)
