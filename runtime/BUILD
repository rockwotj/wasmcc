cc_library(
  name = "runtime",
  srcs = ["vm.cc", "function_handle.cc"],
  hdrs = [
    "vm.h",
    "function_handle.h",
    "signature_converter.h",
  ],
  deps = [
    "//compiler:module",
    "//core:ast",
    "//base:type_traits",
    "//third_party/absl/functional:any_invocable",
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
