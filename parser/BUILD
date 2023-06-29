cc_library(
    name = "validator",
    srcs = ["validator.cc"],
    hdrs = ["validator.h"],
    deps = [
        "//base:type_traits",
        "//core:ast",
    ],
)

cc_test(
    name = "validator_test",
    size = "small",
    srcs = ["validator_test.cc"],
    deps = [
        ":validator",
        "//third_party/gtest:gtest_main",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":validator",
        "//base:coro",
        "//base:stream",
        "//core:ast",
        "//leb128",
        "//third_party/absl/strings:str_format",
    ],
)

cc_test(
    name = "parser_test",
    size = "small",
    srcs = ["parser_test.cc"],
    deps = [
        ":parser",
        "//third_party/gtest:gtest_main",
    ],
)
