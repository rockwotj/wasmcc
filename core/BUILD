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
    srcs = ["instruction.cc"],
    deps = [
      ":value",
      "//base:named_type",
      "//third_party/absl/strings:str_format",
    ],
)

cc_library(
    name = "ast",
    hdrs = ["ast.h"],
    deps = [
        ":instruction",
        ":value",
        "//base:bytes",
        "//base:named_type",
        "//third_party/absl/container:flat_hash_map",
    ],
)

cc_test(
    name = "value_test",
    size = "small",
    srcs = ["value_test.cc"],
    deps = [
        ":value",
        "//third_party/gtest:gtest_main",
    ],
)
