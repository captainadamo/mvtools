// See legal notice in Copying.txt for more information

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .

#ifndef __POBLOCKS__
#define __POBLOCKS__

#include "conc/ObjPool.h"
#include "CopyCode.h"
#include "MTSlicer.h"
#include	"MVInterface.h"	// Required for ALIGN_SOURCEBLOCK
#include "SADFunctions.h"
#include "SearchType.h"
#include "Variance.h"

#if (ALIGN_SOURCEBLOCK > 1)
	#include "AllocAlign.h"
#endif	// ALIGN_SOURCEBLOCK

#include	<vector>



// right now 5 should be enough (TSchniede)
#define MAX_PREDICTOR (20)



class DCTClass;
class MVClip;
class MVFrame;



// v2.5.13.1: This class is currently a bit messy,
// it's being reorganised and reworked for further improvement.

class PlaneOfBlocks
{

public :

	typedef	MTSlicer <PlaneOfBlocks>	Slicer;

	PlaneOfBlocks(int _nBlkX, int _nBlkY, int _nBlkSizeX, int _nBlkSizeY, int _nPel, int _nLevel, int _nFlags, int _nOverlapX, int _nOverlapY, int _yRatioUV, conc::ObjPool <DCTClass> *dct_pool_ptr, bool mt_flag);

	~PlaneOfBlocks();

	/* search the vectors for the whole plane */
	void SearchMVs(MVFrame *_pSrcFrame, MVFrame *_pRefFrame, SearchType st,
                  int stp, int _lambda, int _lSAD, int _pennew, int _plevel,
				  int flags, int *out, const VECTOR *globalMVec, short * outfilebuf, int _fieldShiftCur,
				  int * _meanLumaChange, int _divideExtra,
				  int _pzero, int _pglobal, int _badSAD, int _badrange, bool meander, int *vecPrev, bool _tryMany);


/* plane initialisation */

	/* compute the predictors from the upper plane */
	void InterpolatePrediction(const PlaneOfBlocks &pob);

	void WriteHeaderToArray(int *array);
	int WriteDefaultToArray(int *array, int divideExtra);
	int GetArraySize(int divideExtra);
	void FitReferenceIntoArray(MVFrame *_pRefFrame, int *array);
	void EstimateGlobalMVDoubled(VECTOR *globalMVec, Slicer &slicer); // Fizick
	inline int GetnBlkX() { return nBlkX; }
	inline int GetnBlkY() { return nBlkY; }

	void RecalculateMVs(MVClip & mvClip, MVFrame *_pSrcFrame, MVFrame *_pRefFrame, SearchType st,
                  int stp, int _lambda, int _lSAD, int _pennew,
				  int flags, int *out, short * outfilebuf, int fieldShift, int thSAD,
				  int _divideExtra, int smooth, bool meander);

private:

/* fields set at initialization */

	const int      nBlkX;            /* width in number of blocks */
	const int      nBlkY;            /* height in number of blocks */
	const int      nBlkSizeX;        /* size of a block */
	const int      nBlkSizeY;        /* size of a block */
	const int      nBlkCount;        /* number of blocks in the plane */
	const int      nPel;             /* pel refinement accuracy */
   const int      nLogPel;          /* logarithm of the pel refinement accuracy */
   const int      nScale;           /* scaling factor of the plane */
   const int      nLogScale;        /* logarithm of the scaling factor */
   int            nFlags;           /* additionnal flags */
   const int      nOverlapX;        // overlap size
   const int      nOverlapY;        // overlap size
	const int      yRatioUV;
	const int      nLogyRatioUV;     // log of yRatioUV (0 for 1 and 1 for 2)
	const bool     _mt_flag;         // Allows multithreading

	SADFunction *  SAD;              /* function which computes the sad */
   LUMAFunction * LUMA;             /* function which computes the mean luma */
   VARFunction *  VAR;              /* function which computes the variance */
   COPYFunction * BLITLUMA;
   COPYFunction * BLITCHROMA;
   SADFunction *  SADCHROMA;
   SADFunction *  SATD;              /* SATD function, (similar to SAD), used as replacement to dct */

	std::vector <VECTOR>              /* motion vectors of the blocks */
	               vectors;           /* before the search, contains the hierachal predictor */
	                                  /* after the search, contains the best motion vector */

