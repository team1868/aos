load("//tools/python:defs.bzl", "py_binary", "py_library", "py_test")

package(default_visibility = ["//visibility:public"])

py_library(
    name = "experience_buffer",
    srcs = [
        "experience_buffer.py",
    ],
    deps = [
        "@pip_deps//flashbax",
        "@pip_deps//jax",
    ],
)

py_test(
    name = "experience_buffer_test",
    srcs = [
        "experience_buffer_test.py",
    ],
    deps = [
        ":experience_buffer",
        "@pip_deps//flashbax",
        "@pip_deps//jax",
    ],
)

py_binary(
    name = "main",
    srcs = [
        "main.py",
        "model.py",
        "train.py",
    ],
    deps = [
        ":experience_buffer",
        ":physics",
        "//frc/control_loops/swerve:jax_dynamics",
        "@pip_deps//absl_py",
        "@pip_deps//aim",
        "@pip_deps//clu",
        "@pip_deps//flashbax",
        "@pip_deps//flax",
        "@pip_deps//jax",
        "@pip_deps//jaxtyping",
        "@pip_deps//matplotlib",
        "@pip_deps//numpy",
        "@pip_deps//tensorflow",
        "@pip_deps//tensorflow_probability",
        "@pip_deps//tf_keras",
    ],
)

py_library(
    name = "physics",
    srcs = ["physics.py"],
    deps = [
        "//frc/control_loops/swerve:jax_dynamics",
        "@pip_deps//flax",
        "@pip_deps//jax",
        "@pip_deps//jaxtyping",
    ],
)

py_binary(
    name = "plot",
    srcs = [
        "model.py",
        "plot.py",
    ],
    deps = [
        ":experience_buffer",
        ":physics",
        "//frc/control_loops/swerve:jax_dynamics",
        "@pip_deps//absl_py",
        "@pip_deps//flashbax",
        "@pip_deps//flax",
        "@pip_deps//jax",
        "@pip_deps//jaxtyping",
        "@pip_deps//matplotlib",
        "@pip_deps//numpy",
        "@pip_deps//pygobject",
        "@pip_deps//tensorflow",
        "@pip_deps//tensorflow_probability",
        "@pip_deps//tf_keras",
    ],
)

py_binary(
    name = "lqr_plot",
    srcs = [
        "lqr_plot.py",
        "model.py",
    ],
    deps = [
        ":experience_buffer",
        ":physics",
        "//frc/control_loops/swerve:jax_dynamics",
        "@pip_deps//absl_py",
        "@pip_deps//flashbax",
        "@pip_deps//flax",
        "@pip_deps//jax",
        "@pip_deps//jaxtyping",
        "@pip_deps//matplotlib",
        "@pip_deps//numpy",
        "@pip_deps//pygobject",
        "@pip_deps//tensorflow",
    ],
)
