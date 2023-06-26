cc_library(
  name = "leb128",
  hdrs = ["leb128.h"],
  deps = [
    "//base:stream",
    "//base:bytes",
  ],
)

cc_test(
    name = "leb128_test",
    size = "small",
    srcs = ["leb128_test.cc"],
    deps = [
        ":leb128",
        "//base:stream",
        "//base:bytes",
        "//third_party/gtest:gtest_main",
    ],
)
