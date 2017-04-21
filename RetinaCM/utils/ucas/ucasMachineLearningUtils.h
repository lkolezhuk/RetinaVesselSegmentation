#ifndef _UCAS_MACHINE_LEARNING_UTILS_H
#define _UCAS_MACHINE_LEARNING_UTILS_H

#include <string>
#include <fstream>
#include <functional>
#include <algorithm>
#include "ucasLog.h"

namespace ucas
{
	struct classification_outcome
	{
		int TP, FN, FP;
		classification_outcome() : TP(0), FN(0), FP(0){};
	};

	struct ROC_point
	{
		float threshold, TPR, FPR;
		ROC_point() : threshold(0.0f), TPR(0.0f), FPR(0.0f){}
		ROC_point(float t, float tpr, float fpr) : threshold(t), TPR(tpr), FPR(fpr){}
	};

	template <typename T = float>
	struct categorical_feature
	{
		std::string sampleID;
		std::string category;
		std::string name;
		T value;

		categorical_feature(const std::string & _sampleID, const std::string & _category, const std::string & _name, T _value) :
			sampleID(_sampleID), category(_category), name(_name), value(_value){}
	};

	namespace ml
	{
		// define ROCpoint class
		template <typename T>
		struct ROCpoint
		{
			T tpr, fpr;		// point
			T t;			// threshold

			ROCpoint<T>(T _tpr, T _fpr, T _t) : tpr(_tpr), fpr(_fpr), t(_t){}
			ROCpoint<T>() : tpr(-1), fpr(-1), t(-1){}

			bool operator==(const ROCpoint<T> & r){
				return tpr == r.tpr && fpr == r.fpr;
			}
			bool operator!=(const ROCpoint<T> & r){
				return !(this == r);
			}
		};

		// ROC worker for multithreading
		template <typename T>
		inline void
			ROC_worker(
			std::vector<T> &pos,					// positive sample score array
			std::vector<T> &neg,					// negative sample score array
			std::vector< ROCpoint<T> > &curve,		// preallocated ROC curve
			std::vector<T> &thresholds,				// precomputed set of thresholds
			size_t t0, size_t t1,					// range [t0, t1) of ROC points to calculate
			bool pos_greater_than_neg)				// 1 = the higher the sample score, the higher the probability of being positive
			// 0 = the lower  the sample score, the higher the probability of being positive
		{
			for(size_t k=t0; k<t1; k++)
			{
				int TPs=0, FPs=0;
				for(int i=0; i<pos.size(); i++)
					TPs += static_cast<int>(pos_greater_than_neg ? pos[i] >= thresholds[k] : pos[i] <= thresholds[k]);
				for(int i=0; i<neg.size(); i++)
					FPs += static_cast<int>(pos_greater_than_neg ? neg[i] >= thresholds[k] : neg[i] <= thresholds[k]);
				curve[k].tpr = static_cast<T>(TPs) / pos.size();
				curve[k].fpr = static_cast<T>(FPs) / neg.size();
				curve[k].t = thresholds[k];
			}
		}

