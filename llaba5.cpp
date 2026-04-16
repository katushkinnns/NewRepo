#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <fstream>
#include <iomanip>
#include <string>

using namespace std;
using namespace std::chrono;

int linearSearch(const vector<int>& arr, int target, long long& comparisons) {
    comparisons = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        ++comparisons;
        if (arr[i] == target)
            return static_cast<int>(i);
    }
    return -1;
}

int barrierSearch(vector<int>& arr, int target, long long& comparisons) {
    comparisons = 0;
    size_t n = arr.size();
    if (n == 0) return -1;

    int last = arr[n - 1];
    arr[n - 1] = target;

    size_t i = 0;
    while (arr[i] != target) {
        ++comparisons;
        ++i;
    }
    ++comparisons;

    arr[n - 1] = last;

    if (i < n - 1 || last == target)
        return static_cast<int>(i);
    return -1;
}

int binarySearch(const vector<int>& arr, int target, long long& comparisons) {
    comparisons = 0;
    int left = 0;
    int right = static_cast<int>(arr.size()) - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        ++comparisons;
        if (arr[mid] == target)
            return mid;
        ++comparisons;
        if (arr[mid] < target)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return -1;
}

template <typename SearchFunc, typename PrepareArrayFunc>
void measureAverage(const vector<int>& originalArray,
    SearchFunc searchFunc,
    PrepareArrayFunc prepareArray,
    int reps,
    double& avgTimeNs,
    double& avgComparisons)
{
    long long totalTime = 0;
    long long totalComp = 0;
    for (int r = 0; r < reps; ++r) {
        int target = prepareArray();
        long long comp;
        auto start = high_resolution_clock::now();
        searchFunc(originalArray, target, comp);
        auto end = high_resolution_clock::now();
        totalTime += duration_cast<nanoseconds>(end - start).count();
        totalComp += comp;
    }
    avgTimeNs = static_cast<double>(totalTime) / reps;
    avgComparisons = static_cast<double>(totalComp) / reps;
}

