def repo_name():
    """Computes the name of the repo for use with runfiles.

    Returns:
      A string representing the name of the repo.
    """

    # Use an arbitrary target for our label
    # In Bzlmod, labels start with @@. In WORKSPACE, they start with @.
    # The runfiles directory for the main repo is _main in Bzlmod.
    if str(Label("//:fake_target")).startswith("@@//"):
        return "_main"

    label_name = Label("//:fake_target").workspace_name
    return label_name if label_name else "flatbuffers"
