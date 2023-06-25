cc_library(
    name = "asmjit",
    srcs = glob([
        "src/asmjit/core/*.cpp",
        "src/asmjit/x86/*.cpp",
        "src/asmjit/arm/*.cpp",
    ]),
    hdrs = glob([
        "src/asmjit/x86/*.h",
        "src/asmjit/core/*.h",
        "src/asmjit/*.h",
        "src/asmjit/arm/*.h",
    ]),
    copts = [
        "-DASMJIT_EMBED",
        "-fmerge-all-constants",
        "-Wno-deprecated-anon-enum-enum-conversion",
        "-Wno-deprecated-enum-enum-conversion",
        "-fno-threadsafe-statics",
        "-fno-semantic-interposition",
    ],
    includes = [
        "asmjit/",
        "src/",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"],
)
