#!/bin/bash
echo 'This should never be executed. Something went wrong.' >&2
echo 'This NOOP Python toolchain should never be executed. Something went wrong.' >&2
echo 'Check that your target has `target_compatible_with` set to a platform that supports Python.' >&2
exit 1
