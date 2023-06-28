genrule(
    name = "generate_cfg",
    srcs = [
        "src/config.h.in",
    ],
    outs = ["include/wabt/config.h"],
    cmd = "cat $(locations src/config.h.in) > $@",
)

cc_library(
    name = "picosha2",
    hdrs = [
        "third_party/picosha2/picosha2.h",
    ],
    strip_include_prefix = "third_party/picosha2",
    textual_hdrs = glob(["src/prebuilt/*.cc"]),
)

cc_library(
    name = "wabt",
    srcs = glob(
        [
            "src/*.cc",
            "src/*.c",
            "src/interp/*.cc",
        ],
        exclude = [
            "src/test-*.cc",
            "src/interp/interp-wasm-c-api.cc",
        ],
    ),
    hdrs = glob([
        "include/wabt/*.h",
        "include/wabt/interp/*.h",
        "include/wabt/*.def",
    ]) + [":generate_cfg"],
    copts = [
      "-Wno-deprecated-declarations",
    ],
    strip_include_prefix = "include",
    textual_hdrs = glob([
        "src/prebuilt/*.cc",
    ]),
    deps = [":picosha2"],
    testonly = True,
    visibility = ["//visibility:public"],
)