   bool           smallestPlane;     /* say whether vectors can used predictors from a smaller plane */
//	bool           mmx;               /* can we use mmx asm code */
   bool           isse;              /* can we use isse asm code */
   bool           chroma;            /* do we do chroma me */


   int dctpitch;
	conc::ObjPool <DCTClass> *		// Set to 0 if not used
	               _dct_pool_ptr;

	conc::Array <std::vector <int>, 2>
	               freqArray; // temporary array for global motion estimaton [x|y][value]

	int verybigSAD;

/* working fields */

	// Current frame
   MVFrame *pSrcFrame;
   MVFrame *pRefFrame;
   int nSrcPitch[3];
   int nRefPitch[3];
#if (ALIGN_SOURCEBLOCK > 1)
	int nSrcPitch_plane[3];     // stores the pitch of the whole plane for easy access (nSrcPitch in non-aligned mode)
#endif	// ALIGN_SOURCEBLOCK

   VECTOR zeroMVfieldShifted;  // zero motion vector for fieldbased video at finest level pel2

   int dctmode;
   int dctweight16;

	// Current plane
	SearchType searchType;      /* search type used */
   int nSearchParam;           /* additionnal parameter for this search */
   int LSAD;                   // SAD limit for lambda using - Fizick.
   int penaltyNew;             // cost penalty factor for new candidates
   int penaltyZero;            // cost penalty factor for zero vector
   int pglobal;                // cost penalty factor for global predictor
//	int nLambdaLen;             // penalty factor (lambda) for vector length
	int badSAD;                 // SAD threshold for more wide search
	int badrange;               // wide search radius
	conc::AtomicInt <int> badcount;      // number of bad blocks refined
	bool temporal;              // use temporal predictor
	bool tryMany;               // try refine around many predictors

	conc::AtomicInt <int> planeSAD;      // summary SAD of plane
	conc::AtomicInt <int> sumLumaChange; // luma change sum

	VECTOR _glob_mv_pred_def;
	int _lambda_level;

	// Parameters from SearchMVs() and RecalculateMVs()
	int *_out;
	short *_outfilebuf;
	int *_vecPrev;
	bool _meander_flag;
	int _pnew;
	int _lsad;
	MVClip *	_mv_clip_ptr;
	int _smooth;
	int _thSAD;

	// Working area
	class WorkingArea
	{
	public:
#if (ALIGN_SOURCEBLOCK > 1)
		typedef	std::vector <uint8_t, AllocAlign <uint8_t, ALIGN_SOURCEBLOCK> >	TmpDataArray;
#else	// ALIGN_SOURCEBLOCK
		typedef	std::vector <uint8_t>	TmpDataArray;
#endif	// ALIGN_SOURCEBLOCK

		int x[3];                   /* absolute x coordinate of the origin of the block in the reference frame */
		int y[3];                   /* absolute y coordinate of the origin of the block in the reference frame */
		int blkx;                   /* x coordinate in blocks */
		int blky;                   /* y coordinate in blocks */
		int blkIdx;                 /* index of the block */
		int blkScanDir;             // direction of scan (1 is left to right, -1 is right to left)

		DCTClass * DCT;

		VECTOR globalMVPredictor;   // predictor of global motion vector

		int planeSAD;               // partial summary SAD of plane
		int sumLumaChange;          // partial luma change sum

		int blky_beg;               // First line of blocks to process from this thread
		int blky_end;               // Last line of blocks + 1 to process from this thread

		// Current block
		const uint8_t* pSrc[3];     // the alignment of this array is important for speed for some reason (cacheline?)

		VECTOR bestMV;              /* best vector found so far during the search */
		int nMinCost;               /* minimum cost ( sad + mv cost ) found so far */
		VECTOR predictor;           /* best predictor for the current vector */
		VECTOR predictors[MAX_PREDICTOR];   /* set of predictors for the current block */

		int nDxMin;                 /* minimum x coordinate for the vector */
		int nDyMin;                 /* minimum y coordinate for the vector */
		int nDxMax;                 /* maximum x corrdinate for the vector */
		int nDyMax;                 /* maximum y coordinate for the vector */

		int nLambda;                /* vector cost factor */
		int iter;                   // MOTION_DEBUG only?
		int srcLuma;

		// Data set once
		TmpDataArray dctSrc;
		TmpDataArray dctRef;
#if (ALIGN_SOURCEBLOCK > 1)
		TmpDataArray pSrc_temp_base;// stores base memory pointer to non _base pointer
		uint8_t* pSrc_temp[3];      //for easy WRITE access to temp block
#endif	// ALIGN_SOURCEBLOCK

