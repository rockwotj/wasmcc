cc_library(
    name = "common",
    srcs = [
        "exception.cc",
        "util.cc",
    ],
    hdrs = [
        "exception.h",
        "call_convention.h",
        "register_tracker.h",
        "runtime_stack.h",
        "function_frame.h",
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
