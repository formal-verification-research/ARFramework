FROM ubuntu:latest

RUN apt-get update
WORKDIR /verapak

# tensorflow dependencies
RUN apt-get install -y git build-essential python3-dev python3-pip g++ unzip zip wget
RUN pip3 install -U pip numpy wheel
RUN pip install -U keras_preprocessing --no-deps

# install bazel

# build tensorflow
RUN git clone https://github.com/formal-verification-research/tensorflow.git
WORKDIR /verapak/tensorflow
RUN git checkout v1.15.4 && git submodule update --init --recursive

WORKDIR /verapak/tensorflow/tensorflow
RUN git clone https://github.com/formal-verification-research/ARFramework.git

WORKDIR /verapak/tensorflow
RUN ln -s /usr/bin/python3 /usr/bin/python
RUN echo "\n" | ./configure

RUN wget https://github.com/bazelbuild/bazel/releases/download/0.26.1/bazel-0.26.1-installer-linux-x86_64.sh && chmod +x bazel-0.26.1-installer-linux-x86_64.sh && ./bazel-0.26.1-installer-linux-x86_64.sh

RUN bazel build -c opt --verbose_failures --jobs=16 --config=noaws --config=nogcp --config=noignite --config=nokafka --config=nonccl --config=v1 -s //tensorflow/ARFramework/...

ENV PATH=$PATH:/verapak/tensorflow/bazel-bin/tensorflow/ARFramework
