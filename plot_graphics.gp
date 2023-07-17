set terminal png size 800,600

set datafile separator ","

set title "File times vs. n"
set xlabel "n"
set ylabel "File times"

set yrange [0:750000]
set output "time_curves_r10_k2.png"
datafile = "csv_data/csv_data_r10_k2.csv"
plot datafile using 1:6 with points pt 7 lc rgb 'red' ps 1.5 title "Log file time", \
     datafile using 1:7 with points pt 7 lc rgb 'green' ps 1.5 title "Result file time", \
     datafile using 1:6 smooth csplines with lines lc rgb 'red' lw 2 notitle, \
     datafile using 1:7 smooth csplines with lines lc rgb 'green' lw 2 notitle

set yrange [0:400000]
set output "time_curves_r5_k1.png"
datafile = "csv_data/csv_data_r5_k1.csv"
plot datafile using 1:6 with points pt 7 lc rgb 'red' ps 1.5 title "Log file time", \
     datafile using 1:7 with points pt 7 lc rgb 'green' ps 1.5 title "Result file time", \
     datafile using 1:6 smooth csplines with lines lc rgb 'red' lw 2 notitle, \
     datafile using 1:7 smooth csplines with lines lc rgb 'green' lw 2 notitle

set yrange [0:4000]
set output "time_curves_r3_k0.png"
datafile = "csv_data/csv_data_r3_k0.csv"
plot datafile using 1:6 with points pt 7 lc rgb 'red' ps 1.5 title "Log file time", \
     datafile using 1:7 with points pt 7 lc rgb 'green' ps 1.5 title "Result file time", \
     datafile using 1:6 smooth csplines with lines lc rgb 'red' lw 2 notitle, \
     datafile using 1:7 smooth csplines with lines lc rgb 'green' lw 2 notitle