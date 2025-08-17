load(":defs.bzl", "extract_numpy_headers")

cc_library(
    name = "python",
    defines = [
        "AOS_UPSTREAM_BUNDLED_PYTHON",
    ],
    # TODO(austin): We can probably be less restrictive, but this is pretty safe.
    target_compatible_with = [
        "@platforms//cpu:x86_64",
        "@platforms//os:linux",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@rules_python//python/cc:current_py_cc_headers",
        "@rules_python//python/cc:current_py_cc_libs",
    ],
)

filegroup(
    name = "python_runtime",
    data = [
        "@python3_9_x86_64-unknown-linux-gnu//:files",
    ],
    target_compatible_with = [
        "@platforms//cpu:x86_64",
        "@platforms//os:linux",
    ],
    visibility = ["//visibility:public"],
)

extract_numpy_headers(
    name = "numpy_headers",
    header_prefix = "numpy_headers",
    numpy = "@pip_deps//numpy",
    visibility = ["//visibility:private"],
)

cc_library(
    name = "numpy_cc",
    hdrs = [
        ":numpy_headers",
    ],
    includes = [
        "numpy_headers",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":python",
    ],
)
