cc_library(
  name = "runtime",
  srcs = ["vm.cc"],
  hdrs = [
    "vm.h",
    "function_handle.h",
    "function_signature.h",
  ],
  deps = [
    "//compiler:module",
    "//core:ast",
    "//base:type_traits",
    "//runtime/thread",
  ],
)

cc_test(
  name = "vm_test",
  srcs = ["vm_test.cc"],
  size = "small",
  deps = [
    ":runtime",
    "//compiler",
    "//parser",
    "//testing:wat",
    "//third_party/gtest:gtest_main",
  ],
)
