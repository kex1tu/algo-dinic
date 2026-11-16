# gnuplot -c scripts/plot_results.gp results/
if (ARGC != 1) {
    print "Укажите каталог с measurements.csv и summary.csv"
    exit error
}
raw = ARG1 . "/measurements.csv"
summary = ARG1 . "/summary.csv"
set encoding utf8
set datafile separator comma
set datafile columnheaders
set terminal pngcairo size 1500,740 font "DejaVu Sans,15"
set grid back
set border 3

array Family[3] = ["parallel", "random_chain", "random_graph"]
array Label[3] = ["Параллельные пути", "Цепь со случайными дугами", "Случайный граф"]
array Color[3] = ["#22548A", "#B04C29", "#23795A"]
families = |Family|

set output ARG1 . "/means.png"
set logscale xy
set xlabel "Число вершин V"
set ylabel "Среднее число проверок"
set xrange [55:2400]
set xtics (64, 128, 256, 512, 1024, 2048)
set key left top
plot for [i=1:families] summary using (strcol(1) eq Family[i] ? column(2) : 1/0):4 \
    with linespoints lw 2 pt 7 ps 1.3 lc rgb Color[i] title Label[i]
unset output

unset logscale
set xrange [*:*]
set yrange [0:*]
unset key

array Vertices[2] = [256, 1024]
array LeftEdge[families * |Vertices|]
array Step[families * |Vertices|]
array BinCount[families * |Vertices|]

set table $Histograms
do for [i=1:families] {
    do for [k=1:|Vertices|] {
        panel = (i - 1) * |Vertices| + k
        vertices = Vertices[k]
        stats raw using (strcol(1) eq Family[i] && column(2) == vertices ? column(8) : 1/0) nooutput

        # Подбираем около пяти интервалов с округлёнными границами.
        target = ceil((STATS_max - STATS_min) / 5.0)
        unit = 10.0 ** floor(log10(target > 0 ? target : 1))
        ratio = target / unit
        width = ceil(unit * (ratio <= 1 ? 1 : ratio <= 2 ? 2 : ratio <= 2.5 ? 2.5 : ratio <= 5 ? 5 : 10))
        lower = floor(STATS_min / width) * width
        intervals = int(ceil((STATS_max - lower) / width))
        intervals = intervals > 0 ? intervals : 1
        LeftEdge[panel] = lower
        Step[panel] = width
        BinCount[panel] = intervals
        weight = 100.0 / STATS_records

        # Последний интервал включает максимальное наблюдение.
        bin_index(x) = floor((x - lower) / width)
        bin(x) = lower + ((bin_index(x) < intervals ? bin_index(x) : intervals - 1) + 0.5) * width
        plot raw using (strcol(1) eq Family[i] && column(2) == vertices ? bin(column(8)) : 1/0):(weight) \
            smooth frequency
    }
}
unset table

set datafile separator whitespace
unset datafile columnheaders
stats $Histograms using 2 nooutput
set yrange [0:10 * ceil((STATS_max + 5) / 10)]
set ytics nomirror 0,10,100
set format y "%g%%"
set decimalsign ","
set xlabel "Число проверок"
set ylabel "Доля запусков"
set style fill solid 0.85 border lc rgb "#FFFFFF"

set terminal pngcairo size 1500,(390 * families) font "DejaVu Sans,18"
set output ARG1 . "/histograms.png"
set multiplot layout families,|Vertices| margins 0.09,0.955,0.075,0.93 spacing 0.12,0.13
do for [i=1:families] {
    do for [k=1:|Vertices|] {
        panel = (i - 1) * |Vertices| + k
        lower = LeftEdge[panel]
        width = Step[panel]
        upper = lower + width * BinCount[panel]
        set boxwidth width absolute
        set xrange [lower:upper]
        set xtics nomirror lower,width,upper
        set title sprintf("%s, V = %d", Label[i], Vertices[k])
        plot $Histograms index (panel - 1) using 1:2 with boxes lc rgb Color[i], \
            '' index (panel - 1) using 1:2:(gprintf("%g", $2)."%") with labels offset char 0,0.65
    }
}
unset multiplot
unset output
