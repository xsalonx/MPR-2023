./r.sh --sub_m 200 -m 2 -s 2 -S 4 --out small-even-div;
./r.sh --sub_m 200 -m 2 -s 2 -S 4 --out small-even-div --num_threads_div_proportional;


./r.sh --sub_m 200 -m 1 -s 1 -S 4 --out small-even-odd-div ;
./r.sh --sub_m 200 -m 1 -s 1 -S 4 --out small-even-odd-div --num_threads_div_proportional;


./r.sh --sub_m 200 -m 2 -s 2 -S 8 --out big-even-div;
./r.sh --sub_m 200 -m 2 -s 2 -S 8 --out big-even-div --num_threads_div_proportional;

./r.sh --sub_m 200 -m 1 -s 2 -S 5 --out big-odd-div;
./r.sh --sub_m 200 -m 1 -s 2 -S 5 --out big-odd-div --num_threads_div_proportional;
