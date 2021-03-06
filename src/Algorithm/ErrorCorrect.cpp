//-----------------------------------------------
// Copyright 2009 Wellcome Trust Sanger Institute
// Written by Jared Simpson (js18@sanger.ac.uk)
// Released under the GPL
//-----------------------------------------------
//
// ErrorCorrect - Methods to identify and correct read errors
//
#include "ErrorCorrect.h"
#include "SGAlgorithms.h"

#define VERBOSE_CORRECT 0

// Correct the sequence of a vertex in a transitively reduced string graph
std::string ErrorCorrect::correctVertex(const StringGraph* pGraph, Vertex* pVertex, size_t simpleCutoff, double p_error)
{
    // Construct a multioverlap containing all the reads that have an overlap to
    // this vertex
    MultiOverlap mo = SGAlgorithms::makeExtendedMultiOverlap(pGraph, pVertex);
    if(mo.isConflicted(simpleCutoff))
    {
        SeqTrie leftTrie;
        SeqTrie rightTrie;
        SGAlgorithms::makeExtendedSeqTries(pGraph, pVertex, p_error, &leftTrie, &rightTrie);
        return trieCorrect(pVertex->getSeq().toString(), p_error, leftTrie, rightTrie);
    }
    else
    {
        return mo.calculateConsensusFromPartition(p_error);
    }
}

// trieCorrect builds tries from the overlapping reads
// to attempt to account for overcollapsed repeats 
std::string ErrorCorrect::trieCorrect(const std::string& original, double p_error, SeqTrie& leftTrie, SeqTrie& rightTrie)
{
    std::string consensus;
    
    // Re-map low quality branches in the tries
    leftTrie.remodel(2, log(p_error));
    rightTrie.remodel(2, log(p_error));

    PathScoreVector left_psv;
    PathScoreVector right_psv;

    leftTrie.score(original, p_error, left_psv);
    rightTrie.score(reverse(original), p_error, right_psv);

    size_t leftBestIdx = 0;
    size_t rightBestIdx = 0;
    double leftBestScore = -std::numeric_limits<double>::max();
    double rightBestScore = -std::numeric_limits<double>::max();

#if VERBOSE_CORRECT
    std::cout << "\nLEFTPSV:\n";
#endif

    for(size_t i = 0; i < left_psv.size(); ++i)
    {

        std::string path_str = left_psv[i].path_corrected;
        std::string ods = getDiffString(path_str, original);
#if VERBOSE_CORRECT
        left_psv[i].print();
        printf("CDO: %s\n", ods.c_str());
#endif
        if(left_psv[i].path_score > leftBestScore)
        {
            leftBestScore = left_psv[i].path_score;
            leftBestIdx = i;
        }
    }

#if VERBOSE_CORRECT
    std::cout << "\nRIGHTPSV:\n";
#endif

    for(size_t i = 0; i < right_psv.size(); ++i)
    {
        right_psv[i].reverse();
        std::string path_str = right_psv[i].path_corrected;

        std::string ods = getDiffString(path_str, original);


#if VERBOSE_CORRECT
        right_psv[i].print();
        printf("CDO: %s\n", ods.c_str());
#endif
        if(right_psv[i].path_score > rightBestScore)
        {
            rightBestScore = right_psv[i].path_score;
            rightBestIdx = i;
        }
    }

    double cutoff = -40;
    if( (left_psv.empty() && right_psv.empty()) || (leftBestScore < cutoff && rightBestScore < cutoff))
    {
        consensus = original;
    }
    else if(left_psv.empty() || leftBestScore < cutoff)
    {
        consensus = right_psv[rightBestIdx].path_corrected;
    }
    else if(right_psv.empty() || rightBestScore < cutoff)
    {
        consensus = left_psv[leftBestIdx].path_corrected;
    }
    else
    {
        consensus.reserve(original.size());
        PathScore& left = left_psv[leftBestIdx];
        PathScore& right = right_psv[rightBestIdx];

#if VERBOSE_CORRECT
        std::cout << "\nComputing consensus from paths\n";
        left.print();
        right.print();
#endif

        // Compute the combined consensus
        for(size_t i = 0; i < original.size(); ++i)
        {
            if(left.path_corrected[i] == right.path_corrected[i])
            {
                consensus.push_back(left.path_corrected[i]);
            }
            else
            {
                double ls = log(1 - left.path_score) + left.probVector[i];
                double rs = log(1 - right.path_score)+ right.probVector[i];

                if(ls < rs)
                    consensus.push_back(left.path_corrected[i]);
                else
                    consensus.push_back(right.path_corrected[i]);
            }
        }
    }
    return consensus;
}
