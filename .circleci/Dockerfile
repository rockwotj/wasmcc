FROM fedora:38

ARG TARGETOS
ARG TARGETARCH

RUN sudo dnf install -y \
  curl git lld clang llvm \
  pkg-config clang-tools-extra

RUN curl -SL -o /tmp/bazel \
  https://github.com/bazelbuild/bazelisk/releases/download/v1.17.0/bazelisk-$TARGETOS-$TARGETARCH \
  && chmod +x /tmp/bazel && sudo mkdir -p /usr/local/bin && sudo mv /tmp/bazel /usr/local/bin/
