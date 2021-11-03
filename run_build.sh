#!/usr/bin/env bash

function build_images() {
    local name=$1
    local images_no_cache=$2
    local build_for_centos=$3
    if [[ $build_for_centos -eq 1 ]]; then
        build_dir=docker/centos7
    else
        build_dir=docker/ubuntu20.04
    fi
    dockerfile=$build_dir/Dockerfile
    docker build $images_no_cache -f $dockerfile -t $name $build_dir
}

function build_mindalpha() {
    local image_name=$1
    local running_image_name=$2
    local package_mindalpha=$3
    docker ps | grep $running_image_name
    if [[ $? -ne 0 ]]; then
        docker run -dt  --net=host --name $running_image_name \
            --cap-add=SYS_PTRACE --cap-add=SYS_NICE --security-opt seccomp=unconfined \
            -e TERM=xterm-256color -e COLUMNS="`tput cols`" -e LINES="`tput lines`" \
            -v /home:/home \
            $image_name
    else
        docker start $running_image_name
    fi
    l_base_dir=$(pwd)
    build_dir="${l_base_dir}/build"
    if [[ ! -d $build_dir ]]; then
        mkdir $build_dir
    fi
    if [[ $package_mindalpha -eq 1 ]]; then
        docker exec -t -w ${build_dir} ${running_image_name} /bin/bash \
                    -c "source ~/.bashrc && cd $l_base_dir && bash compile.sh && bash package.sh"
    else
        docker exec -t -w ${build_dir} ${running_image_name} /bin/bash \
                    -c "source ~/.bashrc && cd $l_base_dir && bash compile.sh"
    fi
}

function print_help() {
    echo "usage $0 -n tagname -u usertag -i(build_images) -c(for_centos) -C(no_cache) -m(build_mindalpha) -p(package_mindalpha) -h(help)"
    exit -1
}

function main() {
    default_ubuntu_tags_name="mindalpha-build-ubuntu20.04:v1.0"
    default_centos_tags_name="mindalpha-build-centos7:v1.0"
    tags_name=""
    user_tag=$(whoami)

    images=0
    build_for_centos=0
    images_no_cache=""
    build_mindalpha=0
    package_mindalpha=0

    while getopts nu:icCmph OPTION
    do
        case ${OPTION} in
        h)
            print_help
            ;;
        i)
            images=1
            ;;
        c)
            build_for_centos=1
            ;;
        C)
            images_no_cache="--no-cache"
            ;;
        m)
            build_mindalpha=1
            ;;
        p)
            package_mindalpha=1
            ;;
        n)
            tags_name=${OPTARG}
            ;;
        u)
            user_tag=${OPTARG}
            ;;
        esac
    done
    if [[ -z "$tags_name" ]]; then
        if [[ $build_for_centos -eq 1 ]]; then
            tags_name=$default_centos_tags_name
        else
            tags_name=$default_ubuntu_tags_name
        fi
    fi
    images_name=$(echo $tags_name | sed 's/:/-/g')
    running_image_name=$user_tag-$images_name-env
    if [[ $images -eq 1 ]]; then
        build_images $tags_name "$images_no_cache" $build_for_centos
    fi
    if [[ $build_mindalpha -eq 1 ]]; then
        build_mindalpha $tags_name $running_image_name $package_mindalpha
    fi
}

main $*