template <typename PrepareArrayFunc>
void measureBarrierAverage(vector<int> arrCopy,
    PrepareArrayFunc prepareArray,
    int reps,
    double& avgTimeNs,
    double& avgComparisons)
{
    long long totalTime = 0;
    long long totalComp = 0;
    for (int r = 0; r < reps; ++r) {
        int target = prepareArray();
        long long comp;
        auto start = high_resolution_clock::now();
        barrierSearch(arrCopy, target, comp);
        auto end = high_resolution_clock::now();
        totalTime += duration_cast<nanoseconds>(end - start).count();
        totalComp += comp;
    }
    avgTimeNs = static_cast<double>(totalTime) / reps;
    avgComparisons = static_cast<double>(totalComp) / reps;
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    vector<int> sizes = { 20, 500, 1000, 3000, 5000, 10000 };
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> valueDist(0, 99999);

    ofstream fout1("task1_results.csv");
    fout1 << "Size;Lin_Time_ns;Lin_Comp;Bar_Time_ns;Bar_Comp"
        << ";Lin_Best_Time_ns;Lin_Best_Comp;Lin_Worst_Time_ns;Lin_Worst_Comp"
        << ";Bar_Best_Time_ns;Bar_Best_Comp;Bar_Worst_Time_ns;Bar_Worst_Comp\n";

    for (int n : sizes) {
        vector<int> arr(n);
        for (int& x : arr) x = valueDist(gen);

        double avgLinTime, avgLinComp, avgBarTime, avgBarComp;

        auto randomTarget = [&]() -> int {
            return (gen() % 2 == 0) ? arr[gen() % n] : -1;
            };

        measureAverage(arr, linearSearch, randomTarget, 100, avgLinTime, avgLinComp);
        measureBarrierAverage(arr, randomTarget, 100, avgBarTime, avgBarComp);

        double bestLinTime, bestLinComp, bestBarTime, bestBarComp;
        auto bestTarget = [&]() { return arr[0]; };

        measureAverage(arr, linearSearch, bestTarget, 100, bestLinTime, bestLinComp);
        measureBarrierAverage(arr, bestTarget, 100, bestBarTime, bestBarComp);

        double worstLinTime, worstLinComp, worstBarTime, worstBarComp;
        auto worstTarget = []() { return -1; };

        measureAverage(arr, linearSearch, worstTarget, 100, worstLinTime, worstLinComp);
        measureBarrierAverage(arr, worstTarget, 100, worstBarTime, worstBarComp);

        fout1 << n << ";"
            << avgLinTime << ";" << avgLinComp << ";"
            << avgBarTime << ";" << avgBarComp << ";"
            << bestLinTime << ";" << bestLinComp << ";"
            << worstLinTime << ";" << worstLinComp << ";"
            << bestBarTime << ";" << bestBarComp << ";"
            << worstBarTime << ";" << worstBarComp << "\n";
    }
    fout1.close();

    ofstream gp1("plot_task1_time.gp");
    gp1 << "set terminal png size 800,600\n";
    gp1 << "set output 'task1_time.png'\n";
    gp1 << "set title 'Task 1: Average Search Time'\n";
    gp1 << "set xlabel 'Array Size'\n";
    gp1 << "set ylabel 'Time (ns)'\n";
    gp1 << "set grid\n";
    gp1 << "set datafile separator ';'\n";
    gp1 << "plot 'task1_results.csv' every ::1 using 1:2 with linespoints lw 2 title 'Linear', \\\n";
    gp1 << "     'task1_results.csv' every ::1 using 1:4 with linespoints lw 2 title 'Barrier'\n";
    gp1.close();

    ofstream gp2("plot_task1_comp.gp");
    gp2 << "set terminal png size 800,600\n";
    gp2 << "set output 'task1_comp.png'\n";
    gp2 << "set title 'Task 1: Average Comparisons'\n";
    gp2 << "set xlabel 'Array Size'\n";
    gp2 << "set ylabel 'Comparisons'\n";
    gp2 << "set grid\n";
    gp2 << "set datafile separator ';'\n";
    gp2 << "plot 'task1_results.csv' every ::1 using 1:3 with linespoints lw 2 title 'Linear', \\\n";
    gp2 << "     'task1_results.csv' every ::1 using 1:5 with linespoints lw 2 title 'Barrier'\n";
    gp2.close();

    cout << "[Задание 1] Данные сохранены в task1_results.csv\n";
    cout << "[Задание 1] Сгенерированы скрипты gnuplot: plot_task1_time.gp, plot_task1_comp.gp\n\n";

    ofstream fout2("task2_results.csv");
    fout2 << "Size;Case;Lin_Time_ns;Lin_Comp;Bin_Time_ns;Bin_Comp\n";

    cout << "=== ИТОГОВАЯ ТАБЛИЦА: Линейный vs Бинарный поиск (отсортированные данные) ===" << endl;
    cout << left << setw(8) << "Size" << " | "
        << setw(10) << "Case" << " | "
        << setw(12) << "Lin_T(ns)" << " | " << setw(8) << "Lin_C" << " | "
        << setw(12) << "Bin_T(ns)" << " | " << setw(8) << "Bin_C" << endl;
    cout << string(75, '-') << endl;

    for (int n : sizes) {
        vector<int> arr(n);
        for (int& x : arr) x = valueDist(gen);
        sort(arr.begin(), arr.end());

        double linTimePresent, linCompPresent, binTimePresent, binCompPresent;
        auto presentTarget = [&]() { return arr[gen() % n]; };

        measureAverage(arr, linearSearch, presentTarget, 100, linTimePresent, linCompPresent);
        measureAverage(arr, binarySearch, presentTarget, 100, binTimePresent, binCompPresent);

        double linTimeAbsent, linCompAbsent, binTimeAbsent, binCompAbsent;
        auto absentTarget = []() { return -1; };

        measureAverage(arr, linearSearch, absentTarget, 100, linTimeAbsent, linCompAbsent);
        measureAverage(arr, binarySearch, absentTarget, 100, binTimeAbsent, binCompAbsent);

        auto printRow = [&](const string& caseName, double lt, double lc, double bt, double bc) {
            cout << setw(8) << n << " | "
                << setw(10) << caseName << " | "
                << setw(12) << fixed << setprecision(0) << lt << " | "
                << setw(8) << lc << " | "
                << setw(12) << bt << " | "
                << setw(8) << bc << endl;
            };
        printRow("Present", linTimePresent, linCompPresent, binTimePresent, binCompPresent);
        printRow("Absent", linTimeAbsent, linCompAbsent, binTimeAbsent, binCompAbsent);

        fout2 << n << ";Present;" << linTimePresent << ";" << linCompPresent << ";"
            << binTimePresent << ";" << binCompPresent << "\n";
        fout2 << n << ";Absent;" << linTimeAbsent << ";" << linCompAbsent << ";"
            << binTimeAbsent << ";" << binCompAbsent << "\n";
    }
    fout2.close();

    cout << "\n[Задание 2] Данные сохранены в task2_results.csv\n";
    cout << "Для построения графиков Задания 1 выполните:\n";
    cout << "  gnuplot plot_task1_time.gp\n";
    cout << "  gnuplot plot_task1_comp.gp\n";

    return 0;
}