		// calculate ROC curve from positive and negative sample score arrays (multi-threaded version)
		template <typename T> 
		inline 
			std::vector< ROCpoint<T> >		// return a sequence of ROC points
			ROCmt(
			std::vector<T> &pos,			// positive sample score array
			std::vector<T> &neg,			// negative sample score array
			bool pos_greater_than_neg = 1)	// 1 = the higher the sample score, the higher the probability of being positive
			// 0 = the lower  the sample score, the higher the probability of being positive
			throw ( ucas::Error )
		{
			// check preconditions
			if(pos.empty())
				throw ucas::Error("in ROC(): no positive samples found");
			if(neg.empty())
				throw ucas::Error("in ROC(): no negative samples found");

			// discard nan scores
			pos.erase(std::remove_if(pos.begin(), pos.end(), is_nan), pos.end());
			neg.erase(std::remove_if(neg.begin(), neg.end(), is_nan), neg.end());

			// generate unique, left-to-right ROC points from distinct scores (also adding +inf and -inf corresponding to ROC extremes (0,0) and (1,1))
			std::vector<T> thresholds;
			for(int i=0; i<pos.size(); i++)
				thresholds.push_back(pos[i]);
			for(int i=0; i<neg.size(); i++)
				thresholds.push_back(neg[i]);
			thresholds.push_back(std::numeric_limits<T>::infinity());
			thresholds.push_back(-std::numeric_limits<T>::infinity());
			if(pos_greater_than_neg)
				std::sort(thresholds.begin(), thresholds.end(), std::greater<T>());	
			else
				std::sort(thresholds.begin(), thresholds.end(), std::less<T>());	
			thresholds.erase(unique(thresholds.begin(), thresholds.end()), thresholds.end());

			// compute ROC	
			std::vector< ROCpoint<T> > out(thresholds.size());
			if(ucas::MULTITHREADED_TESTING)
			{
				int n_threads = ucas::THREADS_CONCURRENCY;
				std::vector <std::thread*> threads;
				size_t n_thresholds_i = thresholds.size()/n_threads;
				size_t t_count = 0;
				for(int t=0; t<n_threads; t++)
				{
					std::thread *thr = new std::thread(ucas::ml::ROC_worker<T>, std::ref(pos), std::ref(neg), std::ref(out), std::ref(thresholds), t_count, t_count + (t<thresholds.size()%n_threads ? n_thresholds_i+1 : n_thresholds_i), pos_greater_than_neg );
					if(!thr)
						throw ucas::Error(ucas::strprintf("Failed to allocated thread %d/%d)", t+1, n_threads));
					threads.push_back(thr);
					t_count += (t<thresholds.size()%n_threads ? n_thresholds_i+1 : n_thresholds_i);
				}

				// run and join
				for(int t=0; t<n_threads; t++)
					threads[t]->join();

				// release memory
				for(int t=0; t<n_threads; t++)
					delete threads[t];
			}
			else
				ROC_worker<T>(pos, neg, out, thresholds, 0, thresholds.size(), pos_greater_than_neg);

			// remove duplicates
			out.erase(unique(out.begin(), out.end()), out.end());

			// push (0,0) if needed
			if(out.front().tpr != 0 || out.front().fpr != 0)
				out.insert(out.begin(), ROCpoint<T>(0,0,pos_greater_than_neg ? std::numeric_limits<T>::infinity() : -std::numeric_limits<T>::infinity()));

			// check computed ROC ends at (TPR,FPR)=(1,1)
			if( out.back().tpr != 1 || out.back().fpr != 1 )
				throw ucas::Error("in ROC(): computed ROC not in the (TPR,FPR) range [(0,0),(1,1)]");

			return out;
		}

