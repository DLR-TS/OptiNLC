ARG PROJECT
ARG ARCH=x86_64

FROM eigen3:latest_${ARCH} AS eigen3 
FROM osqp:latest_${ARCH} AS osqp 

FROM debian:bookworm-slim AS optinlc_requirements_base

ARG PROJECT
ARG REQUIREMENTS_FILE="requirements.OptiNLC.system"


RUN mkdir -p /tmp/OptiNLC
WORKDIR /tmp/OptiNLC
COPY ${REQUIREMENTS_FILE} /tmp/OptiNLC

ENV DEBIAN_FRONTEND=noninteractive

ENV DEBIAN_FRONTEND=noninteractive
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    apt-get update && \
    apt-get install --no-install-recommends -y checkinstall && \
    xargs apt-get install --no-install-recommends -y < ${REQUIREMENTS_FILE} && \
    rm -rf /var/lib/apt/lists/*

COPY OptiNLC /tmp/OptiNLC/OptiNLC

COPY --from=eigen3 /tmp/eigen3 /tmp/eigen3 
WORKDIR /tmp/eigen3/build
RUN make install

COPY --from=osqp /tmp/osqp /tmp/osqp 
WORKDIR /tmp/osqp/build
RUN dpkg -i osqp*.deb 

WORKDIR /tmp/OptiNLC
COPY *.txt .


FROM optinlc_requirements_base AS optinlc_builder

ARG PROJECT
WORKDIR /tmp/OptiNLC/OptiNLC
RUN mkdir -p build 
SHELL ["/bin/bash", "-c"]
WORKDIR /tmp/OptiNLC/OptiNLC/build

RUN cmake .. && \
    cmake --build .  --config Release --target install -- -j $(nproc) && \
    cpack -G DEB && find . -type f -name "*.deb" | xargs mv -t . && \
    cd /tmp/OptiNLC/OptiNLC/build && ln -s devel install && \
    mv CMakeCache.txt CMakeCache.txt.build  && \
    rm -rf "sqp*.deb"
