load("@aspect_bazel_lib//lib:repositories.bzl", "aspect_bazel_lib_dependencies", "aspect_bazel_lib_register_toolchains", "register_jq_toolchains")
load("@bazel_features//:deps.bzl", "bazel_features_deps")
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
load("@bzlmodrio-ni//:maven_cpp_deps.bzl", "setup_legacy_bzlmodrio_ni_cpp_dependencies")
load("@rules_pkg//:deps.bzl", "rules_pkg_dependencies")
load("@rules_python//python:repositories.bzl", "py_repositories", "python_register_toolchains")

def dependencies_phase1():
    bazel_skylib_workspace()

    aspect_bazel_lib_dependencies()

    aspect_bazel_lib_register_toolchains()

    register_jq_toolchains()

    py_repositories()

    bazel_features_deps()

    python_register_toolchains(
        name = "python_3_10",
        python_version = "3.10",
    )

    setup_legacy_bzlmodrio_ni_cpp_dependencies()

    rules_pkg_dependencies()
