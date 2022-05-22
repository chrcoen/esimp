FROM zephyrprojectrtos/zephyr-build:latest

ARG LLVM_VERSION=11
ARG SYSTEMC_VERSION=2.3.3
ARG WGET_ARGS="-q --show-progress --progress=bar:force:noscroll --no-check-certificate"


USER root


RUN apt-get -y update && \
	apt-get install --no-install-recommends -y \
		gdb \
        clang-format \
        clang-tidy



RUN wget ${WGET_ARGS} -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
	apt-get update && \
	apt-get install -y clang-$LLVM_VERSION lldb-$LLVM_VERSION lld-$LLVM_VERSION clangd-$LLVM_VERSION llvm-$LLVM_VERSION-dev


# Clean up stale packages
RUN apt-get clean -y && \
	apt-get autoremove --purge -y && \
	rm -rf /var/lib/apt/lists/*

USER user

# Install SystemC
COPY systemc_trace.patch /home/user/
RUN cd /home/user && \
    wget ${WGET_ARGS} https://www.accellera.org/images/downloads/standards/systemc/systemc-${SYSTEMC_VERSION}.tar.gz && \
    tar xf systemc-${SYSTEMC_VERSION}.tar.gz && \
    rm systemc-${SYSTEMC_VERSION}.tar.gz && \
    cd systemc-${SYSTEMC_VERSION} && \
    patch -p1 < ../systemc_trace.patch && \
    rm ../systemc_trace.patch && \
    mkdir -p build && cd build && \
    ../configure && \
    make -j $(nproc) && make install && make clean && \
    cd .. && \
    mkdir -p build_debug && cd build_debug && \
    ../configure --enable-debug --disable-optimize --enable-pthreads --with-arch-suffix=-linux64-debug && \
    make -j $(nproc) && make install && make clean

