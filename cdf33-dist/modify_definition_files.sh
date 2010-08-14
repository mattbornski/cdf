#! /bin/sh
search="<cdf_install_dir>"
replace=$1   # Current working directory

for file in `ls $replace/bin/definitions.*`
do
       echo "Modifying the definition file $file .."
       ed - $file << editend
       1,\$s:$search:$replace:g
       w
       q
editend
   done