		// calculate ROC curve from positive and negative sample score arrays
		template <typename T> 
		inline 
			std::vector<std::pair<T, T>>	// return a (TPRi,FPRi) sequence from (0,0) to (1,1)
			ROC(
			std::vector<T> &pos,			// positive sample score array
			std::vector<T> &neg,			// negative sample score array
			bool pos_greater_than_neg = 1)	// 1 = the higher the sample score, the higher the probability of being positive
			// 0 = the lower  the sample score, the higher the probability of being positive
			throw ( ucas::Error )
		{
			// check preconditions
			if(pos.empty())
				throw Error("in ROC(): no positive samples found");
			if(neg.empty())
				throw Error("in ROC(): no negative samples found");

			// discard nan scores
			pos.erase(std::remove_if(pos.begin(), pos.end(), is_nan), pos.end());
			neg.erase(std::remove_if(neg.begin(), neg.end(), is_nan), neg.end());
			//pos.erase(std::remove_if(pos.begin(), pos.end(), std::not1(std::ptr_fun(mcd::isfinite<T>))), pos.end());
			//neg.erase(std::remove_if(neg.begin(), neg.end(), std::not1(std::ptr_fun(mcd::isfinite<T>))), neg.end());

			// generate unique, left-to-right ROC points from distinct scores (also adding +inf and -inf corresponding to ROC extremes (0,0) and (1,1))
			std::vector<T> thresholds;
			for(int i=0; i<pos.size(); i++)
				thresholds.push_back(pos[i]);
			for(int i=0; i<neg.size(); i++)
				thresholds.push_back(neg[i]);
			thresholds.push_back(std::numeric_limits<T>::infinity());
			thresholds.push_back(-std::numeric_limits<T>::infinity());
			if(pos_greater_than_neg)
				std::sort(thresholds.begin(), thresholds.end(), std::greater<T>());	
			else
				std::sort(thresholds.begin(), thresholds.end(), std::less<T>());	
			thresholds.erase(unique(thresholds.begin(), thresholds.end()), thresholds.end());

			// compute ROC	
			std::vector<std::pair<T, T>> out;
			for(int k=0; k<thresholds.size(); k++)
			{
				int TPs=0, FPs=0;
				for(int i=0; i<pos.size(); i++)
					TPs += static_cast<int>(pos_greater_than_neg ? pos[i] >= thresholds[k] : pos[i] <= thresholds[k]);
				for(int i=0; i<neg.size(); i++)
					FPs += static_cast<int>(pos_greater_than_neg ? neg[i] >= thresholds[k] : neg[i] <= thresholds[k]);
				out.push_back(std::pair<T,T>(static_cast<T>(TPs) / pos.size(), static_cast<T>(FPs) / neg.size()));
			}

			// remove duplicates
			out.erase(unique(out.begin(), out.end()), out.end());


			// push (0,0) if needed
			if(out.front().first != T(0) || out.front().second != T(0))
				out.insert(out.begin(), std::pair<T,T>(T(0),T(0)));

			// check computed ROC ends at (TPR,FPR)=(1,1)
			if( out.back().first != 1 || out.back().second != 1 )
				throw ucas::Error("in ROC(): computed ROC not in the (TPR,FPR) range [(0,0),(1,1)]");

			return out;
		}



		// calculate AUC from ROC curve using trapezoidal rule
		template <typename T>
		inline 
			T											// return AUC
			AUC_trapz(
			std::vector<std::pair<T, T>> &ROC)	// ROC curve
			throw ( ucas::Error )
		{
			// check preconditions
			if(ROC.empty())
				throw Error("in AUC_roc(): ROC is empty");
			if( (ROC.front().first != 0 && ROC.front().second != 0) && (ROC.back().first != 1 && ROC.back().second != 1) )
				throw Error("in AUC_roc(): computed ROC not in the (TPR,FPR) range [(0,0),(1,1)]");

			// calculate AUC using trapezoidal rule
			T out = 0;
			for(int i=0; i<ROC.size()-1; i++)
				out += ((ROC[i].first + ROC[i+1].first)*ucas::abs(ROC[i].second - ROC[i+1].second))/2.0;
			return out;
		}

		// calculate AUC from ROC curve using trapezoidal rule
		template <typename T>
		inline 
			T											// return AUC
			AUC_trapz(
			std::vector< ROCpoint<T> > &ROC)	// ROC curve
			throw ( ucas::Error )
		{
			// check preconditions
			if(ROC.empty())
				throw ucas::Error("in AUC_roc(): ROC is empty");
			if( (ROC.front().tpr != 0 && ROC.front().fpr != 0) && (ROC.back().tpr != 1 && ROC.back().fpr != 1) )
				throw ucas::Error("in AUC_roc(): computed ROC not in the (TPR,FPR) range [(0,0),(1,1)]");

			// calculate AUC using trapezoidal rule
			T out = 0;
			for(int i=0; i<ROC.size()-1; i++)
				out += ((ROC[i].tpr + ROC[i+1].tpr)*ucas::abs(ROC[i].fpr - ROC[i+1].fpr))/2.0;
			return out;
		}

