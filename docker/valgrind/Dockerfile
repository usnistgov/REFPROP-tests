##
## Build me as: docker image build -t gcov . 
## Run me as: docker container run -it --mount type=bind,src="$(pwd)"/output,dst=/output gcov
##

FROM ubuntu:22.04

RUN apt-get -y -m update &&  DEBIAN_FRONTEND=noninteractive apt-get install -y cmake python3 python3-numpy python3-pip g++ gfortran git zip valgrind

# This ADD block forces a build (invalidates the cache) if the git repo contents have changed, otherwise leaves it untouched.
# See https://stackoverflow.com/a/39278224
ADD https://api.github.com/repos/usnistgov/REFPROP-cmake/git/refs/heads/master RPcmake-version.json
RUN git clone --recursive https://github.com/usnistgov/REFPROP-cmake

ADD https://api.github.com/repos/usnistgov/REFPROP-tests/git/refs/heads/master RPtests-version.json
RUN git clone --recursive https://github.com/usnistgov/REFPROP-tests 

ADD REFPROP.zip . 
RUN unzip REFPROP.zip -d REFPROP && ls -l REFPROP

# Build the special shared library & copy fluid files
WORKDIR /REFPROP-cmake
RUN mkdir build && \ 
    cd build && cmake .. -DREFPROP_FORTRAN_PATH=/REFPROP/FORTRAN -DCMAKE_BUILD_TYPE=RelWithDebInfo -DREFPROP_NO_NUMPY=ON && \
    cmake --build .
WORKDIR /REFPROP-cmake/build
RUN cp -rfv /REFPROP/FLUIDS/ . && cp -rfv /REFPROP/MIXTURES/ . && echo `pwd` && echo `ls -l`

# Build the testing EXE
WORKDIR /REFPROP-tests
RUN mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build .

# Set the environment variables
ENV RPPREFIX /REFPROP-cmake/build
ENV RESOURCES /REFPROP-tests/resources

# A little python package for colorizing ANSI output from log as HTML
RUN pip3 install ansi2html

# Copy the build script
ADD run_valgrind.py /

# Run each tag of tests, looking for memory issues
WORKDIR /
# -u for run unbuffered
CMD python3 -u run_valgrind.py

# Alternatively, you can run a single tag by passing a command line argument to the script. To do
# so, comment out the previous CMD command, and uncomment this one:
# CMD python3 -u run_valgrind.py --catchargs [nearvap]