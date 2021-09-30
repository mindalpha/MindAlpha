#!/usr/bin/env bash
function build_images() {
    local name=$1
    docker build --no-cache -f docker/ubuntu20.04/Dockerfile -t $name .
    #docker build  -f docker/ubuntu20.04/Dockerfile -t $name .
}
function build_mindalpha() {
    local image_name=$1
    local running_image_name=$2
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
    docker exec -t -w ${build_dir} ${running_image_name} /bin/bash \
               -c "source ~/.bashrc && cd $l_base_dir && bash compile.sh"
}
function print_help() {
    echo "usage run.sh -n tagname -i(build_images) -m(build_indalpha) -h(help)"
    exit -1
}
tags_name="ubuntu-mindalpha:v1.0"
user=$(whoami)
images_name=$(echo $tags_name | sed 's/:/-/g')
running_image_name=$user-$images_name-env
function main() {
    images=0
    mindalpha=0
	while getopts n:bimh OPTION
    do
        case ${OPTION} in  
        h)
            print_help
            ;;
        i)
            images=1
            ;;
        m)
            mindalpha=1
            ;;
        n) tags_name=${OPTARG}
            ;;
    	esac  
    done
    if [[ $images -eq 1 ]]; then
        build_images $tags_name
    fi
    if [[ $mindalpha -eq 1 ]]; then
        build_mindalpha $tags_name $running_image_name
    fi
}
main $*
