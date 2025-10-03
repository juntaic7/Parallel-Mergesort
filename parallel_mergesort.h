#include <iostream>
#include <parlay/sequence.h>
#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/slice.h>
#include <algorithm>

namespace parallel_mergesort {

    constexpr long MERGE_THRESHOLD = 1000;
    constexpr long SORT_THRESHOLD = 100;

        void merge_array(parlay::slice<long*, long*> A, parlay::slice<long*, long*> B, parlay::slice<long*, long*> out, int version) {
            long n1 = A.size();
            long n2 = B.size();
            assert(out.size() == n1 + n2);

            if (n1 == 0) {
                parlay::copy(B, out);
                return;
            } else if (n2 == 0) {
                parlay::copy(A, out);
                return;
            }

            if (version == 1) {
                std::merge(A.begin(), A.end(),
                        B.begin(), B.end(),
                        out.begin());
                return;
            } else if (version == 3 && n1 + n2 <= MERGE_THRESHOLD) {
                std::merge(A.begin(), A.end(),
                        B.begin(), B.end(),
                        out.begin());
                return;
            }
            
            // To prevent segfault in version 2
            if (n1 <= 1 || n2 <= 1) {
                std::merge(A.begin(), A.end(),
                        B.begin(), B.end(),
                        out.begin());
                return;
            }

            if (n1 < n2) {
                merge_array(B, A, out, version);
                return;
            }

            long mid1 = n1 / 2;
            long mid2 = std::lower_bound(B.begin(), B.end(), A[mid1]) - B.begin();

            parlay::par_do(
                [&] () { merge_array(A.cut(0, mid1), B.cut(0, mid2), out.cut(0, mid1 + mid2), version); },
                [&] () { merge_array(A.cut(mid1, n1), B.cut(mid2, n2), out.cut(mid1 + mid2, n1 + n2), version); }
            );
    }

    void mergesort_ping_pong_(parlay::slice<long*, long*> in, parlay::slice<long*, long*> out, int version, bool inplace) {
        long n = in.size();
        if (n <= 1) {
            if (!inplace) parlay::copy(in, out);
            return;
        }

        if (version == 3) {
            if (n <= SORT_THRESHOLD) {
            std::stable_sort(in.begin(), in.end());
            if (!inplace) parlay::copy(in, out);
            return;
            }
        }
        
        // parallel mergesort needs ping pong buffer to avoid read/write conflicts across concurrent tasks.
        parlay::par_do(
            [&] () { mergesort_ping_pong_(in.cut(0, n / 2), out.cut(0, n / 2), version, !inplace); },
            [&] () { mergesort_ping_pong_(in.cut(n / 2, n), out.cut(n / 2, n), version, !inplace); }
            );
        if (inplace) {
            merge_array(out.cut(0, n / 2), out.cut(n / 2, n), in, version);
        } else {
            merge_array(in.cut(0, n / 2), in.cut(n / 2, n), out, version);
        }
    }


    void mergesort_ping_pong(parlay::sequence<long>& array, int version, bool inplace = true) {
        long n = array.size();
        if (n <= 1) return;

        parlay::sequence<long> temp(n);
        mergesort_ping_pong_(array.cut(0, n), temp.cut(0, n), version, inplace);
    }

    void mergesort_alloc_(parlay::slice<long*, long*> in, parlay::slice<long*, long*> temp, int version) {
        long n = in.size();
        if (n <= 1) return;

        if (version == 3 && n <= SORT_THRESHOLD) {
            std::stable_sort(in.begin(), in.end());
            return;
        }

        long mid = n / 2;
        parlay::par_do(
            [&](){ mergesort_alloc_(in.cut(0,   mid), temp.cut(0, mid), version); },
            [&](){ mergesort_alloc_(in.cut(mid, n), temp.cut(mid, n), version); }
        );


        merge_array(in.cut(0, mid), in.cut(mid, n), temp, version);
        parlay::copy(temp, in);
    }

    void mergesort_alloc(parlay::sequence<long>& array, int version) {
        long n = array.size();
        if (n <= 1) return;
        parlay::sequence<long> temp(n);
        mergesort_alloc_(array.cut(0, n), temp.cut(0, n), version);
    }

    void mergesort(parlay::sequence<long>& array, int version, bool alloc=false, bool inplace=true) {
        if (alloc) {
            mergesort_alloc(array, version);
        } else {
            mergesort_ping_pong(array, version, inplace);
        }
    }

}
