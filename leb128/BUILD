cc_library(
    name = "leb128",
    hdrs = ["leb128.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//base:bytes",
        "//base:stream",
    ],
)

cc_test(
    name = "leb128_test",
    size = "small",
    srcs = ["leb128_test.cc"],
    deps = [
        ":leb128",
        "//base:bytes",
        "//base:stream",
        "//third_party/gtest:gtest_main",
    ],
)
