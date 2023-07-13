workspace(name = "wasmcc")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

# Hedron's Compile Commands Extractor for Bazel
# https://github.com/hedronvision/bazel-compile-commands-extractor
http_archive(
    name = "hedron_compile_commands",
    sha256 = "3cd0e49f0f4a6d406c1d74b53b7616f5e24f5fd319eafc1bf8eee6e14124d115",
    strip_prefix = "bazel-compile-commands-extractor-3dddf205a1f5cde20faf2444c1757abe0564ff4c",
    url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/3dddf205a1f5cde20faf2444c1757abe0564ff4c.tar.gz",
)

http_archive(
    name = "com_google_googletest",
    sha256 = "1da22a94baad2d93f1f776b3a6dfc3deed004357502756cc24c6e119ce611574",
    strip_prefix = "googletest-ec4fed93217bc2830959bb8e86798c1d86956949",
    urls = ["https://github.com/google/googletest/archive/ec4fed93217bc2830959bb8e86798c1d86956949.zip"],
)

http_archive(
    name = "com_asmjit",
    build_file = "//third_party/asmjit:asmjit.BUILD",
    sha256 = "4845eb9d9e6e8da34694c451a00bc3a4c02fe1f60e12dbde9f09ae5ecb690528",
    strip_prefix = "asmjit-3577608cab0bc509f856ebf6e41b2f9d9f71acc4",
    urls = ["https://github.com/asmjit/asmjit/archive/3577608cab0bc509f856ebf6e41b2f9d9f71acc4.zip"],
)

http_archive(
    name = "com_google_absl",
    sha256 = "51d676b6846440210da48899e4df618a357e6e44ecde7106f1e44ea16ae8adc7",
    strip_prefix = "abseil-cpp-20230125.3",
    urls = ["https://github.com/abseil/abseil-cpp/archive/refs/tags/20230125.3.zip"],
)

http_archive(
    name = "bazel_skylib",
    sha256 = "f7be3474d42aae265405a592bb7da8e171919d74c16f082a5457840f06054728",
    urls = ["https://github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz"],
)

http_archive(
    name = "nlohmann_json",
    build_file = "//third_party/json:json.BUILD",
    sha256 = "8c4b26bf4b422252e13f332bc5e388ec0ab5c3443d24399acb675e68278d341f",
    url = "https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz",
)

http_archive(
    name = "com_github_webassembly_wabt",
    build_file = "//third_party/wabt:wabt.BUILD",
    patch_args = ["-p1"],
    patches = ["//third_party/wabt:config.patch"],
    sha256 = "67f74a55d8e5e811b74443d40707a6fc814cadc3f2b60e11153c32c16922c182",
    strip_prefix = "wabt-1.0.33",
    url = "https://github.com/WebAssembly/wabt/releases/download/1.0.33/wabt-1.0.33.tar.xz",
)

http_archive(
    name = "platforms",
    sha256 = "5308fc1d8865406a49427ba24a9ab53087f17f5266a7aabbfc28823f3916e1ca",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/platforms/releases/download/0.0.6/platforms-0.0.6.tar.gz",
        "https://github.com/bazelbuild/platforms/releases/download/0.0.6/platforms-0.0.6.tar.gz",
    ],
)

http_archive(
    name = "rules_license",
    sha256 = "4531deccb913639c30e5c7512a054d5d875698daeb75d8cf90f284375fe7c360",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_license/releases/download/0.0.7/rules_license-0.0.7.tar.gz",
        "https://github.com/bazelbuild/rules_license/releases/download/0.0.7/rules_license-0.0.7.tar.gz",
    ],
)

load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")

hedron_compile_commands_setup()