							WorkingArea (int nBlkSizeX, int nBlkSizeY, int dctpitch, int nLogyRatioUV, int yRatioUV);
		virtual			~WorkingArea ();

		inline bool IsVectorOK(int vx, int vy) const;
		inline int MotionDistorsion(int vx, int vy) const;
	};

	class WorkingAreaFactory
	:	public conc::ObjFactoryInterface <WorkingArea>
	{
	public:
							WorkingAreaFactory (int nBlkSizeX, int nBlkSizeY, int dctpitch, int nLogyRatioUV, int yRatioUV);
	protected:
		// conc::ObjFactoryInterface
		virtual WorkingArea *
							do_create ();
	private:
		int				_blk_size_x;
		int				_blk_size_y;
		int				_dctpitch;
		int				_y_ratio_uv_log;
		int				_y_ratio_uv;
	};

	typedef	conc::ObjPool <WorkingArea>	WorkingAreaPool;

	WorkingAreaFactory
						_workarea_fact;
	WorkingAreaPool
						_workarea_pool;

	VECTOR *       _gvect_estim_ptr;	// Points on the global motion vector estimation result. 0 when not used.
	conc::AtomicInt <int>
	               _gvect_result_count;

/* mv search related functions */

	/* fill the predictors array */
	void FetchPredictors(WorkingArea &workarea);

	/* performs a diamond search */
	void DiamondSearch(WorkingArea &workarea, int step);

	/* performs a square search */
//	void SquareSearch(WorkingArea &workarea);

	/* performs an exhaustive search */
//	void ExhaustiveSearch(WorkingArea &workarea, int radius); // diameter = 2*radius - 1

	/* performs an n-step search */
	void NStepSearch(WorkingArea &workarea, int stp);

	/* performs a one time search */
	void OneTimeSearch(WorkingArea &workarea, int length);

	/* performs an epz search */
	void PseudoEPZSearch(WorkingArea &workarea);

//	void PhaseShiftSearch(int vx, int vy);

	/* performs an exhaustive search */
	void ExpandingSearch(WorkingArea &workarea, int radius, int step, int mvx, int mvy); // diameter = 2*radius + 1

	void Hex2Search(WorkingArea &workarea, int i_me_range);
	void CrossSearch(WorkingArea &workarea, int start, int x_max, int y_max, int mvx, int mvy);
	void UMHSearch(WorkingArea &workarea, int i_me_range, int omx, int omy);

   /* inline functions */
	inline const uint8_t *GetRefBlock(WorkingArea &workarea, int nVx, int nVy);
	inline const uint8_t *GetRefBlockU(WorkingArea &workarea, int nVx, int nVy);
	inline const uint8_t *GetRefBlockV(WorkingArea &workarea, int nVx, int nVy);
	inline const uint8_t *GetSrcBlock(int nX, int nY);
//	inline int LengthPenalty(int vx, int vy);
	int LumaSADx (WorkingArea &workarea, const unsigned char *pRef0);
	inline int LumaSAD (WorkingArea &workarea, const unsigned char *pRef0);
	inline void CheckMV0(WorkingArea &workarea, int vx, int vy);
	inline void CheckMV(WorkingArea &workarea, int vx, int vy);
	inline void CheckMV2(WorkingArea &workarea, int vx, int vy, int *dir, int val);
	inline void CheckMVdir(WorkingArea &workarea, int vx, int vy, int *dir, int val);
	inline int ClipMVx(WorkingArea &workarea, int vx);
	inline int ClipMVy(WorkingArea &workarea, int vy);
	inline VECTOR ClipMV(WorkingArea &workarea, VECTOR v);
	inline static int Median(int a, int b, int c);
	inline static unsigned int SquareDifferenceNorm(const VECTOR& v1, const VECTOR& v2);
	inline static unsigned int SquareDifferenceNorm(const VECTOR& v1, const int v2x, const int v2y);
	inline bool IsInFrame(int i);

	void Refine(WorkingArea &workarea);

	void	search_mv_slice (Slicer::TaskData &td);
	void	recalculate_mv_slice (Slicer::TaskData &td);

	void	estimate_global_mv_doubled_slice (Slicer::TaskData &td);

};

#endif
