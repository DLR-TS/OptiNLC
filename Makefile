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




MATHEMATICS_TOOLBOX_SHORT_HASH:=$(shell cd ./mathematics_toolbox && git rev-parse --short HEAD)
OSQP_PROJECT=osqp
OSQP_TAG:=${MATHEMATICS_TOOLBOX_SHORT_HASH}_${ARCH}
OSQP_IMAGE=${OSQP_PROJECT}:${OSQP_TAG}

EIGEN3_PROJECT=eigen3
EIGEN3_TAG:=${MATHEMATICS_TOOLBOX_SHORT_HASH}_${ARCH}
EIGEN3_IMAGE=${EIGEN3_PROJECT}:${EIGEN3_TAG}




SHORT_HASH:=$(shell git rev-parse --short HEAD)
DOCKER_REPOSITORY="ghcr.io/dlr-ts/optinlc"

PROJECT:=optinlc
TAG:=${SHORT_HASH}_${ARCH}
IMAGE=${PROJECT}:${TAG}
OPTINLC_IMAGE:=${IMAGE}
IMAGE_PUBLISH=${DOCKER_REPOSITORY}:${PROJECT}_${TAG}

DOCKER_CACHE_DIRECTORY="${ROOT_DIR}/.docker_cache"
DOCKER_ARCHIVE="${DOCKER_CACHE_DIRECTORY}/optinlc_${TAG}.tar"


.PHONY: show-hash
show-hash:
	@echo "Git Short Hash: $(GIT_SHORT_HASH)"

.PHONY: all
all:
	make load_docker_images
	make docker_pull_fast
	make build_fast
	make save_docker_images



.PHONY: clean_build
clean_build: clean init_submodules build_mathematics_toolbox _build

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
build: all 

.PHONY: _build
_build: check_cross_compile_deps
	rm -rf build
	cd mathematics_toolbox && make all
	@if [ "$(CROSS_COMPILE)" = "true" ]; then \
        echo "Cross-compiling ${PROJECT}:${TAG} for $(ARCH)..."; \
        docker buildx build \
            --builder=default \
            --platform=$(DOCKER_PLATFORM) \
            --tag ${PROJECT}:${TAG} \
            --build-arg ARCH=$(ARCH) \
            --build-arg EIGEN3_TAG=$(EIGEN3_TAG) \
            --build-arg OSQP_TAG=$(OSQP_TAG) \
            --build-arg PROJECT=$(PROJECT) \
            --load .; \
    else \
        docker build --network host \
            --tag ${PROJECT}:${TAG} \
            --build-arg ARCH=$(ARCH) \
            --build-arg EIGEN3_TAG=$(EIGEN3_TAG) \
            --build-arg OSQP_TAG=$(OSQP_TAG) \
            --build-arg PROJECT=$(PROJECT) .; \
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

.PHONY: build_fast
build_fast:
	@if [ -n "$$(docker images -q ${PROJECT}:${TAG})" ]; then \
        echo "Docker image: ${PROJECT}:${TAG} already build, skipping build."; \
    else \
        make _build;\
    fi
	docker cp $$(docker create --rm ${PROJECT}:${TAG}):/tmp/OptiNLC/OptiNLC/build "${ROOT_DIR}/OptiNLC"

.PHONY: docker_pull_fast
docker_pull_fast:
	@[ -n "$$(docker images -q ${IMAGE})" ] || make pull


.PHONY: clean
clean:  ## Clean OptiNLC build artifacts 
	rm -rf "OptiNLC/build"
	rm -rf "${OUTPUT_DIRECTORY}"
	cd mathematics_toolbox && make clean
	docker rm $$(docker ps -a -q --filter "ancestor=${PROJECT}:${TAG}") --force 2> /dev/null || true
	docker rmi $$(docker images -q ${PROJECT}:${TAG}) --force 2> /dev/null || true
	docker rmi --force $$(docker images --filter "dangling=true" -q --no-trunc) 2> /dev/null || true


.PHONY: push
push: docker_push

.PHONY: docker_push
docker_push:
	docker tag "${IMAGE}" "${IMAGE_PUBLISH}"
	docker push "${IMAGE_PUBLISH}"

.PHONY: pull
pull: docker_pull

.PHONY: docker_pull
docker_pull:
	docker pull "${IMAGE_PUBLISH}" || true
	docker tag "${IMAGE_PUBLISH}" "${IMAGE}" || true
	docker rmi "${IMAGE_PUBLISH}" || true

.PHONY: save_docker_images
save_docker_images:
	mkdir -p "${DOCKER_CACHE_DIRECTORY}"
	@if [ -w "$$(dirname "${DOCKER_ARCHIVE}")" ]; then \
        rm -rf "${DOCKER_ARCHIVE}"; \
        echo "Saving Docker images to ${DOCKER_ARCHIVE}..."; \
        docker save -o "${DOCKER_ARCHIVE}" ${OSQP_IMAGE} ${EIGEN3_IMAGE} ${OPTINLC_IMAGE}; \
    else \
        echo "WARNING: Directory $$(dirname "${DOCKER_ARCHIVE}") is not writable. Unable to save docker images, skipping saving of docker images."; \
    fi

.PHONY: load_docker_images
load_docker_images:
	@docker load --input "${DOCKER_ARCHIVE}" 2>/dev/null || true


