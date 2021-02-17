#pragma once

#include "math/num.hpp"
#include "math/vec6.hpp"
#include "math/mat6.hpp"


namespace math {

/**
 * lu_decom() - Perform an LU-decomposition with partial pivoting.
 * @m:  The matrix to factorize.
 * @lu: The result of the factorization, encoded in a single matrix.
 * @p:  The permutation vector.
 *
 * Performes a LU-decomposition on the given matrix A, such that PA = LU with
 * L being a lower triangular matrix and U being an upper triangular matrix
 * and P being the row-pivoting matrix. The pivoting matrix is encoded as
 * vector, containing the row indices of A in the order in which the occur in
 * PA.
 */
template<class T>
auto lu_decomp(mat6_t<T> const& a, mat6_t<T>& lu, vec6_t<index_t>& p, T eps=num<T>::eps) -> bool
{
    // initialization
    lu = a;
    p = { 0, 1, 2, 3, 4, 5 };

    // TODO: optimize/unroll?

    // decomposition
    for (index_t c = 0; c < 6 - 1; ++c) {
        // partial pivoting for current column:
        // swap row r >= c with largest absolute value at [r, c] (i.e. in column) with row c
        {
            // step 1: find element with largest absolute value in column
            index_t r = 0;
            T v = num<T>::zero;

            for (index_t i = c; i < 6; ++i) {
                auto const vi = std::abs(lu[{i, c}]);

                if (v < vi) {
                    v = vi;
                    r = i;
                }
            }

            // step 1.5: abort if we cannot find a sufficiently large pivot
            if (v <= eps) {
                return false;
            }

            // step 2: permutate, swap row r and c
            if (r != c) {
                for (index_t i = 0; i < 6; ++i) {
                    std::swap(lu[{r, i}], lu[{c, i}]);      // swap U[r, :] and U[c, :]
                }
                std::swap(p[r], p[c]);
            }
        }

        // LU-decomposition step:
        for (index_t r = c + 1; r < 6; ++r) {
            // L[r, c] = U[r, c] / U[c, c]
            lu[{r, c}] = lu[{r, c}] / lu[{c, c}];

            // U[r, :] = U[r, :] - (U[r, c] / U[c, c]) * U[c, :]
            for (index_t k = c + 1; k < 6; ++k) {
                lu[{r, k}] = lu[{r, k}] - lu[{r, c}] * lu[{c, k}];
            }
        }
    }

    // last check for r=5, c=5 because we've skipped that above
    if (std::abs(lu[{5, 5}]) <= eps) {
        return false;
    }

    return true;
}

/**
 * lu_solve() - Solve a system of linear equations via a LU-decomposition.
 * @lu: The L and U matrices, encoded in one matrix.
 * @p:  The permutation vector.
 * @b:  The right-hand-side vector of the system.
 * @x:  The vector to solve for.
 *
 * Solves the system of linear equations LUx = Pb, where L is a lower
 * triangular matrix, U is an upper triangular matrix, and P the permutation
 * matrix encoded by the vector p. Essentially performs two steps, first
 * solving Ly = Pb for a temporary vector y and then Ux = y for the desired x.
 */
template<class T>
void lu_solve(mat6_t<T> const& lu, vec6_t<index_t> const& p, vec6_t<T> const& b, vec6_t<T>& x)
{
    // step 0: compute Pb
    auto pb = vec6_t<T> { b[p[0]], b[p[1]], b[p[2]], b[p[3]], b[p[4]], b[p[5]] };

    // step 1: solve Ly = Pb for y (forward substitution)
    auto y = vec6_t<T>{};

    y[0] = pb[0];
    y[1] = pb[1] - lu[{1, 0}] * y[0];
    y[2] = pb[2] - lu[{2, 0}] * y[0] - lu[{2, 1}] * y[1];
    y[3] = pb[3] - lu[{3, 0}] * y[0] - lu[{3, 1}] * y[1] - lu[{3, 2}] * y[2];
    y[4] = pb[4] - lu[{4, 0}] * y[0] - lu[{4, 1}] * y[1] - lu[{4, 2}] * y[2] - lu[{4, 3}] * y[3];
    y[5] = pb[5] - lu[{5, 0}] * y[0] - lu[{5, 1}] * y[1] - lu[{5, 2}] * y[2] - lu[{5, 3}] * y[3] - lu[{5, 4}] * y[4];

    // step 2: solve Ux = y for x (backward substitution)
    x[5] = y[5];
    x[5] /= lu[{5, 5}];

    x[4] = y[4] - lu[{4, 5}] * x[5];
    x[4] /= lu[{4, 4}];

    x[3] = y[3] - lu[{3, 5}] * x[5] - lu[{3, 4}] * x[4];
    x[3] /= lu[{3, 3}];

    x[2] = y[2] - lu[{2, 5}] * x[5] - lu[{2, 4}] * x[4] - lu[{2, 3}] * x[3];
    x[2] /= lu[{2, 2}];

    x[1] = y[1] - lu[{1, 5}] * x[5] - lu[{1, 4}] * x[4] - lu[{1, 3}] * x[3] - lu[{1, 2}] * x[2];
    x[1] /= lu[{1, 1}];

    x[0] = y[0] - lu[{0, 5}] * x[5] - lu[{0, 4}] * x[4] - lu[{0, 3}] * x[3] - lu[{0, 2}] * x[2] - lu[{0, 1}] * x[1];
    x[0] /= lu[{0, 0}];
}

/**
 * ge_solve() - Solve a system of linear equations via Gaussian elimination.
 * @a: The system matrix A.
 * @b: The right-hand-side vector b.
 * @x: The vector to solve for.
 *
 * Solves the system of linear equations Ax = b using Gaussian elimination
 * with partial pivoting.
 */
template<class T>
auto ge_solve(mat6_t<T> a, vec6_t<T> b, vec6_t<T>& x, T eps=num<T>::eps) -> bool
{
    // TODO: optimize/unroll?

    // step 1: Gaussian elimination
    for (index_t c = 0; c < 6 - 1; ++c) {
        // partial pivoting for current column:
        // swap row r >= c with largest absolute value at [r, c] (i.e. in column) with row c
        {
            // step 1: find element with largest absolute value in column
            index_t r = 0;
            T v = num<T>::zero;

            for (index_t i = c; i < 6; ++i) {
                auto const vi = std::abs(a[{i, c}]);

                if (v < vi) {
                    v = vi;
                    r = i;
                }
            }

            // step 1.5: abort if we cannot find a sufficiently large pivot
            if (v <= eps) {
                return false;
            }

            // step 2: permutate, swap row r and c
            if (r != c) {
                for (index_t i = c; i < 6; ++i) {
                    std::swap(a[{r, i}], a[{c, i}]);        // swap A[r, :] and A[c, :]
                }
                std::swap(b[r], b[c]);                      // swap b[r] and b[c]
            }
        }

        // Gaussian elimination step
        for (index_t r = c + 1; r < 6; ++r) {
            auto const v = a[{r, c}] / a[{c, c}];

            // b[r] = b[r] - (A[r, c] / A[c, c]) * b[c]
            b[r] = b[r] - v * b[c];

            // A[r, :] = A[r, :] - (A[r, c] / A[c, c]) * A[c, :]
            for (index_t k = c + 1; k < 6; ++k) {
                a[{r, k}] = a[{r, k}] - v * a[{c, k}];
            }
        }
    }

    // last check for r=5, c=5 because we've skipped that above
    if (std::abs(a[{5, 5}]) <= eps) {
        return false;
    }

    // step 2: backwards substitution
    x[5] = b[5];
    x[5] /= a[{5, 5}];

    x[4] = b[4] - a[{4, 5}] * x[5];
    x[4] /= a[{4, 4}];

    x[3] = b[3] - a[{3, 5}] * x[5] - a[{3, 4}] * x[4];
    x[3] /= a[{3, 3}];

    x[2] = b[2] - a[{2, 5}] * x[5] - a[{2, 4}] * x[4] - a[{2, 3}] * x[3];
    x[2] /= a[{2, 2}];

    x[1] = b[1] - a[{1, 5}] * x[5] - a[{1, 4}] * x[4] - a[{1, 3}] * x[3] - a[{1, 2}] * x[2];
    x[1] /= a[{1, 1}];

    x[0] = b[0] - a[{0, 5}] * x[5] - a[{0, 4}] * x[4] - a[{0, 3}] * x[3] - a[{0, 2}] * x[2] - a[{0, 1}] * x[1];
    x[0] /= a[{0, 0}];

    return true;
}

} /* namespace math */
