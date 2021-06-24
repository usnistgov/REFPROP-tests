## 
## Just use docker-compose to spin up this job
##

FROM ubuntu:20.04

RUN apt-get -y -m update && DEBIAN_FRONTEND=noninteractive apt-get install -y cmake python3-numpy g++ gfortran git zip python3-six lcov python3-pip

# This ADD forces a build (invalidates the cache) if the git repo contents have changed, otherwise leaves it untouched.
# See https://stackoverflow.com/a/39278224
ADD https://api.github.com/repos/usnistgov/REFPROP-cmake/git/refs/heads/master RPcmake-version.json
RUN git clone --recursive https://github.com/usnistgov/REFPROP-cmake

ADD https://api.github.com/repos/usnistgov/REFPROP-tests/git/refs/heads/master RPtests-version.json
RUN git clone --recursive https://github.com/usnistgov/REFPROP-tests 

ADD REFPROP.zip . 

RUN unzip REFPROP.zip -d REFPROP

# Build the special shared library & copy fluid files
WORKDIR /REFPROP-cmake
RUN mkdir build && cd build && \
    cmake .. -DREFPROP_FORTRAN_PATH=/REFPROP/FORTRAN -DCMAKE_BUILD_TYPE=Release -DREFPROP_NO_NUMPY=ON && \
    cmake --build .
WORKDIR /REFPROP-cmake/build
RUN cp -rfv /REFPROP/FLUIDS/ . && cp -rfv /REFPROP/MIXTURES/ . && echo `pwd` && echo `ls -l`

# Build the testing EXE
WORKDIR /REFPROP-tests
RUN mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build .

# A little python package for colorizing ANSI output from log as HTML
RUN python3 -m pip install ansi2html

# Set the environment variables
ENV RPPREFIX /REFPROP-cmake/build
ENV RESOURCES /REFPROP-tests/resources

RUN apt-get install nano
RUN pip3 install ctREFPROP

COPY run_test.py /

# Run each tag of tests
WORKDIR /
# -u for run unbuffered
CMD python3 -u run_test.py