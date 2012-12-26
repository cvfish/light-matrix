/**
 * @file map_expr_internal.h
 *
 * @brief Internal implementation of map expressions
 *
 * @author Dahua Lin
 */

#ifndef LIGHTMAT_MAP_EXPR_INTERNAL_H_
#define LIGHTMAT_MAP_EXPR_INTERNAL_H_

#include <light_mat/matrix/matrix_properties.h>
#include <light_mat/mateval/map_accessors.h>
#include <light_mat/mateval/ewise_eval.h>
#include <light_mat/mateval/fun_maps.h>


namespace lmat
{
	template<typename U> struct linear_map_policy;
	template<typename U> struct percol_map_policy;
}


namespace lmat { namespace internal {

	/********************************************
	 *
	 *  value type determination
	 *
	 ********************************************/

	template<typename Arg, bool IsXpr> struct _arg_value_type;

	template<typename Arg>
	struct _arg_value_type<Arg, true>
	{
		typedef typename matrix_traits<Arg>::value_type type;
	};

	template<typename Arg>
	struct _arg_value_type<Arg, false>
	{
		typedef Arg type;
	};


	template<typename Arg>
	struct arg_value_type
	{
		typedef typename _arg_value_type<Arg, meta::is_mat_xpr<Arg>::value>::type type;
	};

	template<typename FTag, typename... Args> struct map_expr_value;

	template<typename FTag, typename Arg1>
	struct map_expr_value<FTag, Arg1>
	{
		typedef typename matrix_traits<Arg1>::value_type arg1_vtype;
		typedef typename fun_traits<FTag, arg1_vtype>::result_type type;
	};

	template<typename FTag, typename Arg1, typename Arg2>
	struct map_expr_value<FTag, Arg1, Arg2>
	{
		typedef typename arg_value_type<Arg1>::type arg1_vtype;
		typedef typename arg_value_type<Arg2>::type arg2_vtype;
		typedef typename fun_traits<FTag, arg1_vtype, arg2_vtype>::result_type type;
	};

	template<typename FTag, typename Arg1, typename Arg2, typename Arg3>
	struct map_expr_value<FTag, Arg1, Arg2, Arg3>
	{
		typedef typename arg_value_type<Arg1>::type arg1_vtype;
		typedef typename arg_value_type<Arg2>::type arg2_vtype;
		typedef typename arg_value_type<Arg3>::type arg3_vtype;
		typedef typename fun_traits<FTag, arg1_vtype, arg2_vtype, arg3_vtype>::result_type type;
	};


	/********************************************
	 *
	 *  map helper
	 *
	 ********************************************/

#define LMAT_CHECK_EWISE_ACC_SUPPORT(A) \
		static_assert(meta::supports_ewise_access<A>::value, #A " must supports element-wise access.");

	template<typename Arg>
	struct map_expr_helper1
	{
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg)

		static const int ct_nrows = meta::nrows<Arg>::value;
		static const int ct_ncols = meta::ncols<Arg>::value;
		typedef matrix_shape<ct_nrows, ct_ncols> shape_type;
		typedef typename meta::domain_of<Arg>::type domain;

		static shape_type get_shape(const Arg& arg)
		{
			return arg.shape();
		}
	};


	template<typename Arg1, typename Arg2, bool IsXpr1, bool IsXpr2>
	struct map_expr_helper2;

	template<typename Arg1, typename Arg2>
	struct map_expr_helper2<Arg1, Arg2, false, true>
	{
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg2)

		static const int ct_nrows = meta::nrows<Arg2>::value;
		static const int ct_ncols = meta::ncols<Arg2>::value;
		typedef matrix_shape<ct_nrows, ct_ncols> shape_type;
		typedef typename meta::domain_of<Arg2>::type domain;

		static shape_type get_shape(const Arg1& arg1, const Arg2& arg2)
		{
			return arg2.shape();
		}
	};

	template<typename Arg1, typename Arg2>
	struct map_expr_helper2<Arg1, Arg2, true, false>
	{
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg1)

		static const int ct_nrows = meta::nrows<Arg1>::value;
		static const int ct_ncols = meta::ncols<Arg1>::value;
		typedef matrix_shape<ct_nrows, ct_ncols> shape_type;
		typedef typename meta::domain_of<Arg1>::type domain;

