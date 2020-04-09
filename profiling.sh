#!/bin/bash

ns=1234

list=
kernel_list=
kernel_vals=
i=0
#GPU kernels
for g in copy program lambda closure function function_lambda function_macro program_template program_T4 program_T4xyzw program_T4ls_fma
do
  ./process_sequential -s 8192 -o sample_sequential.nc -r result_sequential.nc --generator-factory count -n $ns --use-GPU --GPU-factory $g
  #gather profiling infos
  ncks -A profiling_gpu.nc profiling_process.nc
  fo=profiling_GPU_$g.nc
  mv profiling_process.nc $fo
  #appending file to list
  list=$list" "$fo

  #tune (optional)
  ##add kernel_# attribute
  kernel=`ncdump -h $fo | grep iteration:kernel | cut -d'"' -f2`
  kernel_vals=$kernel_vals', "'$i'_'$g'"'
  ###all vars
  ncatted -a kernel_name,,c,c,$i'_'$g $fo
  ###global
  ncatted -a kernel_$i,global,c,c,$i'_'$g'_'$kernel $fo
  ###global list for ncecat
  kernel_list=$kernel_list" --glb_att_add kernel_${i}=${i}_${g}_$kernel"
  ##cleaning
  ft=`basename $fo .nc`.tmp.nc
  ncwa --history -O -a dim1,dimP $fo $ft
  ncatted -O -a cell_methods,,d,, $ft $fo
  rm $ft

  #next iteration
  ((++i))
done

#kernel name var
fk=kernels.cdl
/bin/echo -e -n 'netcdf kernels\n{\ndimensions:\n\n	kernel = ' > $fk
/bin/echo -e -n $i >> $fk
/bin/echo -e -n '; //UNLIMITED ;\n	dimString=128;\nvariables:\n	char kernel(kernel, dimString);\ndata:\n	kernel = ' >> $fk
/bin/echo -e -n $kernel_vals | sed 's/, //' >> $fk
/bin/echo -e -n ';\n}//kernels' >> $fk
ncgen $fk -o kernels.nc

#ensemble cat
fo=profiling_GPU.nc
ncecat -O $list $kernel_list -o $fo
ncrename -d record,kernel $fo
ncatted -a kernel,,d,, -a kernel_name,,d,, -a kernel_0,,d,, $fo
ncks -A kernels.nc $fo


#exit

#show header
ncdump -v kernel $fo
