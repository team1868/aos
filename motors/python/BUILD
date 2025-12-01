load("//tools/python:defs.bzl", "py_binary", "py_library")

py_binary(
    name = "big_phase_current",
    srcs = [
        "big_phase_current.py",
    ],
    legacy_create_init = False,
    target_compatible_with = ["@platforms//cpu:x86_64"],
    deps = [
        ":python_init",
        "//frc/control_loops/python:controls",
        "@pip_deps//glog",
        "@pip_deps//matplotlib",
        "@pip_deps//python_gflags",
        "@pip_deps//scipy",
    ],
)

py_binary(
    name = "haptic_phase_current",
    srcs = [
        "haptic_phase_current.py",
    ],
    legacy_create_init = False,
    target_compatible_with = ["@platforms//cpu:x86_64"],
    deps = [
        ":python_init",
        "//frc/control_loops/python:controls",
        "@pip_deps//glog",
        "@pip_deps//python_gflags",
    ],
)

py_library(
    name = "python_init",
    srcs = ["__init__.py"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = ["//motors:python_init"],
)