		// calculate AUC from sample score arrays using trapezoidal rule applied on the calculated ROC curve
		template <typename T>
		inline 
			T									// return AUC
			AUC_trapz(
			std::vector<T> &pos,			// positive sample score array
			std::vector<T> &neg,			// negative sample score array
			bool pos_greater_than_neg = 1)	// 1 = the higher the sample score, the higher the probability of being positive
			// 0 = the lower  the sample score, the higher the probability of being positive
			throw (Error)
		{
			std::vector<std::pair<T,T>> roc = ROC(pos, neg, pos_greater_than_neg);
			return AUC_trapz<T>(roc);
		}


		// calculate AUC with Wilcoxon-Mann-Whitney from positive and negative sample score arrays
		template <typename T>
		inline 
			T									// return AUC
			AUC_wmw(
			std::vector<T> &pos,			// positive sample scores array
			std::vector<T> &neg,			// negative sample scores array
			bool pos_greater_than_neg = 1)	// 1 = the higher the sample score, the higher the probability of being positive
			// 0 = the lower  the sample score, the higher the probability of being positive
		{
			// discard nonfinite scores
			int nPos = int(pos.size()), nNeg = int(neg.size());	// store #pos, #neg before we erase samples
			pos.erase(std::remove_if(pos.begin(), pos.end(), is_nan), pos.end());
			neg.erase(std::remove_if(neg.begin(), neg.end(), is_nan), neg.end());

			// apply WMW rule
			double res = 0;
			if(pos_greater_than_neg)
			{
				for(auto & p: pos)
					for(auto & n : neg)
						if(p > n)
							res += 1.0;
						else if(p == n)
							res += 0.5;
			}
			else
			{
				for(auto & p: pos)
					for(auto & n : neg)
						if(p < n)
							res += 1.0;
						else if(p == n)
							res += 0.5;
			}
			return res/(static_cast<T>(pos.size())*static_cast<T>(neg.size()));
		}

		template <typename T>
		inline 
			void 
			saveROC(std::vector<std::pair<T, T>> & curve, const std::string & filePath) throw ( ucas::Error )
		{
			std::ofstream f(filePath);
			if(!f.is_open())
				throw ucas::CannotOpenFileError(filePath);
			for(auto & point : curve)
				f << ucas::strprintf("-1 %.6f %.6f\n", point.first, point.second);
			f.close();
		}

		template <typename T>
		inline 
			void 
			saveROC(std::vector<ROCpoint<T>> & curve, const std::string & filePath) throw ( ucas::Error )
		{
			std::ofstream f(filePath);
			if(!f.is_open())
				throw ucas::CannotOpenFileError(filePath);
			for(auto & point : curve)
				f << ucas::strprintf("-1 %.6f %.6f\n", point.tpr, point.fpr);
			f.close();
		}

		// read sample .sco files
		inline void 
			readSCO(
			const std::string& path,			// absolute path of sample score file
			std::vector<double> &scores)		// output vector of sample scores
			throw (ucas::Error)
		{
			std::ifstream f(path);
			if(!f.is_open())
				throw ucas::Error(ucas::strprintf("Cannot open sample score file at \"%s\"", path.c_str()));

			std::string line;
			std::getline(f, line);			// skip header
			while(std::getline(f, line))
			{
				std::replace(line.begin(), line.end(), '\t', ' ');
				line = ucas::singlespaces(line);
				std::vector <std::string> tokens;
				ucas::split(line, " ", tokens);
				if(tokens.size() != 2)
					throw ucas::Error(ucas::strprintf("Cannot parse line \"%s\" from file \"%s\": expected 2 space-separated tokens, found %d", line.c_str(), path.c_str(), tokens.size()));
				scores.push_back(ucas::str2f(tokens[1].c_str()));
			}						
			f.close();
		}
	}


