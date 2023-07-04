cc_library(
    name = "compiler",
    srcs = [
        "compiler.cc",
        "function.cc",
    ],
    hdrs = [
        "compiler.h",
        "function.h",
    ],
    deps = [
        "//base:coro",
        "//compiler/common",
        "//compiler/x64",
        "//core:ast",
        "//third_party/absl/container:flat_hash_map",
    ],
)
