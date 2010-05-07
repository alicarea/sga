//-----------------------------------------------
// Copyright 2009 Wellcome Trust Sanger Institute
// Written by Jared Simpson (js18@sanger.ac.uk)
// Released under the GPL
//-----------------------------------------------
//
// OverlapAlgorithm - This class implements all the logic
// for finding and outputting overlaps for sequence reads
//
#ifndef OVERLAPALGORITHM_H
#define OVERLAPALGORITHM_H

#include "BWT.h"
#include "OverlapBlock.h"
#include "SearchSeed.h"
#include "BWTAlgorithms.h"
#include "Util.h"

struct OverlapResult
{
	OverlapResult() : isSubstring(false) {}
	bool isSubstring;
};

class OverlapAlgorithm
{
	public:
		OverlapAlgorithm(const BWT* pBWT, const BWT* pRevBWT, 
		                 size_t minO, double er,
						 int seedLen, int seedStride,
						  bool irrOnly) : m_pBWT(pBWT), 
		                                  m_pRevBWT(pRevBWT),
										  m_minOverlap(minO),
										  m_errorRate(er),
										  m_seedLength(seedLen),
										  m_seedStride(seedStride),
										  m_bIrreducible(irrOnly) {}

		// Perform the overlap
		// This function is threaded so everything must be const
		OverlapResult overlapRead(const SeqRecord& read, OverlapBlockList* pOutList) const;
	
		// Perform an irreducible overlap
		OverlapResult overlapReadExact(const SeqRecord& read, OverlapBlockList* pOBOut) const;

		// Perform an inexact overlap
		OverlapResult overlapReadInexact(const SeqRecord& read, OverlapBlockList* pOBOut) const;

		// Write the result of an overlap to an ASQG file
		void writeResultASQG(std::ostream& writer, const SeqRecord& read, const OverlapResult& result) const;

		// Write all the overlap blocks pList to the filehandle
		void writeOverlapBlocks(std::ostream& writer, size_t readIdx, const OverlapBlockList* pList) const;

	private:

		// Calculate the ranges in pBWT that contain a prefix of at least minOverlap basepairs that
		// overlaps with a suffix of w.
		void findOverlapBlocksExact(const std::string& w, const BWT* pBWT, const BWT* pRevBWT, 
     		                               const AlignFlags& af, OverlapBlockList* pOBTemp, 
     							           OverlapBlockList* pOBFinal, OverlapResult& result) const;

		// Same as above while allowing mismatches
		void findOverlapBlocksInexactQueue(const std::string& w, const BWT* pBWT, 
				                      const BWT* pRevBWT, const AlignFlags& af, 
									  OverlapBlockList* pOBList, OverlapBlockList* pOBFinal, 
									  OverlapResult& result) const;

		// Same as above while allowing mismatches
		void findOverlapBlocksInexact(const std::string& w, const BWT* pBWT, 
				                      const BWT* pRevBWT, const AlignFlags& af, 
									  OverlapBlockList* pOBList, OverlapBlockList* pOBFinal, 
									  OverlapResult& result) const;
							  

		// Reduce the block list pOBList by removing blocks that correspond to transitive edges
		void computeIrreducibleBlocks(const BWT* pBWT, const BWT* pRevBWT, 
		                              OverlapBlockList* pOBList, OverlapBlockList* pOBFinal) const;
		
		// these recursive functions do the actual work of computing the irreducible blocks
		void _processIrreducibleBlocksExact(const BWT* pBWT, const BWT* pRevBWT, 
		                                    OverlapBlockList& obList, OverlapBlockList* pOBFinal) const;

		//
		void _processIrreducibleBlocksInexact(const BWT* pBWT, const BWT* pRevBWT, 
		                                      OverlapBlockList& obList, OverlapBlockList* pOBFinal) const;

		// Update the overlap block list with a righthand extension to b, removing ranges that become invalid
		void updateOverlapBlockRangesRight(const BWT* pBWT, const BWT* pRevBWT, 
		                                   OverlapBlockList& obList, char b) const;

		//
		inline void calculateSeedParameters(const std::string& w, int& seed_length, int& seed_stride) const;
		
		//
		inline int createSearchSeeds(const std::string& w, const BWT* pBWT, 
                                     const BWT* pRevBWT, int seed_length, int seed_stride, 
									 SearchSeedVector* pOutVector) const;

		//
		inline void extendSeedsExactRightQueue(const std::string& w, const BWT* pBWT, const BWT* pRevBWT, 
                                                 ExtendDirection dir, const SearchSeedVector* pInVector, 
												 SearchSeedQueue* pOutQueue) const;

		//
		inline void extendSeedsExactRight(const std::string& w, const BWT* pBWT, const BWT* pRevBWT, 
                                                 ExtendDirection dir, const SearchSeedVector* pInVector, 
												 SearchSeedVector* pOutVector) const;
	

		//
		inline bool extendSeedExactRight(SearchSeed& seed, const std::string& w, const BWT* pBWT, const BWT* pRevBWT) const;
		inline bool extendSeedExactLeft(SearchSeed& seed, const std::string& w, const BWT* pBWT, const BWT* pRevBWT) const;

		inline void branchSeedRight(const SearchSeed& seed, const std::string& w, const BWT* pBWT, const BWT* pRevBWT, SearchSeedQueue* pQueue) const;
		inline void branchSeedLeft(const SearchSeed& seed, const std::string& w, const BWT* pBWT, const BWT* pRevBWT, SearchSeedQueue* pQueue) const;

		//
		inline void extendSeedInexactRight(SearchSeed& seed, const std::string& w, const BWT* pBWT, const BWT* pRevBWT, 
				                           SearchSeedVector* pOutVector) const;

		//
        inline void extendSeedInexactLeft(SearchSeed& seed, const std::string& w, const BWT* pBWT, const BWT* pRevBWT,
                                          SearchSeedVector* pOutVector) const;


		// Data
		const BWT* m_pBWT;
		const BWT* m_pRevBWT;
		size_t m_minOverlap;
		double m_errorRate;
		int m_seedLength;
		int m_seedStride;
		bool m_bIrreducible;
};

#endif
