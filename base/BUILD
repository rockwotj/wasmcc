package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "align",
    hdrs = ["align.h"],
)

cc_library(
    name = "named_type",
    hdrs = ["named_type.h"],
)

cc_library(
    name = "type_traits",
    hdrs = ["type_traits.h"],
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
    srcs = ["stream.cc"],
    hdrs = ["stream.h"],
    deps = [":bytes"],
)

cc_test(
    name = "stream_test",
    size = "small",
    srcs = ["stream_test.cc"],
    deps = [
        ":bytes",
        ":stream",
        "//third_party/gtest:gtest_main",
    ],
)
