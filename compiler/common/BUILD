cc_library(
    name = "common",
    srcs = [
        "exception.cc",
        "util.cc",
    ],
    hdrs = [
        "call_convention.h",
        "exception.h",
        "function_frame.h",
        "register_tracker.h",
        "runtime_stack.h",
        "util.h",
    ],
    visibility = [
        "//compiler:__subpackages__",
    ],
    deps = [
        "//base:align",
        "//third_party/asmjit",
    ],
)
