#!/bin/sh

testcases='exp(1)+pi+phi
(2*3)/(3-5)
((((5)))(3))
(e^pi)-pi
((2*phi)-1)^2
+
1.0e2-1
1.0e-3
1.0e*3
'

out=""
equals='='

for tc in $testcases; do
    out+="$tc = $(echo $tc | ./ce 2>&1)"
    out+=$'\n'
done

echo "$out" | column -t -s '=' -o '='
