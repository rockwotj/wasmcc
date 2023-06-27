package(
    default_visibility = ["//visibility:public"],
)

cc_library(
  name = "value",
  hdrs = ["value.h"],
  srcs = ["value.cc"], 
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
    ":value",
    ":instruction",
    "//base:bytes",
    "//base:named_type",
  ],
)
