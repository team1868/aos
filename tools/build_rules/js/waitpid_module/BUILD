load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load(":defs.bzl", "collect_nodejs_headers")

collect_nodejs_headers(
    name = "nodejs_headers",
    src = "@nodejs_linux_amd64//:npm_files",
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
)

cc_binary(
    name = "waitpid_module.so",
    srcs = [
        "waitpid_module.cc",
    ],
    linkshared = True,
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    deps = [":nodejs_headers"],
)

# Native modules have to have the .node extension. cc_binary will append .so if
# we use `name = "waitpid_module.node"` so we manually rename the file here.
genrule(
    name = "rename",
    srcs = [":waitpid_module.so"],
    outs = [":waitpid_module.node"],
    cmd = "cp $(SRCS) $(OUTS)",
)

filegroup(
    name = "waitpid_module",
    srcs = [
        ":waitpid_module.node",
    ],
    visibility = ["//visibility:public"],
)
