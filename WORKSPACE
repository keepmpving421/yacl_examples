workspace(name = "yacl_test")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# git_repository(
#     name = "yacl",
#     commit = "47dbaf6663a9a80e5a94511a20b44f2350451afe",
#     remote = "https://github.com/secretflow/yacl.git",
# )

local_repository(
  name = "yacl", 
  path = "/home/zxy/yacl/"
)


load("@yacl//bazel:repositories.bzl", "yacl_deps")

yacl_deps()

load("@rules_python//python:repositories.bzl", "py_repositories")

py_repositories()

load(
    "@rules_foreign_cc//foreign_cc:repositories.bzl",
    "rules_foreign_cc_dependencies",
)

rules_foreign_cc_dependencies(
    register_built_tools = False,
    register_default_tools = False,
    register_preinstalled_tools = True,
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()