	namespace mltest
	{
		inline void auc_tests(int iterations = 100, double eps = 10e-10) throw (ucas::Error)
		{
			for(int k=1; k<=iterations; k++)
			{
				double t0 = -TIME(0);
				printf("auc_tests iteration %03d/%03d...", k, iterations);

				int nPos = 10000, nNeg = 10000;
				//int nPos = rand()%99991 +10;	// in the range [10, 100000]
				//int nNeg = rand()%99991 +10;	// in the range [10, 100000]

				// generate pseudo-random positive and negative sample scores in the range [0,1]
				// also randomly add -inf or +inf with 1% probability
				std::vector<double> pos, neg;
				for(int i=0; i<nPos; i++)
					if( (rand()%100+1) == 1)
						pos.push_back( rand()%2 ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity());
					else
						pos.push_back( static_cast<double>(rand())/static_cast<double>(RAND_MAX) );
				for(int i=0; i<nNeg; i++)
					if( (rand()%100+1) == 1)
						neg.push_back( rand()%2 ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity());
					else 
						neg.push_back( static_cast<double>(rand())/static_cast<double>(RAND_MAX) );


				// test #1: AUC must be the almost the same for all methods
				double val1a = ucas::ml::AUC_trapz(pos, neg);
				double val1b = ucas::ml::AUC_wmw(pos, neg);
				if(ucas::abs(val1a-val1b) > eps)
					throw ucas::Error(ucas::strprintf("test #1 failed: mcd::abs(AUC_trapz-AUC_wmw) > %g (%g - %g = %g)", eps, val1a, val1b, ucas::abs(val1a-val1b)));


				// test #2: AUC must satisfy the complement rule
				double val2a = ucas::ml::AUC_trapz(pos, neg, 0);
				double val2b = ucas::ml::AUC_wmw(pos, neg, 0);
				if(ucas::abs(val2a+val1a-1) > 10e-10)
					throw ucas::Error(ucas::strprintf("test #2 failed: famcd::absbs(val2a+val1a-1) > %g (%g)", eps, val2a+val1a-1));
				if(ucas::abs(val2b+val1b-1) > 10e-10)
					throw ucas::Error(ucas::strprintf("test #2 failed: mcd::abs(val2b+val1b-1) > %g (%g)", eps, val2b+val1b-1));


				// test #3: AUC should not be influenced by nan values
				int nPosNans = rand()%100+1;
				int nNegNans = rand()%100+1;
				double denominator = 0;
				for(int i=0; i<nPosNans; i++)
					pos.push_back((i-i)/denominator);
				for(int i=0; i<nNegNans; i++)
					neg.push_back((i-i)/denominator);
				double val3a = ucas::ml::AUC_trapz(pos, neg);
				double val3b = ucas::ml::AUC_wmw(pos, neg);
				if(ucas::abs(val3a-val1a) > 0)
					throw ucas::Error(ucas::strprintf("test #3 failed: mcd::abs(val3a-val1a) > 0 (%g)", ucas::abs(val3a-val1a)));
				if(ucas::abs(val3b-val1b) > 0)
					throw ucas::Error(ucas::strprintf("test #3 failed: mcd::abs(val3b-val1b) > 0 (%g)", ucas::abs(val3b-val1b)));


				// test #4: AUC = 1 if all positive scores = +inf and all negative scores = -inf (and 0 otherwise)
				pos.clear();
				neg.clear();
				for(int i=0; i<nPos; i++)
					pos.push_back( std::numeric_limits<double>::infinity() );
				for(int i=0; i<nNeg; i++)
					neg.push_back( -std::numeric_limits<double>::infinity() );
				double val4a = ucas::ml::AUC_trapz(pos, neg);
				double val4b = ucas::ml::AUC_wmw(pos, neg);
				double val4c = ucas::ml::AUC_trapz(pos, neg, false);
				double val4d = ucas::ml::AUC_wmw(pos, neg, false);
				if(val4a != 1)
					throw ucas::Error(ucas::strprintf("test #4 failed: val4a != 1 (%g)", val4a));
				if(val4b != 1)
					throw ucas::Error(ucas::strprintf("test #4 failed: val4b != 1 (%g)", val4b));
				if(val4c != 0)
					throw ucas::Error(ucas::strprintf("test #4 failed: val4c != 0 (%g)", val4a));
				if(val4d != 0)
					throw ucas::Error(ucas::strprintf("test #4 failed: val4d != 0 (%g)", val4b));
				printf("DONE in %.4f s\n", t0 + TIME(0));
			}
		}
	}
}

#endif