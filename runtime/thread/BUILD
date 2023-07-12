cc_library(
    name = "thread",
    srcs = [
        "thread.cc",
    ] + select({
        "@platforms//cpu:x86_64": ["thread_x64.cc"],
        "@platforms//cpu:arm64": ["thread_arm64.cc"],
    }),
    hdrs = [
        "thread.h",
    ],
    visibility = [
        "//runtime:__subpackages__",
    ],
    deps = [
        "//base:align",
        "//base:assert",
    ],
)

cc_test(
    name = "thread_test",
    size = "small",
    srcs = ["thread_test.cc"],
    deps = [
        ":thread",
        "//third_party/gtest:gtest_main",
    ],
)
