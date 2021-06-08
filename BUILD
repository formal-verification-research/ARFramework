# Description:
#   TensorFlow C++ inference example for labeling images.

package(
    default_visibility = ["//tensorflow:internal"],
)

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

exports_files(["data/grace_hopper.jpg"])

load("//tensorflow:tensorflow.bzl", "tf_cc_binary")

tf_cc_binary(
    name = "ARFramework_main",
    copts = ["--std=c++14"],
    srcs = [
        "main.cpp",
        "GraphManager.cpp",
        "ARFramework.cpp",
        "grid_tools.cpp",
        "tensorflow_graph_tools.cpp",
    ],
    includes = [
        "GraphManager.hpp",
        "ARFramework.hpp",
        "grid_tools.hpp",
        "tensorflow_graph_tools.hpp",
    ],
    linkopts = ["-lm"],
    deps = [
        "//tensorflow/cc:cc_ops",
        "//tensorflow/core:core_cpu",
        "//tensorflow/core:framework",
        "//tensorflow/core:tensorflow",
    ],
)

tf_cc_binary(
    name = "ARFramework_tools_test",
    copts = ["--std=c++14"],
    srcs = [
        "test.cpp",
        "grid_tools.cpp",
    ],
    includes = [
        "grid_tools.hpp",
    ],
    linkopts = ["-lm"],
    deps = [
        "//tensorflow/cc:cc_ops",
        "//tensorflow/core:core_cpu",
        "//tensorflow/core:framework",
        "//tensorflow/core:tensorflow",
    ],
)

tf_cc_binary(
    name = "ARFramework_FGSM_test",
    copts = ["--std=c++14"],
    srcs = [
        "FGSMTest.cpp",
        "grid_tools.cpp",
        "GraphManager.cpp",
        "tensorflow_graph_tools.cpp",
    ],
    includes = [
        "grid_tools.hpp",
        "GraphManager.hpp",
        "tensorflow_graph_tools.hpp",
    ],
    linkopts = ["-lm"],
    deps = [
        "//tensorflow/cc:cc_ops",
        "//tensorflow/core:core_cpu",
        "//tensorflow/core:framework",
        "//tensorflow/core:tensorflow",
    ],
)

