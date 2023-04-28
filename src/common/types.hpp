// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_COMMON_TYPES_HPP
#define IPTSD_COMMON_TYPES_HPP

#ifndef EIGEN_DEFAULT_TO_ROW_MAJOR
#define EIGEN_DEFAULT_TO_ROW_MAJOR
#endif

#include <Eigen/Eigen>
#include <gsl/gsl>

#include <cmath>
#include <cstddef>
#include <cstdint>

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using f32 = float;
using f64 = double;

using isize = std::ptrdiff_t;
using usize = std::size_t;

template <class T, int Size = Eigen::Dynamic>
using Vector = Eigen::Vector<T, Size>;

template <class T, int Size = Eigen::Dynamic>
using Array = Eigen::Array<T, 1, Size>;

template <class T, int Rows = Eigen::Dynamic, int Cols = Eigen::Dynamic>
using Matrix = Eigen::Matrix<T, Rows, Cols>;

template <class T, int Rows = Eigen::Dynamic, int Cols = Eigen::Dynamic>
using Image = Eigen::Array<T, Rows, Cols>;

template <class T, int Size = Eigen::Dynamic>
using VectorBase = Eigen::MatrixBase<Vector<T, Size>>;

template <class T, int Size = Eigen::Dynamic>
using ArrayBase = Eigen::ArrayBase<Array<T, Size>>;

template <class T, int Rows = Eigen::Dynamic, int Cols = Eigen::Dynamic>
using MatrixBase = Eigen::MatrixBase<Matrix<T, Rows, Cols>>;

template <class T, int Rows = Eigen::Dynamic, int Cols = Eigen::Dynamic>
using ImageBase = Eigen::ArrayBase<Image<T, Rows, Cols>>;

template <class T>
using DenseBase = Eigen::DenseBase<T>;

template <class T>
using Scalar = typename DenseBase<T>::Scalar;

template <class T>
using Vector2 = Vector<T, 2>;

template <class T>
using Vector3 = Vector<T, 3>;

template <class T>
using Vector4 = Vector<T, 4>;

template <class T>
using Vector5 = Vector<T, 5>;

template <class T>
using Vector6 = Vector<T, 6>;

template <class T>
using Matrix2 = Matrix<T, 2, 2>;

template <class T>
using Matrix3 = Matrix<T, 3, 3>;

template <class T>
using Matrix4 = Matrix<T, 4, 4>;

template <class T>
using Matrix5 = Matrix<T, 5, 5>;

template <class T>
using Matrix6 = Matrix<T, 6, 6>;

using Point = Vector2<Eigen::Index>;
using Box = Eigen::AlignedBox<Eigen::Index, 2>;

#endif // IPTSD_COMMON_TYPES_HPP
