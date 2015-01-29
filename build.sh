#!/bin/bash

############################################################
#
# Author: Rasmus Zakarias
# Project: Umbrella
# Date: 27. Nov 2014
# 
# Description: The Umbrella project is defined by this file: One
# umbrella script to build everything :D.
#
# Well in fact it depends on the deptree.sh file to describe the build
# order to resolve inter-project dependencies.
#
# Usage:
#
# build.sh [<project name>] - where <project name> is the name of the
# sub-autotool project to build. Giving no argument builds everything.
#
############################################################

BUILDDIR=${PWD}/sys
MAKE_SCRIPTS=$(find . -type f | grep -v .svn  | grep -e "linux.mak$")

COLOR_NORM="\033[0m"
COLOR_GREEN="\033[1;32m"
COLOR_WHITE="\033[1;38m"
COLOR_RED="\033[1;31m"
COLOR_YELLOW="\033[1;33m"

if [ ! -f /bin/bash ]; 
then
 echo "Error: This script requires Bash, sh does not cut it. In particular we need /bin/bash"
 exit -1;
fi

#
# Note build time in seconds since EPOCH
#
START=$(date +%s)

theend() {
 echo "Umberlla build script ran for " $(( $(date +%s) - ${START})) " seconds."
 exit ${1}
}

put() {
    eval hash"$1"='$2';
}

get() {
    eval echo '${hash'"\"${1}\""'#hash}';
}

print_start() {
    printf "${COLOR_WHITE}%-50s %s${COLOR_NORM}" ${1}
}

print_ok() {
    echo -e "${COLOR_WHITE} [ ${COLOR_GREEN} OK ${COLOR_NORM}${COLOR_WHITE} ] ${COLOR_NORM}"
}

print_fail() {
    echo -e "${COLOR_WHITE} [ ${COLOR_RED}FAIL${COLOR_NORM}${COLOR_WHITE} ] ${COLOR_NORM}"
}

print_skip() {
    echo -e "${COLOR_WHITE} [ ${COLOR_YELLOW}SKIP${COLOR_NORM}${COLOR_WHITE} ] ${COLOR_NORM}"
}

helper_assign_priority() {
    echo $(for mkfile in ${MAKE_SCRIPTS}; do
	PROJ=${mkfile}
	P=$(echo ${PROJ} | grep -e ${1})
	if [ "${P}" != "" ]; then
	    echo -n -e "${R}\n${2}${PROJ}"
	else
	    echo -n -e "${R}\n${PROJ}"
	fi
    done  | sort )
}

set_swl() {
    MAKE_SCRIPTS=$(helper_assign_priority ${1} ${2})
}

#
# Build in the {linux} folder with a {liunx.mak} file, afterwards do a
# {make install}.
#
# \param ${1} - current directory
#
# \param ${2} - configuration (debug/release)
#
# \return - this function prints "1" on success "0" otherwise.
#
build_dir() {
    CURDIR=${1}
    CONF=${2}
    pushd ${CURDIR} 2>&1 > /dev/null
    OUT1=$(make -f linux.mak BUILDDIR=${BUILDDIR} 2>&1)
    FST=$?
    OUT2=$(make install 2>&1)
    SND=$?

    if [[ "${FST}" == "0" && "${SND}" == "0" ]]; then
	echo "1"
	echo "" > build.log	
    else

	echo "${OUT2}" >> build.log
	echo "0"
    fi

    popd 2>&1 > /dev/null
}


#
#
#
clean_dir() {
    CURDIR=${1}
    pushd ${CURDIR} 2>&1 > /dev/null
    OUT=$(make --silent -f linux.mak clean 2>&1)
    FST=$?
    if [[ "$?" == "0"  ]]; then
	echo "1"
    else
	echo "0"
    fi
    popd 2>&1 > /dev/null
}


