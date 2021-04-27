FROM ubuntu:latest

RUN apt update
WORKDIR /verapak

# tensorflow dependencies
RUN apt install -y git build-essential 
RUN apt install -y python3-dev python3-pip
RUN pip3 install -U pip 
RUN pip install -U numpy wheel
RUN pip install -U keras_preprocessing --no-deps

# install bazel
RUN apt install -y g++ unzip zip wget 
RUN wget https://github.com/bazelbuild/bazel/releases/download/0.26.1/bazel-0.26.1-installer-linux-x86_64.sh
RUN chmod +x bazel-0.26.1-installer-linux-x86_64.sh
RUN ./bazel-0.26.1-installer-linux-x86_64.sh

# build tensorflow
RUN git clone https://github.com/formal-verification-research/tensorflow.git
RUN cd tensorflow && git checkout arframework
RUN cd tensorflow/tensorflow && git clone https://github.com/formal-verification-research/ARFramework.git
WORKDIR /verapak/tensorflow
RUN ln -s /usr/bin/python3 /usr/bin/python
RUN echo "\n" | ./configure
RUN bazel build -c opt --verbose_failures --jobs=16 --config=noaws --config=nogcp --config=noignite --config=nokafka --config=nonccl --config=c++17 -s //tensorflow/ARFramework/...

ENV PATH=$PATH:/verapak/tensorflow/bazel-bin/tensorflow/ARFramework
