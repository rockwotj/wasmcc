workspace(name = "wasmcc")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

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
  urls = ["https://github.com/abseil/abseil-cpp/archive/e6c09ae4b2acd421a29706f86e66eaa422262ad0.zip"],
  strip_prefix = "abseil-cpp-e6c09ae4b2acd421a29706f86e66eaa422262ad0",
  sha256 = "bd92719155e8ff9854efd9547425920ffa83beaae5de1d0627e9caebc555d415",
)

http_archive(
  name = "bazel_skylib",
  urls = ["https://github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz"],
  sha256 = "f7be3474d42aae265405a592bb7da8e171919d74c16f082a5457840f06054728",
)

http_archive(
    name = "rules_foreign_cc",
    strip_prefix = "rules_foreign_cc-6ecc134b114f6e086537f5f0148d166467042226",
    sha256 = "059d1d1ec0819b316d05eb7f9f0e07c5cf9636e0cbb224d445162f2d0690191e",
    url = "https://github.com/bazelbuild/rules_foreign_cc/archive/6ecc134b114f6e086537f5f0148d166467042226.tar.gz",
)

http_archive(
    name = "nlohmann_json",
    url = "https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz",
    sha256 = "8c4b26bf4b422252e13f332bc5e388ec0ab5c3443d24399acb675e68278d341f",
    build_file = "//third_party/json:json.BUILD",
)

load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")

hedron_compile_commands_setup()

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies()


