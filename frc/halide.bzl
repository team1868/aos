load("@rules_cc//cc:cc_library.bzl", "cc_library")

def halide_library(name, src, function, args, visibility = None):
    native.genrule(
        name = name + "_build_generator",
        outs = [
            name + "_generator",
        ],
        srcs = [
            src,
        ],
        cmd = "$(location //frc:halide_generator_compile_script) $(OUTS) $(location " + src + ")",
        tools = [
            "//frc:halide_generator_compile_script",
        ],
    )
    native.genrule(
        name = "generate_" + name,
        srcs = [
            ":" + name + "_generator",
        ],
        outs = [
            name + ".h",
            name + ".o",
            name + ".stmt.html",
        ],
        # TODO(austin): Upgrade halide...
        cmd = "$(location :" + name + "_generator) -g '" + function + "' -o $(RULEDIR) -f " + name + " -e 'o,h,html' " + select({
            "@platforms//cpu:aarch64": "target=arm-64-linux ",
            "@platforms//cpu:x86_64": "target=host ",
            "//conditions:default": "",
        }) + args,
        target_compatible_with = select({
            "@platforms//cpu:arm64": [],
            "@platforms//cpu:x86_64": [],
            "//conditions:default": ["@platforms//:incompatible"],
        }) + ["@platforms//os:linux"],
    )
    cc_library(
        name = name,
        srcs = [name + ".o"],
        hdrs = [name + ".h"],
        visibility = visibility,
        target_compatible_with = select({
            "@platforms//cpu:arm64": [],
            "@platforms//cpu:x86_64": [],
            "//conditions:default": ["@platforms//:incompatible"],
        }) + ["@platforms//os:linux"],
        deps = [
            "//third_party:halide_runtime",
        ],
    )
