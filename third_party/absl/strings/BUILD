package(
    default_visibility = ["//visibility:public"],
)

[
    alias(
        name = lib,
        actual = "@com_google_absl//absl/strings:" + lib,
    )
    for lib in [
        "strings",
        "str_format",
    ]
]
