#!/bin/bash

#USER DEFINED VARIABLES
input="energies.txt" #note: requires line break at end in order to process the final entry as well
init_macro="macros/NDGAr.init.mac"
template_macro="macros/NDGAr_template.config.mac"
target_macro="macros/NDGAr.config.mac"
outdir="output"
executable="build/nexus"
n_events=10

#create output folder if does not exist
if [ -d "$outdir" ]; then
    echo "output folder: ${outdir}"
else
    echo "Creating output folder: ${outdir}... "
    mkdir $outdir
    echo "Done!"
fi
    
#Run batch
cat $input | while IFS="" read -r energy
do
    cp $template_macro $target_macro
    f_id=$(echo $energy | tr -d '[:blank:]')
    f_id=$(echo $f_id | tr -d '.')
    echo $OSTYPE
    if [ "$OSTYPE" = "linux-gnu" ]
    then
        sed -i "s/<outdir>/$outdir/g" $target_macro;
        sed -i "s/<energy>/$energy/g" $target_macro;
        sed -i "s/<f_id>/$f_id/g" $target_macro;  
    elif [ "$OSTYPE" = "darwin23" ]
    then
        # Mac OSX
        sed -i'' -e "s/<outdir>/$outdir/g" $target_macro;
        sed -i'' -e "s/<energy>/$energy/g" $target_macro;
        sed -i'' -e "s/<f_id>/$f_id/g" $target_macro;
    else
        echo "ERROR: Undefined OS!"
        echo "Please check for compatibility on your OS and then include your OS in run_batch.sh before continuing."
        echo "Leaving..."
        exit 1
    fi
    
    echo "Computing for engergy: $energy ..."
    ./$executable -b -n $n_events $init_macro
done
