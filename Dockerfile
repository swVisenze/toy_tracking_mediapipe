# Copyright 2019 The MediaPipe Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

FROM ubuntu:18.04

MAINTAINER <mediapipe@google.com>

WORKDIR /io
WORKDIR /mediapipe

ENV DEBIAN_FRONTEND=noninteractive
ARG OPENCV_VERSION=3.4.16

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        gcc-8 g++-8 \
        ca-certificates \
        curl \
        ffmpeg \
        git \
        wget \
        python3-dev \
        python3-pip \
        software-properties-common \
        cmake \
        unzip \
        libhdf5-dev \
        libopenblas-dev \
        libprotobuf-dev \
        libjpeg-dev \
        libpng-dev \
        libtiff-dev \
        libwebp-dev \
        libopenjp2-7-dev \
        libeigen3-dev \
        libavformat-dev \
        libdc1394-22-dev \
        libgtk2.0-dev \
        libswscale-dev \
        libtbb2 libtbb-dev && \
    add-apt-repository -y ppa:openjdk-r/ppa && \
    apt-get update && apt-get install -y openjdk-8-jdk && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 100 --slave /usr/bin/g++ g++ /usr/bin/g++-8
RUN pip3 install --upgrade setuptools
RUN pip3 install wheel
RUN pip3 install future
RUN pip3 install six==1.14.0
RUN pip3 install protobuf==3.6.1
RUN pip3 install tensorflow==1.14.0
RUN pip3 install tf_slim
RUN pip3 install typer
RUN pip3 install path
RUN pip3 install joblib

RUN ln -s /usr/bin/python3 /usr/bin/python

# Install opencv
RUN set -ex \
    && wget -q --no-check-certificate https://github.com/opencv/opencv/archive/${OPENCV_VERSION}.zip -O opencv.zip \
    && wget -q --no-check-certificate https://github.com/opencv/opencv_contrib/archive/${OPENCV_VERSION}.zip -O opencv_contrib.zip \
    && unzip -qq opencv.zip -d /opt && rm -rf opencv.zip \
    && unzip -qq opencv_contrib.zip -d /opt && rm -rf opencv_contrib.zip \
    && cmake \
        -D CMAKE_BUILD_TYPE=RELEASE \
        -D CMAKE_INSTALL_PREFIX=/usr/local \
        -D OPENCV_EXTRA_MODULES_PATH=/opt/opencv_contrib-${OPENCV_VERSION}/modules \
        -D EIGEN_INCLUDE_PATH=/usr/include/eigen3 \
        -D OPENCV_ENABLE_NONFREE=FALSE \
        -D WITH_JPEG=ON \
        -D WITH_PNG=ON \
        -D WITH_TIFF=ON \
        -D WITH_WEBP=ON \
        -D WITH_JASPER=ON \
        -D WITH_EIGEN=ON \
        -D WITH_TBB=OFF \
        -D WITH_LAPACK=ON \
        -D WITH_PROTOBUF=ON \
        -D WITH_V4L=OFF \
        -D WITH_GSTREAMER=OFF \
        -D WITH_GTK=OFF \
        -D WITH_QT=OFF \
        -D WITH_CUDA=OFF \
        -D WITH_VTK=OFF \
        -D WITH_OPENEXR=OFF \
        -D WITH_FFMPEG=ON \
        -D WITH_OPENCL=OFF \
        -D WITH_OPENNI=OFF \
        -D WITH_XINE=OFF \
        -D WITH_GDAL=OFF \
        -D WITH_IPP=OFF \
        -D BUILD_OPENCV_PYTHON3=ON \
        -D BUILD_OPENCV_PYTHON2=OFF \
        -D BUILD_OPENCV_JAVA=OFF \
        -D BUILD_TESTS=OFF \
        -D BUILD_IPP_IW=OFF \
        -D BUILD_PERF_TESTS=OFF \
        -D BUILD_EXAMPLES=OFF \
        -D BUILD_ANDROID_EXAMPLES=OFF \
        -D BUILD_DOCS=OFF \
        -D BUILD_ITT=OFF \
        -D INSTALL_PYTHON_EXAMPLES=OFF \
        -D INSTALL_C_EXAMPLES=OFF \
        -D INSTALL_TESTS=OFF \
        -D BUILD_opencv_aruco=OFF \
        -D BUILD_opencv_bgsegm=OFF \
        -D BUILD_opencv_bioinspired=OFF \
        -D BUILD_opencv_ccalib=OFF \
        -D BUILD_opencv_datasets=OFF \
        -D BUILD_opencv_dnn=OFF \
        -D BUILD_opencv_dnn_objdetect=OFF \
        -D BUILD_opencv_dpm=OFF \
        -D BUILD_opencv_face=OFF \
        -D BUILD_opencv_fuzzy=OFF \
        -D BUILD_opencv_hfs=OFF \
        -D BUILD_opencv_img_hash=OFF \
        -D BUILD_opencv_js=OFF \
        -D BUILD_opencv_line_descriptor=OFF \
        -D BUILD_opencv_phase_unwrapping=OFF \
        -D BUILD_opencv_plot=OFF \
        -D BUILD_opencv_quality=OFF \
        -D BUILD_opencv_reg=OFF \
        -D BUILD_opencv_rgbd=OFF \
        -D BUILD_opencv_saliency=OFF \
        -D BUILD_opencv_shape=OFF \
        -D BUILD_opencv_structured_light=OFF \
        -D BUILD_opencv_surface_matching=OFF \
        -D BUILD_opencv_world=OFF \
        -D BUILD_opencv_xobjdetect=OFF \
        -D BUILD_opencv_xphoto=OFF \
        -D CV_ENABLE_INTRINSICS=ON \
        -D WITH_PTHREADS=ON \
        -D WITH_PTHREADS_PF=ON \
        /opt/opencv-${OPENCV_VERSION} \
    && make -j$(nproc) \
    && make install

# Config
RUN touch /etc/ld.so.conf.d/mp_opencv.conf && \
    bash -c  "echo /usr/local/lib >> /etc/ld.so.conf.d/mp_opencv.conf" && \
    ldconfig -v

# Install bazel
ARG BAZEL_VERSION=5.2.0
RUN mkdir /bazel && \
    wget --no-check-certificate -O /bazel/installer.sh "https://github.com/bazelbuild/bazel/releases/download/${BAZEL_VERSION}/b\
azel-${BAZEL_VERSION}-installer-linux-x86_64.sh" && \
    wget --no-check-certificate -O  /bazel/LICENSE.txt "https://raw.githubusercontent.com/bazelbuild/bazel/master/LICENSE" && \
    chmod +x /bazel/installer.sh && \
    /bazel/installer.sh  && \
    rm -f /bazel/installer.sh

ARG USER_ID
ARG GROUP_ID

# RUN addgroup --gid $GROUP_ID user
RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID user
USER user

# COPY . /mediapipe/

# If we want the docker image to contain the pre-built object_detection_offline_demo binary, do the following
# RUN bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/demo:object_detection_tensorflow_demo
