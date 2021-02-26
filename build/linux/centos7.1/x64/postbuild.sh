base_path=$1
project=$2
version=$3
target=$4

if [ $# -ge 5 ]; then
dir=${base_path}/${project}/v${version}/linux/centos7.1/x64_release
else
dir=${base_path}/${project}/v${version}/linux/centos7.1/x64
fi

mkdir -p ${dir}/include/
mkdir -p ${dir}/lib/
rm -rf ${dir}/include/*
rm -rf ${dir}/lib/*

\cp -r ../../../../include/* ${dir}/include/
\cp -r ${target} ${dir}/lib/
