#ifndef _UCAS_MATH_UTILS_H
#define _UCAS_MATH_UTILS_H

#include <limits>

namespace ucas
{
	/*******************
    *    CONSTANTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
	const double PI = 3.14159265;				//pi
	const double LOG2E = 1.44269504088896340736; //log2(e)
	const float		inf_pos_r32		=  std::numeric_limits<float>::infinity();
	const float		inf_neg_r32		= -std::numeric_limits<float>::infinity();
	const double	inf_pos_r64		=  std::numeric_limits<double>::infinity();
	const double	inf_neg_r64		= -std::numeric_limits<double>::infinity();
	const int		inf_pos_si32	=  std::numeric_limits<int>::max();
	const int		inf_neg_si32	=  std::numeric_limits<int>::min();
    /*-------------------------------------------------------------------------------------------------------------------------*/

	// interval [start, end)
	template <typename T = int>
	struct interval
	{
		T start, end;
		interval() : start(0), end(0){}
		interval(T _start, T _end) : start(_start), end(_end){}
		interval(T _dims) : start(0), end(_dims){}
		T size(){return T(fabs(double(start)-end));}
		std::pair< interval<T>, interval<T> > subtract(interval<T> i)
		{
			std::pair< interval<T>, interval<T> > result;

			if(i.start > start || i.end < start)
				result.first.start = start;
			if(i.start <= start && i.end >= start && i.end < end)
				result.first.start = i.end;
			if(i.start >= end || (i.start <= start && i.end < end))
				result.first.end = end;
			if(i.start > start && i.start < end)
				result.first.end = i.start;
			if(i.start > start && i.end < end)
			{
				result.second.start = i.end;
				result.second.end = end;
			}

			return result;
		}
		bool contains(int i){return i >= start && i<end;}
	};

	// distance function
	template <class T>
	inline T distance(T& x1, T& y1, T& x2, T& y2){
		return static_cast<T>( sqrt( static_cast<double>(x1 - x2) * (x1 - x2) + static_cast<double>(y1 - y2) * (y1 - y2)) );
	}

	// log2 function
	template <typename T>
	inline T log2(const T& x){
		return static_cast<T>(std::log(x) * LOG2E);
	}

	// round function
	template <class T> 
	inline int round(T x) {
		return static_cast<int>(x > 0 ? x + 0.5 : x - 0.5);
	}

	// abs function
	template <class T>
	inline T abs(const T & x){
		if(x < 0)
			return -x;
		else
			return x;
	}

	// partition 'elems' into 'nParts' which differ at most by 1
	inline std::vector < interval<int> > partition(interval<int> elems, size_t nParts)
	{
		std::vector < interval<int> > parts(nParts);
		size_t n_elems_i = elems.size()/nParts;
		size_t elem_count = elems.start;
		for(int i=0; i<parts.size(); i++)
		{
			parts[i].start	= int(elem_count);
			parts[i].end	= int(elem_count + n_elems_i + (i < elems.size()%nParts ? 1 : 0));
			elem_count += parts[i].size();
		}
		return parts;
	}

	template <typename T>
	T infinity(){ return std::numeric_limits<T>::infinity();}

	template <typename T>
	T ssqrt(T x){ return x <= static_cast<T>(0) ? static_cast<T>(0) : sqrt(x);}

	// isnan
#include <cmath>
#ifdef _WIN32
	inline bool is_nan(double x){
		return _isnan(x) == 1;
	}
#else
	inline bool is_nan(double x){
		return std::isnan(x);
	}
#endif


	// generates all octaves between a (>=0) and b (>=0) in a logarithmic scale with base 10
	template<typename T>
	std::vector<T> octspace10(T a, T b) throw (const char*)
	{
		if(a <= 0 || b <= 0)
			throw "in octspace10(): either a or b are not greater than zero";
		T right = pow(10, ceil(log(b)/log(10)));
		T left = pow(10, floor(log(a)/log(10)));
		std::vector<T> result;
		for(T elem = left; elem <= right; elem*=10)
			for(int i=1; i<10; i++)
				if(elem*i >= a && elem*i <= b)
					result.push_back(elem*i);
		return result;
	}

	// isfinite
	template <typename T>
	inline bool isfinite(T x){
		return 
			!is_nan(x) && 
			x != std::numeric_limits<T>::infinity() &&
			x != -std::numeric_limits<T>::infinity();
	}

	template <typename T>
	inline void meanstd(T data[], size_t dim, double & mean, double & std){
		mean = std = 0;
		if(dim == 0)
			return;
		double sum=0, sumsq=0;
		for(size_t i=0; i<dim; i++)
		{
			sum += data[i];
			sumsq += data[i]*data[i];
		}
		mean = sum / static_cast<double>(dim);
		std = std::sqrt(sumsq / static_cast<double>(dim) - mean*mean);
	}

	template <typename T>
	inline void minmax(T data[], size_t dim, T & min, T & max){
		if(dim == 0)
			return;
		min = max = data[0];
		for(size_t i=1; i<dim; i++)
		{
			min = data[i] < min ? data[i] : min;
			max = data[i] > max ? data[i] : max;
		}
	}
}

namespace Maths
{
	namespace Interpolation
	{
		//! Linearly interpolates a given set of points.
		class Linear
		{
		public:

			//! Class constructor
			Linear(int _n, double *x, double *y)
			{
				n = _n;
				m_x = new double[static_cast<int>(n)];
				m_y = new double[static_cast<int>(n)];

				for (int i = 0; i < n; ++i) 
				{
					m_x[i] = x[i];
					m_y[i] = y[i];
				}
			}

			//! Class destructor
			~Linear()
			{
				delete [] m_x;
				delete [] m_y;
			}

			//! Returns an interpolated value.
			double getValue(double x)
			{
				int i = 0;
				while ( (i < n-1) && x > m_x[++i]);
				//printf("x = %.5f, i = %d, m_x[%d] = %.5f, m_x[%d] = %.5f, ", x, i, i-1, m_x[i-1], i, m_x[i]);


				double a = (x - m_x[i - 1]) / (m_x[i] - m_x[i - 1]);
				//printf("a = %.5f, m_y[%d] = %.5f, m_y[%d] = %.5f\n", a, i-1, m_y[i-1], i, m_y[i]);
				return m_y[i - 1] + a * (m_y[i] - m_y[i - 1]);
			}

		private:

			double *m_x, *m_y, n;
		};


		//! A static function implementing the Linear Class for one off calculations
		inline double Linear_once(int N, double *x, double *y, double a )
		{
			// This function is created to enable an Instant Calculator on CodeCogs. 
			// You probably shouldn't be using this function otherwise. 

			Maths::Interpolation:: Linear A(N, x, y);
			return A.getValue(a);
		}
	}
}


#endif