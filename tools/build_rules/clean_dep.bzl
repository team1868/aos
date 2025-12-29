def clean_dep(target):
    """Returns string to 'target' in @aos repository.

    Use this function when referring to targets in the @aos
    repository from macros that may be called from external repositories.

    This is stolen from tensorflow.bzl (https://github.com/tensorflow/tensorflow/blob/69b50ff7537c7e9bea8ad45b973f75e66a9a0fb9/tensorflow/tensorflow.bzl#L102C1-L111C30).
    """

    # A repo-relative label is resolved relative to the file in which the
    # Label() call appears, i.e. @aos.
    return str(Label(target))

def aos_repo_name():
    """Computes the name of the AOS repo for use with runfiles.

    Returns:
      A string representing the name of the AOS repo.
    """

    # Use an arbitrary target for our label
    # In Bzlmod, labels start with @@. In WORKSPACE, they start with @.
    # The runfiles directory for the main repo is _main in Bzlmod.
    if str(Label("//:fake_target")).startswith("@@//"):
        return "_main"

    label_name = Label("//:fake_target").workspace_name
    return label_name if label_name else "aos"
