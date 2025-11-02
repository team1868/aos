# Start from the official debian:python base image.
FROM docker.io/python:bookworm

# Set arguments for the user and group IDs.
ARG ENGFLOW_UID=108
ARG ENGFLOW_GID=108
ARG ENGFLOW_USER=engflow

# Create the 'engflow' group with the specific GID.
# Then, create the 'engflow' user with the specific UID and GID.
# We also create a home directory for this user.
RUN apt-get update && \
    apt-get install -y --no-install-recommends strace && \
    groupadd --gid $ENGFLOW_GID $ENGFLOW_USER && \
    useradd --uid $ENGFLOW_UID --gid $ENGFLOW_GID --shell /bin/bash --create-home $ENGFLOW_USER && \
    rm -rf /var/lib/apt/lists/*
