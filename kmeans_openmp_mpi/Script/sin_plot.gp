set terminal 'pdfcairo';
set output '../Pictures/sin.pdf';
set ylabel 'y';
set xlabel 'x';
plot '../Data/input_sin_d_2_k_100.csv' using 1:2 with dot title 'datapoints', \
	 '../Data/output.csv' using 1:2 title 'clusters';
