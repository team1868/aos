load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_file")
load("//debian:phoenix6.bzl", "files")

def _convert_deb_to_target(deb):
    target = deb
    target = target.replace("-", "_")
    target = target.replace(".", "_")
    target = target.replace(":", "_")
    target = target.replace("+", "x")
    target = target.replace("~", "_")
    return "deb_%s_repo" % target

def _debian_deps_extension_impl(_ctx):
    base_url = "https://realtimeroboticsgroup.org/build-dependencies"
    for f, sha256 in files.items():
        name = _convert_deb_to_target(f)
        http_file(
            name = name,
            urls = [base_url + "/" + f],
            sha256 = sha256,
            downloaded_file_path = f,
        )

debian_deps_extension = module_extension(
    implementation = _debian_deps_extension_impl,
)
