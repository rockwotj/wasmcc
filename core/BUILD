package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "value",
    srcs = ["value.cc"],
    hdrs = ["value.h"],
)

cc_library(
    name = "instruction",
    hdrs = ["instruction.h"],
    deps = [":value"],
)

cc_library(
    name = "ast",
    hdrs = ["ast.h"],
    deps = [
        ":instruction",
        ":value",
        "//base:bytes",
        "//base:named_type",
    ],
)

cc_test(
  name = "value_test",
  srcs = ["value_test.cc"],
  size = "small",
  deps = [
    ":value",
    "//third_party/gtest:gtest_main",
  ],
)
