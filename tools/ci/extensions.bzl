load("//tools/ci:repo_defs.bzl", "ci_configure")

def _ci_configure_extension_impl(_ctx):
    ci_configure(name = "ci_configure")

ci_configure_extension = module_extension(
    implementation = _ci_configure_extension_impl,
)
