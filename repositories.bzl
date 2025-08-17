load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

def repositories():
    # This one is tricky to get an archive because it has recursive submodules. These semi-automated steps do work though:
    # git clone -b 1.11.321 --recurse-submodules --depth=1 https://github.com/aws/aws-sdk-cpp
    # cd aws-sdk-cpp
    # echo bsdtar -a -cf aws_sdk-version.tar.gz --ignore-zeros @\<\(git archive HEAD\) $(git submodule foreach --recursive --quiet 'echo @\<\(cd $displaypath \&\& git archive HEAD --prefix=$displaypath/\)')
    # Now run the command that printed, and the output will be at aws_sdk-version.tar.gz.
    http_archive(
        name = "aws_sdk",
        build_file = "@aos//debian:aws_sdk.BUILD",
        sha256 = "08856b91139d209f7423e60dd8f74a14ab6d053ca40088fcb42fd02484003e95",
        url = "https://realtimeroboticsgroup.org/build-dependencies/aws_sdk-1.11.321.tar.gz",
    )

    http_archive(
        name = "snappy",
        sha256 = "90f74bc1fbf78a6c56b3c4a082a05103b3a56bb17bca1a27e052ea11723292dc",
        strip_prefix = "snappy-1.2.2",
        url = "https://github.com/google/snappy/archive/refs/tags/1.2.2.tar.gz",
    )

    http_archive(
        name = "com_github_rawrtc_re",
        patch_args = ["-p1"],
        patches = ["@aos//third_party:rawrtc/rawrtc_re.patch"],
        sha256 = "39b31ae8fb98d3c1f405f7d55dc272af1f0f5ba1a6f2381ff88e917d0d7672c9",
        strip_prefix = "re-9384f3a5f38a03c871270fda566045b3bf57bbee",
        url = "https://github.com/rawrtc/re/archive/9384f3a5f38a03c871270fda566045b3bf57bbee.tar.gz",
    )

    http_archive(
        name = "com_github_rawrtc_rew",
        patch_args = ["-p1"],
        patches = ["@aos//third_party:rawrtc/rawrtc_rew.patch"],
        sha256 = "03fe0408b3bdc2e820c0a29e40e3d769028882c7eb7e30de6b7fe61e42a2ff6e",
        strip_prefix = "rew-24c91fd839b40b11f727c902fa46d20874da33fb",
        url = "https://github.com/rawrtc/rew/archive/24c91fd839b40b11f727c902fa46d20874da33fb.tar.gz",
    )

    http_archive(
        name = "com_github_rawrtc_usrsctp",
        patch_args = ["-p1"],
        patches = ["@aos//third_party:rawrtc/rawrtc_usrsctp.patch"],
        sha256 = "374ef5acacc981a8b27b4fe957f081c2e44b45fdb55e652c22ed9012d9ccaf6a",
        strip_prefix = "usrsctp-bd1a92db338ba1e57453637959a127032bb566ff",
        url = "https://github.com/rawrtc/usrsctp/archive/bd1a92db338ba1e57453637959a127032bb566ff.tar.gz",
    )

    http_archive(
        name = "com_github_rawrtc_rawrtc_common",
        patch_args = ["-p1"],
        patches = ["@aos//third_party:rawrtc/rawrtc_rawrtc_common.patch"],
        sha256 = "8ad48a7231aa00d2218392b0fc23e17d34a161f2cc5347f7f38a37bb7e6271a5",
        strip_prefix = "rawrtc-common-aff7a3a3b9bbf49f7d2fc8b123edd301825b3e1c",
        url = "https://github.com/rawrtc/rawrtc-common/archive/aff7a3a3b9bbf49f7d2fc8b123edd301825b3e1c.tar.gz",
    )

    http_archive(
        name = "com_github_rawrtc_rawrtc_data_channel",
        patch_args = ["-p1"],
        patches = ["@aos//third_party:rawrtc/rawrtc_rawrtc_data_channel.patch"],
        sha256 = "0c5bfed79faf5dec5a7de7669156e5d9dacbf425ad495127bab52f28b56afaa8",
        strip_prefix = "rawrtc-data-channel-7b1b8d57c6d07da18cc0de8bbca8cc5e8bd06eae",
        url = "https://github.com/rawrtc/rawrtc-data-channel/archive/7b1b8d57c6d07da18cc0de8bbca8cc5e8bd06eae.tar.gz",
    )

    http_archive(
        name = "com_github_rawrtc_rawrtc",
        patch_args = ["-p1"],
        patches = ["@aos//third_party:rawrtc/rawrtc_rawrtc.patch"],
        sha256 = "ba449026f691ce57cd9a685b1a16d1bbb88d63d8b5eac38406ed7b53d63c9d07",
        strip_prefix = "rawrtc-aa3ae4b247275cc6e69c30613b3a4ba7fdc82d1b",
        url = "https://github.com/rawrtc/rawrtc/archive/aa3ae4b247275cc6e69c30613b3a4ba7fdc82d1b.tar.gz",
    )

    http_file(
        name = "sample_logfile",
        downloaded_file_path = "log.fbs",
        sha256 = "45d1d19fb82786c476d3f21a8d62742abaeeedf4c16a00ec37ae350dcb61f1fc",
        urls = ["https://realtimeroboticsgroup.org/build-dependencies/small_sample_logfile2.fbs"],
    )

    http_file(
        name = "com_github_foxglove_mcap_mcap",
        executable = True,
        sha256 = "e87895e9af36db629ad01c554258ec03d07b604bc61a0a421449c85223357c71",
        urls = ["https://github.com/foxglove/mcap/releases/download/releases%2Fmcap-cli%2Fv0.0.51/mcap-linux-amd64"],
    )
