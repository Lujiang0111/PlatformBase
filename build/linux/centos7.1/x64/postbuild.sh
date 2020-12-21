base_path=$1
project=$2
version=$3
target=$4
dir=${base_path}/${project}/v${version}/linux/centos7.1/x64
rm -rf ${dir}
mkdir -p ${dir}/include/
mkdir -p ${dir}/lib/
rm -rf ${dir}/include/*
rm -rf ${dir}/lib/*

\cp -r ../../../../include/* ${dir}/include/
\cp -r ${target} ${dir}/lib/