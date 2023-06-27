
cc_library(
  name = "validator",
  hdrs = ["validator.h"],
  srcs = ["validator.cc"],
  deps = [
    "//core:ast",
    "//base:type_traits",
  ],
)

cc_test(
  name = "validator_test",
  srcs = ["validator_test.cc"],
  size = "small",
  deps = [
    ":validator",
    "//third_party/gtest:gtest_main",
  ],
)

cc_library(
  name = "parser",
  hdrs = ["parser.h"],
  srcs = ["parser.cc"],
  deps = [
    "//core:ast",
    "//base:stream",
    "//base:coro",
    "//leb128",
    ":validator",
    "//third_party/absl/strings:str_format",
  ],
  visibility = ["//visibility:public"],
)