		static shape_type get_shape(const Arg1& arg1, const Arg2& arg2)
		{
			return arg1.shape();
		}
	};

	template<typename Arg1, typename Arg2>
	struct map_expr_helper2<Arg1, Arg2, true, true>
	{
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg1)
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg2)

		static const int ct_nrows = meta::common_nrows<Arg1, Arg2>::value;
		static const int ct_ncols = meta::common_ncols<Arg1, Arg2>::value;
		typedef matrix_shape<ct_nrows, ct_ncols> shape_type;
		typedef typename meta::common_domain<Arg1, Arg2>::type domain;

		static shape_type get_shape(const Arg1& arg1, const Arg2& arg2)
		{
			return common_shape(arg1, arg2);
		}
	};


	template<typename Arg1, typename Arg2, typename Arg3, bool IsXpr1, bool IsXpr2, bool IsXpr3>
	struct map_expr_helper3;

	template<typename Arg1, typename Arg2, typename Arg3>
	struct map_expr_helper3<Arg1, Arg2, Arg3, false, false, true>
	{
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg3)

		static const int ct_nrows = meta::nrows<Arg3>::value;
		static const int ct_ncols = meta::ncols<Arg3>::value;
		typedef matrix_shape<ct_nrows, ct_ncols> shape_type;
		typedef typename meta::domain_of<Arg3>::type domain;

		static shape_type get_shape(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
		{
			return arg3.shape();
		}
	};

	template<typename Arg1, typename Arg2, typename Arg3>
	struct map_expr_helper3<Arg1, Arg2, Arg3, false, true, false>
	{
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg2)

		static const int ct_nrows = meta::nrows<Arg2>::value;
		static const int ct_ncols = meta::ncols<Arg2>::value;
		typedef matrix_shape<ct_nrows, ct_ncols> shape_type;
		typedef typename meta::domain_of<Arg2>::type domain;

		static shape_type get_shape(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
		{
			return arg2.shape();
		}
	};

	template<typename Arg1, typename Arg2, typename Arg3>
	struct map_expr_helper3<Arg1, Arg2, Arg3, true, false, false>
	{
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg1)

		static const int ct_nrows = meta::nrows<Arg1>::value;
		static const int ct_ncols = meta::ncols<Arg1>::value;
		typedef matrix_shape<ct_nrows, ct_ncols> shape_type;
		typedef typename meta::domain_of<Arg1>::type domain;

		static shape_type get_shape(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
		{
			return arg1.shape();
		}
	};

	template<typename Arg1, typename Arg2, typename Arg3>
	struct map_expr_helper3<Arg1, Arg2, Arg3, false, true, true>
	{
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg2)
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg3)

		static const int ct_nrows = meta::common_nrows<Arg2, Arg3>::value;
		static const int ct_ncols = meta::common_ncols<Arg2, Arg3>::value;
		typedef matrix_shape<ct_nrows, ct_ncols> shape_type;
		typedef typename meta::common_domain<Arg2, Arg3>::type domain;

		static shape_type get_shape(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
		{
			return common_shape(arg2, arg3);
		}
	};

	template<typename Arg1, typename Arg2, typename Arg3>
	struct map_expr_helper3<Arg1, Arg2, Arg3, true, false, true>
	{
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg1)
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg3)

		static const int ct_nrows = meta::common_nrows<Arg1, Arg3>::value;
		static const int ct_ncols = meta::common_ncols<Arg1, Arg3>::value;
		typedef matrix_shape<ct_nrows, ct_ncols> shape_type;
		typedef typename meta::common_domain<Arg1, Arg3>::type domain;

		static shape_type get_shape(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
		{
			return common_shape(arg1, arg3);
		}
	};

	template<typename Arg1, typename Arg2, typename Arg3>
	struct map_expr_helper3<Arg1, Arg2, Arg3, true, true, false>
	{
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg1)
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg2)

		static const int ct_nrows = meta::common_nrows<Arg1, Arg2>::value;
		static const int ct_ncols = meta::common_ncols<Arg1, Arg2>::value;
		typedef matrix_shape<ct_nrows, ct_ncols> shape_type;
		typedef typename meta::common_domain<Arg1, Arg2>::type domain;

		static shape_type get_shape(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
		{
			return common_shape(arg1, arg2);
		}
	};

	template<typename Arg1, typename Arg2, typename Arg3>
	struct map_expr_helper3<Arg1, Arg2, Arg3, true, true, true>
	{
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg1)
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg2)
		LMAT_CHECK_EWISE_ACC_SUPPORT(Arg3)

		static const int ct_nrows = meta::common_nrows<Arg1, Arg2, Arg3>::value;
		static const int ct_ncols = meta::common_ncols<Arg1, Arg2, Arg3>::value;
		typedef matrix_shape<ct_nrows, ct_ncols> shape_type;
		typedef typename meta::common_domain<Arg1, Arg2, Arg3>::type domain;

		static shape_type get_shape(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
		{
			return common_shape(arg1, arg2, arg3);
		}
	};


	template<typename... Args> struct map_expr_helper;

	template<typename Arg>
	struct map_expr_helper<Arg>
	{
		typedef map_expr_helper1<Arg> type;
	};

	template<typename Arg1, typename Arg2>
	struct map_expr_helper<Arg1, Arg2>
	{
		typedef map_expr_helper2<Arg1, Arg2,
				meta::is_mat_xpr<Arg1>::value,
				meta::is_mat_xpr<Arg2>::value> type;
	};

	template<typename Arg1, typename Arg2, typename Arg3>
	struct map_expr_helper<Arg1, Arg2, Arg3>
	{
		typedef map_expr_helper3<Arg1, Arg2, Arg3,
				meta::is_mat_xpr<Arg1>::value,
				meta::is_mat_xpr<Arg2>::value,
				meta::is_mat_xpr<Arg3>::value> type;
	};


	/********************************************
	 *
	 *  fun type mapping
	 *
	 ********************************************/

	template<typename FTag, typename U, typename... Args> struct map_expr_fun;

	template<typename FTag, typename U, typename Arg1>
	struct map_expr_fun<FTag, U, Arg1>
	{
		typedef typename matrix_traits<Arg1>::value_type arg1_vtype;
		typedef typename fun_map<FTag, arg1_vtype>::type type;
	};

	template<typename FTag, typename U, typename Arg1, typename Arg2>
	struct map_expr_fun<FTag, U, Arg1, Arg2>
	{
		typedef typename arg_value_type<Arg1>::type arg1_vtype;
		typedef typename arg_value_type<Arg2>::type arg2_vtype;
		typedef typename fun_map<FTag, arg1_vtype, arg2_vtype>::type type;
	};

	template<typename FTag, typename U, typename Arg1, typename Arg2, typename Arg3>
	struct map_expr_fun<FTag, U, Arg1, Arg2, Arg3>
	{
		typedef typename arg_value_type<Arg1>::type arg1_vtype;
		typedef typename arg_value_type<Arg2>::type arg2_vtype;
		typedef typename arg_value_type<Arg3>::type arg3_vtype;
		typedef typename fun_map<FTag, arg1_vtype, arg2_vtype, arg3_vtype>::type type;
	};


	/********************************************
	 *
	 *  arg reader mapping
	 *
	 ********************************************/

	template<typename Arg, bool IsXpr, typename U> struct _arg_vec_reader_map;
	template<typename Arg, bool IsXpr, typename U> struct _arg_multicol_reader_map;

	template<typename Arg, typename U>
	struct _arg_vec_reader_map<Arg, true, U>
	{
		typedef typename vec_reader_map<Arg, U>::type type;

		LMAT_ENSURE_INLINE
		static type get(const Arg& a)
		{
			return vec_reader_map<Arg, U>::get(a);
		}
	};

	template<typename Arg, typename U>
	struct _arg_vec_reader_map<Arg, false, U>
	{
		typedef single_reader<Arg, U> type;

		LMAT_ENSURE_INLINE
		static type get(const Arg& a)
		{
			return type(a);
		}
	};

	template<typename Arg, typename U>
	struct _arg_multicol_reader_map<Arg, true, U>
	{
		typedef typename multicol_reader_map<Arg, U>::type type;

		LMAT_ENSURE_INLINE
		static type get(const Arg& a)
		{
			return multicol_reader_map<Arg, U>::get(a);
		}
	};

	template<typename Arg, typename U>
	struct _arg_multicol_reader_map<Arg, false, U>
	{
		typedef multicol_single_reader<Arg, U> type;

		LMAT_ENSURE_INLINE
		static type get(const Arg& a)
		{
			return type(a);
		}
	};

	template<typename Arg, typename U>
	struct arg_vec_reader_map
	{
		typedef _arg_vec_reader_map<Arg, meta::is_mat_xpr<Arg>::value, U> intern_map_t;
		typedef typename intern_map_t::type type;

		LMAT_ENSURE_INLINE
		static type get(const Arg& arg)
		{
			return intern_map_t::get(arg);
		}
	};

	template<typename Arg, typename U>
	struct arg_multicol_reader_map
	{
		typedef _arg_multicol_reader_map<Arg, meta::is_mat_xpr<Arg>::value, U> intern_map_t;
		typedef typename intern_map_t::type type;

		LMAT_ENSURE_INLINE
		static type get(const Arg& arg)
		{
			return intern_map_t::get(arg);
		}
	};



	/********************************************
	 *
	 *  Evaluation policy
	 *
	 ********************************************/

	template<typename Mat>
	struct prefers_linear
	{
		static const bool value = meta::supports_linear_index<Mat>::value;
	};

	template<typename Mat, typename T, typename Kind, bool IsLinear>
	struct _prefers_simd
	{
		static const bool value = false;
	};

	template<typename Mat, typename Kind>
	struct _prefers_simd<Mat, float, Kind, true>
	{
		static const int pw = math::simd_traits<float, Kind>::pack_width;
		static const int mod_ = meta::nelems<Mat>::value % pw;

		static const bool value = meta::is_continuous<Mat>::value && (mod_ == 0);
	};

	template<typename Mat, typename Kind>
	struct _prefers_simd<Mat, double, Kind, true>
	{
		static const int pw = math::simd_traits<float, Kind>::pack_width;
		static const int mod_ = meta::nelems<Mat>::value % pw;

		static const bool value = meta::is_continuous<Mat>::value && (mod_ == 0);
	};

	template<typename Mat, typename Kind>
	struct _prefers_simd<Mat, float, Kind, false>
	{
		static const int pw = math::simd_traits<float, Kind>::pack_width;
		static const int mod_ = meta::nrows<Mat>::value % pw;

		static const bool value = meta::is_percol_continuous<Mat>::value && (mod_ == 0);
	};

	template<typename Mat, typename Kind>
	struct _prefers_simd<Mat, double, Kind, false>
	{
		static const int pw = math::simd_traits<float, Kind>::pack_width;
		static const int mod_ = meta::nrows<Mat>::value % pw;

		static const bool value = meta::is_percol_continuous<Mat>::value && (mod_ == 0);
	};


	template<typename T1, typename T2>
	struct are_simd_compatible_types
	{
		static const bool value = false;
	};

	template<typename T>
	struct are_simd_compatible_types<T, T> { static const bool value = true; };

	template<typename T>
	struct are_simd_compatible_types<mask_t<T>, T> { static const bool value = true; };

	template<typename T>
	struct are_simd_compatible_types<T, mask_t<T> > { static const bool value = true; };


	template<typename Mat, typename T, typename Kind, bool IsLinear>
	struct prefers_simd
	{
		typedef typename matrix_traits<Mat>::value_type VT;
		static const bool value = are_simd_compatible_types<VT, T>::value
				&& _prefers_simd<Mat, T, Kind, IsLinear>::value;
	};


	template<class S, class D>
	struct preferred_map_policy
	{
		typedef typename meta::common_value_type<S, D>::type vtype;

		static const bool prefer_linear =
				prefers_linear<S>::value &&
				prefers_linear<D>::value;

		static const bool prefer_simd =
				prefers_simd<S, vtype, default_simd_kind, prefer_linear>::value &&
				prefers_simd<D, vtype, default_simd_kind, prefer_linear>::value;

		typedef typename meta::if_c<prefer_simd,
				atags::simd<default_simd_kind>,
				atags::scalar >::type atag;

		typedef typename meta::if_c<prefer_linear,
				linear_map_policy<atag>,
				percol_map_policy<atag> >::type type;

	};


	template<typename Arg, bool IsXpr>
	struct _arg_prefers_linear
	{
		static const bool value = true;
	};

	template<typename Arg>
	struct _arg_prefers_linear<Arg, true>
	{
		static const bool value = prefers_linear<Arg>::value;
	};

	template<typename Arg, bool IsXpr, typename Kind, bool IsLinear>
	struct _arg_prefers_simd
	{
		static const bool value = true;
	};

	template<typename Arg, typename Kind, bool IsLinear>
	struct _arg_prefers_simd<Arg, true, Kind, IsLinear>
	{
		typedef typename matrix_traits<Arg>::value_type T;
		static const bool value = prefers_simd<Arg, T, Kind, IsLinear>::value;
	};


	template<typename Arg>
	struct arg_prefers_linear
	{
		static const bool value = _arg_prefers_linear<Arg, meta::is_mat_xpr<Arg>::value>::value;
	};

	template<typename Arg, typename Kind, bool IsLinear>
	struct arg_prefers_simd
	{
		static const bool value =
				_arg_prefers_simd<Arg, meta::is_mat_xpr<Arg>::value, Kind, IsLinear>::value;
	};



} }

#endif /* MAP_EXPR_INTERNAL_H_ */