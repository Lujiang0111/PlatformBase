#!/bin/sh
#$1第一个参数为比较的动态库名字
#$2第二个参数为需要的动态库名字
#return 1  -----match success
#return 0 	-----match failed
function check_version()
{
#严格匹配版本号
	if [ "${3}" == "true" ];then
		if [ "${1}" == "${2}" ];then
			return 1
		else
			echo -e "\033[31mError: Different open dynamic library version! There is $1 ,but we need $2 \033[0m"
			return 0
		fi
	fi
	
	local x1=`echo ${1} | cut -d '.' -f 1`
	local y1=`echo ${1} | cut -d '.' -f 2`
	#local z1=`echo ${1} | cut -d '.' -f 5`
	local x2=`echo ${2} | cut -d '.' -f 1`
	local y2=`echo ${2} | cut -d '.' -f 2`
	#local z2=`echo ${2} | cut -d '.' -f 5`
#主版本号不对
	if [ $x1 -ne $x2 ];then
		echo -e "\033[31mError: $name1 main version is wrong ! There is $1 ,but we need $2 \033[0m"
		return 0	
	else
#次版本号小于要求的次版本号
		if [ $y1 -lt $y2 ];then
			echo -e "\033[31mError: Dynamic library $name1 minor version is wrong ! There is $1 ,but we need $2 \033[0m"
			return 0
#此版本号大于要求的次版本号，提示
		elif [ $y1 -gt $y2 ];then
			echo -e "\033[31mWarning: Dynamic library $name1 minor version is too large ,maybe not compatible ! There is $1 ,but we need $2 \033[0m"
			return 1
		elif [ $y1 -eq $y2 ];then 
			#echo "$name1 version is correct "
			return 1
		fi
	fi
}

function copy_dependency()
{
	local i
	local array
	
	include_dst_path=../../../../deps/include
	lib_dst_path=../../../../deps/lib
	mkdir -p ${include_dst_path}
	mkdir -p ${lib_dst_path}
	array=($(echo "$@"))
	index=${#array[@]}
        	let index-=1
	release=${array[index]}
	if [ "${release}" != "release" ];then
        let index+=1
	else
		mkdir -p ../../../../bin/release
	fi
	for((i=1;i<${index};i+=5))
	do
		echo -e "\033[33m${array[i]} ${array[i+1]} ${array[i+2]} ${array[i+3]} ${array[i+4]}\033[0m"
		array1=`ls $1/${array[i]}/`
		res=0
		for name in ${array1}
		do
			version=`echo ${name} | cut -c 2-`	
			check_version ${version} ${array[i+1]} ${array[i+4]}
			if [ $? -eq 1 ];then
				let res=1
				finalversion=${version}
			fi			
		done
#路径上找到兼容的版本
		if [ ${res} -eq 1 ];then
			if [ "${release}" = "release" ];then
				src_base_path=$1/${array[i]}/v${finalversion}/linux/centos7.1/x64_release
				if [ ! -x "${src_base_path}" ];then
					echo -e "\033[33mWarning: do not find release lib in ${src_base_path}\033[0m"
					src_base_path=$1/${array[i]}/v${finalversion}/linux/centos7.1/x64
				fi
			else			
				src_base_path=$1/${array[i]}/v${finalversion}/linux/centos7.1/x64
			fi
			include_cpy_path=${src_base_path}/include
			lib_cpy_path=${src_base_path}/lib
			#copy include
			if [ "${array[i+2]}" = "true" ]; then
				mkdir -p ${include_dst_path}/${array[i]}
				\cp -r ${include_cpy_path}/* ${include_dst_path}/${array[i]}
			fi
			#copy lib
			if [ "${array[i+3]}" = "true" ]; then
				\cp -r ${lib_cpy_path}/* ${lib_dst_path}/
				for file in ${lib_cpy_path}/*.so.*
				do
					if [ "${file}" = "${lib_cpy_path}/*.so.*" ]; then
						:					
					else
						realname=`echo ${file} | rev |cut -d '/' -f 1 | rev`
						libname=`echo ${realname} |cut -d '.' -f 1`
						ln -sf ${realname} ${lib_dst_path}/${libname}.so						
						
					fi
				done
			fi
		fi
	done
}

rm -rf ../../../../bin
mkdir -p ../../../../bin
copy_dependency $*
