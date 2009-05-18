#!/bin/bash

# Check that pngnq runs properly with the test images

IMAGES=./images/[!x]*[!nq8].png
BROKEN_IMAGES=./images/X*.png

pngnq -f ${IMAGES}

for IMAGE in ${IMAGES}
    do
        QUANTIMAGE=${IMAGE}
        QUANTIMAGE=${QUANTIMAGE/.png/-nq8.png}
        echo 
        echo "*********************************************"
        echo Comparing ${IMAGE} to ${QUANTIMAGE}
        pngcomp -b2 ${IMAGE} ${QUANTIMAGE}
        
        echo "*********************************************"
    done

rm images/*nq8.png

