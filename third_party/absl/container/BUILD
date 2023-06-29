package(
    default_visibility = ["//visibility:public"],
)

[
    alias(
        name = container,
        actual = "@com_google_absl//absl/container:" + container,
    )
    for container in [
        "fixed_array",
        "inlined_vector",
        "flat_hash_map",
        "flat_hash_set",
        "node_hash_map",
        "node_hash_set",
        "btree",
    ]
]
