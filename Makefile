SHELL:=/bin/bash

.DEFAULT_GOAL := all

ROOT_DIR:=$(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")
OUTPUT_DIRECTORY=${ROOT_DIR}/output
SUBMODULES_PATH?=${ROOT_DIR}

.EXPORT_ALL_VARIABLES:
DOCKER_BUILDKIT?=1
DOCKER_CONFIG?=
ARCH?=$(shell uname -m)
DOCKER_PLATFORM?=linux/$(ARCH)
CROSS_COMPILE?=$(shell if [ "$(shell uname -m)" != "$(ARCH)" ]; then echo "true"; else echo "false"; fi)

PROJECT:=optinlc
TAG := $(shell git rev-parse --short HEAD)_${ARCH}

.PHONY: show-hash
show-hash:
	@echo "Git Short Hash: $(GIT_SHORT_HASH)"



.PHONY: all
all: build

.PHONY: init_submodules
init_submodules:
ifeq ($(wildcard ${SUBMODULES_PATH}/mathematics_toolbox/*),)
	git submodule update --init mathematics_toolbox
else
	@echo "Submodules already initialized, skipping submodule init."
endif

.PHONY: check_cross_compile_deps
check_cross_compile_deps:
	@if [ "$(CROSS_COMPILE)" = "true" ]; then \
        echo "Cross-compiling for $(ARCH) on $(shell uname -m)"; \
        if ! which qemu-$(ARCH)-static >/dev/null || ! docker buildx inspect $(ARCH)builder >/dev/null 2>&1; then \
            echo "Installing cross-compilation dependencies..."; \
            sudo apt-get update && sudo apt-get install -y qemu qemu-user-static binfmt-support; \
            docker run --privileged --rm tonistiigi/binfmt --install $(ARCH); \
            if ! docker buildx inspect $(ARCH)builder >/dev/null 2>&1; then \
                docker buildx create --name $(ARCH)builder --driver docker-container --use; \
            fi; \
        fi; \
    fi

.PHONY: build_mathematics_toolbox
build_mathematics_toolbox:
	cd "${SUBMODULES_PATH}/mathematics_toolbox" && make build

.PHONY: build
build: clean init_submodules build_mathematics_toolbox _build

.PHONY: _build
_build: check_cross_compile_deps
	@if [ "$(CROSS_COMPILE)" = "true" ]; then \
        echo "Cross-compiling ${PROJECT}:${TAG} for $(ARCH)..."; \
        docker buildx build --platform $(DOCKER_PLATFORM) \
                 --tag ${PROJECT}:${TAG} \
                 --build-arg ARCH=${ARCH} \
                 --build-arg PROJECT=${PROJECT} \
                 --load .; \
    else \
        docker build --network host \
                 --tag ${PROJECT}:${TAG} \
                 --build-arg ARCH=${ARCH} \
                 --build-arg PROJECT=${PROJECT} .; \
    fi
	docker cp $$(docker create --rm ${PROJECT}:${TAG}):/tmp/OptiNLC/OptiNLC/build "${ROOT_DIR}/OptiNLC"

.PHONY: test
test: build
	mkdir -p ${OUTPUT_DIRECTORY}
	docker run -t -v ${OUTPUT_DIRECTORY}:/tmp/output --platform $(DOCKER_PLATFORM) ${PROJECT}:${TAG} /bin/bash -c 'cd /tmp/output && /tmp/OptiNLC/OptiNLC/build/OptiNLC_TestRunner -d yes'


.PHONY: plot
plot: 
	gnuplot eigen_plot.gnuplot

.PHONY: run
run: build
	docker run -it --platform $(DOCKER_PLATFORM) ${PROJECT}:${TAG} /tmp/OptiNLC/OptiNLC/build/OptiNLC

.PHONY: clean
clean:  ## Clean OptiNLC build artifacts 
	rm -rf "${ROOT_DIR}/${PROJECT}/build"
	rm -rf "${OUTPUT_DIRECTORY}"
	docker rm $$(docker ps -a -q --filter "ancestor=${PROJECT}:${TAG}") --force 2> /dev/null || true
	docker rmi $$(docker images -q ${PROJECT}:${TAG}) --force 2> /dev/null || true
	docker rmi --force $$(docker images --filter "dangling=true" -q --no-trunc) 2> /dev/null || true

