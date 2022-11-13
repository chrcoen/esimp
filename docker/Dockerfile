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


# Install llvm
RUN wget ${WGET_ARGS} -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
	apt-get update && \
	apt-get install -y clang-$LLVM_VERSION lldb-$LLVM_VERSION lld-$LLVM_VERSION clangd-$LLVM_VERSION llvm-$LLVM_VERSION-dev

# Set clang version as default
RUN ln -s /usr/bin/clang-${LLVM_VERSION} /usr/bin/clang && \
    ln -s /usr/bin/clang++-${LLVM_VERSION} /usr/bin/clang++ && \
    ln -s /usr/bin/llvm-objdump-${LLVM_VERSION} /usr/bin/llvm-objdump
    
# Clean up stale packages
RUN apt-get clean -y && \
	apt-get autoremove --purge -y && \
	rm -rf /var/lib/apt/lists/*

USER user

# Install SystemC
RUN mkdir -p /home/user/workspace && \
    cd /home/user/workspace && \
    git clone https://github.com/chrcoen/systemc.git && \
    cd systemc && \
    git checkout trace_switching && \
    autoreconf && \
    mkdir -p build && \
    cd build && \
    ../configure && \
    make -j $(nproc) && \
    make install && \
    make clean && \
    cd .. && \
    mkdir -p build_debug && cd build_debug && \
    ../configure --enable-debug --disable-optimize --enable-pthreads --with-arch-suffix=-linux64-debug && \
    make -j $(nproc) && \
    make install && \
    make clean