#------------------------------------------------------------
# 
# Build all linux.mak files.
#
# For each make script called "linux.mak" in the sub-directories we
# change directory to where that file is make. However, if there is a
# filter list and the current directory is not on that list we will
# skip it.
#
#
# \param ${1} - build/clean action 
#
# \param ${2} - release/debug configuration
#
#------------------------------------------------------------
build_all_linuxdotmak() {
    . deptree.sh
    ALL_OK="1"
    for mkfile in ${MAKE_SCRIPTS}; do
	OK="1"
	CURDIR=$(dirname ${mkfile:2});
	if [ "${FILTER}" == "" ]; then
	    print_start ${CURDIR:0:${#CURDIR}-6};
	    case ${1} in
		"build")
		    OK=$(build_dir ${CURDIR} ${2})
		    ;;
		"clean")
		    OK=$(clean_dir ${CURDIR})
		    ;;
	    esac

	    if [ "${OK}" == "0" ]; then
		print_fail;
		ALL_OK="0";
	    else
		print_ok;
	    fi
	else
	    B="0"
	    for proj in ${FILTER}; do
		if [ "$(echo ${mkfile} | grep ${proj})" != "" ]; then
		    case ${1} in
			"build")
			    OK=$(build_dir ${CURDIR} ${2})
			    ;;
			"clean")
			    OK=$(clean_dir ${CURDIR})
			    ;;
		    esac
		    B="1"
		fi;
	    done
	    
	    if [ "${B}" != "0" ]; then
		print_start ${CURDIR:0:${#CURDIR}-6};
		if [ "${OK}" == "0" ]; then
		print_fail;
		ALL_OK="0";
		else
		    print_ok;
		fi
	    fi
	fi
    done
    
    [ "${ALL_OK}" == "1" ] && theend 0 || theend -1
}

info_proj() {
    FOUND="0"
    echo -e "Information about project ${1}:\n\n"
    if [ "${1}" == "" ]; then
	echo "Please state the name of an existing project also.";
	exit -1;
    fi

    for mkfile in ${MAKE_SCRIPTS}; do
	if [ "$(echo ${mkfile} | grep ${1})" != "" ]; then
	    FOUND="${mkfile}";
	fi;
    done
    
    if [ "${FOUND}" != "0" ]; then
	README=$(dirname ${FOUND})/../README
	if [ -f ${README} ]; then
	    cat ${README}
	else
	    echo "Project ${1} is not documented please write \"${README}\".";
	fi
    else
	echo "Project ${1} does not exists.";
    fi
    echo -e "\n\n "
}


############################################################
#
# Script that provides Eclipse project support
#
############################################################

#TODO(rwz): Add the content needed to create an Eclipse CDT project from automake like on OSX
BASE64_PROJ_TEMPLATE_ZIP_LIN_LUNA=""
#TODO(rwz): Add the content needed to create an Eclipse CDT project from automake like on OSX
BASE64_PROJ_TEMPLATE_ZIP_WIN_LUNA=""
BASE64_PROJ_TEMPLATE_ZIP_OSX_LUNA="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\nB1QzQSplHvoX8ESSeQSrLZK/EdSGqGZ07G6IvlGKRnNO4csRmxu32Qq53o13M7R/3XVdt+/edvbz\nePqwEYlGUshwOWzNzgXidFnblQKOW/MnlxqGOUZtysEl64kvhydd/zcSedwR7ExerVblz+yi1+pX\nPyhPsHPfL/6G96wPGlQaNqe+Z1Yh3xSPgB9QSwMEFAAAAAgAW1OIRf1A6IdnAAAAqwAAADgAHAAu\nc2V0dGluZ3Mvb3JnLmVjbGlwc2UubGludXh0b29scy50b29scy5sYXVuY2guY29yZS5wcmVmc1VU\nCQAD3W6FVPxuhVR1eAsAAQT1AQAABBQAAABLTc7JLChO1SsoSk1LLUrNS04t1itLLSrOzM+zNeTK\nL0rXS4WqyMnMK60oyc/PKdaDkDmJpXnJGXrJ+UWpej5wyYDEkgxbG0VdXWXlAiBTV9eOHFOCK4tL\nUnNd88rAxqUl5hSncgEAUEsBAh4DFAAAAAgA4EiIRat5lMA/AQAAmQMAAAgAGAAAAAAAAQAAAKSB\nAAAAAC5wcm9qZWN0VVQFAAMkXIVUdXgLAAEE9QEAAAQUAAAAUEsBAh4DFAAAAAgAKVOIRUAKx1UZ\nBgAAdBgAAAkAGAAAAAAAAQAAAKSBgQEAAC5jcHJvamVjdFVUBQADfm6FVHV4CwABBPUBAAAEFAAA\nAFBLAQIeAwoAAAAAAGtTiEUAAAAAAAAAAAAAAAAKABgAAAAAAAAAEADtQd0HAAAuc2V0dGluZ3Mv\nVVQFAAP5boVUdXgLAAEE9QEAAAQUAAAAUEsBAh4DFAAAAAgA61KIRRDTekShAQAA4AMAAB8AGAAA\nAAAAAQAAAKSBIQgAAC5zZXR0aW5ncy9sYW5ndWFnZS5zZXR0aW5ncy54bWxVVAUAAwluhVR1eAsA\nAQT1AQAABBQAAABQSwECHgMUAAAACAAnRohFfR0R0sMAAABhAgAAJAAYAAAAAAABAAAApIEbCgAA\nLnNldHRpbmdzL29yZy5lY2xpcHNlLmNkdC5jb3JlLnByZWZzVVQFAAP5V4VUdXgLAAEE9QEAAAQU\nAAAAUEsBAh4DFAAAAAgAW1OIRf1A6IdnAAAAqwAAADgAGAAAAAAAAQAAAKSBPAsAAC5zZXR0aW5n\ncy9vcmcuZWNsaXBzZS5saW51eHRvb2xzLnRvb2xzLmxhdW5jaC5jb3JlLnByZWZzVVQFAAPdboVU\ndXgLAAEE9QEAAAQUAAAAUEsFBgAAAAAGAAYAOgIAABUMAAAAAA==\n"

exec_test() {
    echo $(command -v ${1})
}

check_dep_programs() {
    if [ "$(exec_test base64)" == "0" ]; 
    then
	echo "base64 command does not exists as required."
	exit -1;
    fi

    if [ "$(exec_test unzip)" == "0" ]; 
    then
	echo "unzip command does not exists as required."
	exit -1;
    fi

   if [ "$(exec_test sed)" == "0" ]; 
    then
	echo "sed command does not exists as required."
	exit -1;
    fi

   if [ "$(exec_test realpath)" == "0" ]; 
    then
	echo "realpath command does not exists as required."
	exit -1;
    fi

}

test_dir() {

    if [ ! -d ${1}/linux ]; then
	echo "Error: ${1} does not look like an Umbrella automake project."
	exit -1;
    fi

    if [ ! -f ${1}/linux/linux.mak ]; 
    then
	echo "Error: ${1} does not look like an Umbrella automake project."
	exit -1;
    fi
}

diewarn_if_dir_exists() {
  if [ -d ${1} ]; then
      echo "Warning: Directory already exists in ${1}."
      exit -1;
  fi
}

replace_pattern() {
    pattern=${1};
    text=${2}
    file=${3}
    target=${4}
    echo "TARGET: ${target}"
    cat ${file} | sed 's|<\!--###'${pattern}'###-->|'${text}'|g' > ${target};
    if [ "$?" != "0" ]; then
	echo "Warning: Creating file ${target} resulted in non-zero return code.";
    fi
}

#
# ${1} - the path of the project in automake 
#
make_eclipse_project() {
  path=${1};
  if [ "${path}" == "" ];
  then
      echo "${0}: <path to Unbrella-automake project>";
      exit -1;
  fi

  check_dep_programs

  test_dir ${path}

  path=$(realpath ${path})

  name=$(basename ${path})
  target_eclipse_dir=${path}/../${name}_eclipse_proj
  build_path="${path}/linux"
  temp_file="/tmp/eclipse.proj.zip.b64"

  diewarn_if_dir_exists ${target_eclipse_dir}
  
  mkdir -p ${target_eclipse_dir}
  echo -e ${BASE64_PROJ_TEMPLATE_ZIP_OSX_LUNA} | base64 -d > ${temp_file}
  
  unzip ${temp_file} -d ${target_eclipse_dir}
  rm -f ${temp_file}
  
  replace_pattern "name" ${name} "${target_eclipse_dir}/.project" ${target_eclipse_dir}/.project1
  replace_pattern "path" ${path} "${target_eclipse_dir}/.project1" ${target_eclipse_dir}/.project
  rm -f ${target_eclipse_dir}/.project1

  replace_pattern "path" ${build_path}  ${target_eclipse_dir}/.cproject ${target_eclipse_dir}/.cproject1
  mv ${target_eclipse_dir}/.cproject1 ${target_eclipse_dir}/.cproject

}
############################################################
#
# END of Eclipse project support
#
############################################################



############################################################
#
# New Project
#
############################################################


temp_zip_file=/tmp/umbrella.project.zip

PROJECT_TEMPLATE_ZIP=`cat << _EOF
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
BQYAAAAAEQARAEgFAADLXQAAAAA=`

CODENAME_PTRN="<!--###codename###-->"
PROJNAME_PTRN="<!--###projname###-->"
MAINFILE_PTRN="<!--###mainfile###-->"

new_project() {
echo -n "Project path with basename as wanted project-name> "
read projectpath
echo -n "Project codename> "
read codename
echo -n "mainfile> "
read mainfile

projectname=$(basename ${projectpath})

echo -n "will create directory ${projectpath} with project ${projectname} continue [Y/n]? " 
read ok

if [ "${ok}" == "" ]; 
then 
 ok="Y";
fi

if [ "${ok}" == "Y" ];then
echo -e "${PROJECT_TEMPLATE_ZIP}" | base64 -d > ${temp_zip_file}
if [ -d ${projectpath} ];
then
echo "Error: project in ${projectpath} already exists. Please remove first."
exit -1
fi 
mkdir -p ${projectpath}
unzip ${temp_zip_file} -d ${projectpath} 
rm -f ${temp_zip_file}

## FILES ##
# template.am -> <!-###projname##-->.am
# configure.am replace
# src/Makefile.am replace
# test/Makefile.am replace
# include/Makefile.am replace
# touch <!--###mainfile###-->.c

mv ${projectpath}/template.am ${projectpath}/${projectname}.am
echo ${projectpath}/configure.ac ${projectpath}/configure.ac1
replace_pattern "projname" ${projectname} ${projectpath}/configure.ac ${projectpath}/configure.ac1
rm -f ${projectpath}/configure.ac
replace_pattern "codename" ${codename}    ${projectpath}/configure.ac1 "${projectpath}/configure.ac"
rm -f ${projectpath}/configure.ac1
echo ${projectpath}/configure.ac ${projectpath}/configure.ac1
replace_pattern "mainfile" "src/${mainfile}"  "${projectpath}/configure.ac" "${projectpath}/configure.ac1"
mv ${projectpath}/configure.ac1 ${projectpath}/configure.ac

replace_pattern "projname" "${projectname}" "${projectpath}/src/Makefile.am" "${projectpath}/src/Makefile.am1"
rm -f ${projectpath}/src/Makefile.am

replace_pattern "mainfile" "${mainfile}"    "${projectpath}/src/Makefile.am1" "${projectpath}/src/Makefile.am"
rm -f ${projectpath}/src/Makefile.am1

replace_pattern "projname" "${projectname}" "${projectpath}/test/Makefile.am" "${projectpath}/test/Makefile.am1"
mv ${projectpath}/test/Makefile.am1 ${projectpath}/test/Makefile.am

replace_pattern "mainfile" "${mainfile}" "${projectpath}/include/Makefile.am" "${projectpath}/include/Makefile.am1"
mv ${projectpath}/include/Makefile.am1 ${projectpath}/include/Makefile.am

echo -e "#include <${mainfile}.h>\nstatic char * libname() { return \"${projectname}\"; } " > ${projectpath}/src/${mainfile}.c
touch ${projectpath}/include/${mainfile}.h
fi 
}
############################################################
#
#
#
############################################################

echo "############################################################"
echo "# Umbrella build script system                             #"
echo "#                                                          #"
echo "# Available commands are:                                  #"
echo "#  new <proj_path> create a new project, e.g platform/time.#"
echo "#  build [proj_1,...,proj_n] build projects listed.        #"
echo "#  clean [proj_1,...,proj_n] clean projects listen.        #"
echo "#  eclipse [proj1,...,proj_n] build Eclipse proj.          #"
echo "#  info <proj> describe project <proj>.                    #"
echo "#                                                          #"
echo "############################################################"

CMD=$1
shift
FILTER=${@}

case $CMD in
    "new")
	new_project ${1}
	;;
    "debug")
	if [ "$FILTER" == "" ]; then
	    echo "Building all projects in ${BUILDDIR}: "
	else
	    echo "Building project(s) in ${BUILDDIR}: ${@}"
	fi
	build_all_linuxdotmak "build" "debug";
	;;
    "build") 
	if [ "$FILTER" == "" ]; then
	    echo "Building all projects in ${BUILDDIR}: "
	else
	    echo "Building project(s) in ${BUILDDIR}: ${@}"
	fi
	build_all_linuxdotmak "build" "release";

	;;
    "clean")
	if [ "$FILTER" == "" ]; then
	    echo "Cleaning all projects: "
	    echo "${BUILDDIR} nuked !"
	    rm -rf ${BUILDDIR}
	else
	    echo "Cleaning project(s): ${@}"
	fi
	build_all_linuxdotmak "clean";
	;;
    "eclipse")
	make_eclipse_project ${1};
	;;
    "info")
	echo "$(info_proj $1)";
	;;
    "extract_eclipse")
	$(echo -e ${BASE64_PROJ_TEMPLATE_ZIP_OSX_LUNA} | base64 -d > eclipse.proj.zip)
	;;
    "")
	echo "Please pick a command from the set above."
	;;
    *)
	echo "Unfortunately your ${CMD} command is unknown";
esac

