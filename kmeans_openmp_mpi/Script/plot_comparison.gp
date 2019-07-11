set terminal 'pdfcairo';
set output '../Pictures/comparison_new.pdf';
set ylabel 'y';
set xlabel 'x';
set datafile separator ","
plot '../Data/input_hard.txt' using 1:2 with dot title 'points', \
	 '../Data/output_hard.csv' using 1:2 title 'clusters found', \
	 '../Data/output_hard_correct.csv' using 1:2 title 'correct clusters';
