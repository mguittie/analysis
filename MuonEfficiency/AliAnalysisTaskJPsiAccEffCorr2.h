#ifndef ALIANALYSISTASKJPSIACCEFFCORR_H
#define ALIANALYSISTASKJPSIACCEFFCORR_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/// \ingroup muondep
/// \class AliAnalysisTaskJPsiAccEffCorr2
/// \brief task to extract the JPsi acc*eff corrections
//Author: Philippe Pillot - SUBATECH Nantes

#include <TObject.h>
#include <TArrayF.h>
#include <TArrayD.h>
#include <THashList.h>
#include "AliAnalysisTaskSE.h"
#include "AliMuonTrackCuts.h"

class TH1F;
class TF1;
class TObjArray;
class TList;
class THashList;
class AliCounterCollection;

//________________________________________________________________________
class Weights : public TNamed {
  
public:
  
  Weights();
  Weights(const Char_t *name, Float_t ptMin, Float_t ptMax, Float_t yMin, Float_t yMax,
	  Int_t nCentBin, Float_t *binLowEdge, Double_t *nSig, Bool_t weightRec);
  virtual ~Weights() {}
  
  // set the array of weights associated to centrality key words using the given centrality bins
  void Init(TArrayF &centBinLowEdge);
  
  /// get the pt/y validity range for these weights
  void GetValidityRange(Float_t &ptMin, Float_t &ptMax, Float_t &yMin, Float_t &yMax) const {
    ptMin = fPtMin; ptMax = fPtMax; yMin = fYMin; yMax = fYMax;
  }
  
  /// return the array of centrality bin low edges
  const TArrayF& GetCentBinLowEdge() const { return fBinLowEdge; }
  
  /// get the global centrality range for these weights
  void GetCentralityRange(Float_t &centMin, Float_t &centMax) const {
    centMin = fBinLowEdge[0]; centMax = fBinLowEdge[fnCentBin];
  }
  
  /// return the flag telling whether to weight at the reconstruction or at the generation level
  Bool_t WeightRec() const { return fWeightRec; }
  
  /// get the list of weights with associated centrality key words
  const THashList& GetWeights() const {return fWeights;}
  
  /// check whether the given pt/y range is within the pt/y range valid for these weights
  Bool_t IsValid(Float_t ptMin, Float_t ptMax, Float_t yMin, Float_t yMax) const {
    return (ptMin >= fPtMin && ptMax <= fPtMax && yMin >= fYMin && yMax <= fYMax);
  }
  
  /// check whether the pt/y range of the given weights is within the pt/y range valid for these weights
  Bool_t IsValid(Weights &w) const {
    return (w.fPtMin >= fPtMin && w.fPtMax <= fPtMax && w.fYMin >= fYMin && w.fYMax <= fYMax);
  }
  
public:
  
  // return the centrality key work (or list of key words) for the given range
  static TString GetCentKey(Float_t centMin, Float_t centMax, const TArrayF &centBinLowEdge);
  
private:
  
  /// Not implemented
  Weights(const Weights& rhs);
  /// Not implemented
  Weights& operator = (const Weights& rhs);
  
private:
  
  Float_t   fPtMin;      ///< pt bin low-edge
  Float_t   fPtMax;      ///< pt bin up-edge
  Float_t   fYMin;       ///< y bin low-edge
  Float_t   fYMax;       ///< y bin up-edge
  Int_t     fnCentBin;   ///< number of centrality bins
  TArrayF   fBinLowEdge; ///< centrality bin low-edge values
  TArrayD   fnSignal;    ///< number of signal in each bin
  Bool_t    fWeightRec;  ///< weight at the reconstruction or at the generation level
  THashList fWeights;    //!< list of weights for efficiency calculation
  
  ClassDef(Weights, 1);
};

//________________________________________________________________________
class FuncRange : public TObject {

public:
  
  FuncRange();
  FuncRange(TF1 &f, Float_t centMin, Float_t centMax, Double_t pTMin, Double_t pTMax, Double_t yMin, Double_t yMax);
  virtual ~FuncRange();
  
  /// check the validity of the function for this particular centrality-pT-y combination
  Bool_t IsValid(Float_t cent, Double_t pT, Double_t y) const {
    return (cent > fCentMin && cent <= fCentMax && pT >= fPtMin && pT < fPtMax && y >= fYMin && y < fYMax);
  }
  
  /// check whether the given validity domain overlap the one of this function
  Bool_t Overlap(Float_t centMin, Float_t centMax, Double_t pTMin, Double_t pTMax, Double_t yMin, Double_t yMax) const {
    return !(centMin+1.e-4 > fCentMax || centMax-1.e-4 < fCentMin || pTMin+1.e-4 > fPtMax || pTMax-1.e-4 < fPtMin ||
             yMin+1.e-4 > fYMax || yMax-1.e-4 < fYMin);
  }
  
  /// return the function
  TF1* GetFunc() { return fFunc; }
  
private:
  
  /// Not implemented
  FuncRange(const FuncRange& rhs);
  /// Not implemented
  FuncRange& operator = (const FuncRange& rhs);
  
  Float_t  fCentMin; ///< centrality bin low-edge
  Float_t  fCentMax; ///< centrality bin up-edge
  Double_t fPtMin;   ///< pt bin low-edge
  Double_t fPtMax;   ///< pt bin up-edge
  Double_t fYMin;    ///< y bin low-edge
  Double_t fYMax;    ///< y bin up-edge
  TF1     *fFunc;    ///< function (owner)
  
  ClassDef(FuncRange, 2);
};

//________________________________________________________________________
class AliAnalysisTaskJPsiAccEffCorr2 : public AliAnalysisTaskSE {

public:
  
  AliAnalysisTaskJPsiAccEffCorr2();
  AliAnalysisTaskJPsiAccEffCorr2(const char *name);
  virtual ~AliAnalysisTaskJPsiAccEffCorr2();
  
  virtual void   UserCreateOutputObjects();
  virtual void   NotifyRun();
  virtual void   UserExec(Option_t *);
  virtual void   Terminate(Option_t *);
  
  /// set centrality binning
  void SetCentBins(Int_t nBins, Float_t *binLowEdge) {fCentBinLowEdge.Set(nBins+1, binLowEdge);}
  
  /// set pt binning
  void SetPtBins(Int_t nBins, Float_t *binLowEdge) {fPtBinLowEdge.Set(nBins+1, binLowEdge);}
  
  /// set y binning
  void SetYBins(Int_t nBins, Float_t *binLowEdge) {fYBinLowEdge.Set(nBins+1, binLowEdge);}
  
  /// set the number of muons to be matched with the trigger (0, 1 or 2)
  void SetNMatch(Int_t level = 1) {fNMatch = (level>=0 && level<3) ? level : 0;}
  
  /// set the muon pt cut value
  void SetMuLowPtCut(Double_t cut) {fMuLowPtCut = cut;}
  
  /// set the flag to select tracks using MC label (activated by default)
  void UseMCLabel(Bool_t flag = kTRUE) { fUseMCLabel = flag; }
  
  // Set the number of signal versus centrality in a given pt/y bin
  void SetSigWeights(const Char_t *name, Float_t ptMin, Float_t ptMax, Float_t yMin, Float_t yMax,
		     Int_t nCentBin, Float_t *binLowEdge, Double_t *nSig, Bool_t weightRec);
  
  // Set the number of interested events per run
  void LoadRunWeights(const Char_t *fileName);
    
  // set standard cuts to select tracks to be considered
  void SetMuonTrackCuts(AliMuonTrackCuts &trackCuts);
  
  // create the original function with the parameters used in simulation to generate the pT distribution
  void SetOriginPtFunc(TString formula, const Double_t *param, Double_t xMin, Double_t xMax);
  // create the new function with the parameters used to generate the new pT distribution
  void SetNewPtFunc(TString formula, const Double_t *param, Double_t xMin, Double_t xMax,
                    Float_t centMin = -999., Float_t centMax = 999.,
                    Double_t pTMin = -999., Double_t pTMax = 999.,
                    Double_t yMin = -999., Double_t yMax = 999.);
  
  // create the original function with the parameters used in simulation to generate the y distribution
  void SetOriginYFunc(TString formula, const Double_t *param, Double_t xMin, Double_t xMax);
  // create the new function with the parameters used to generate the new y distribution
  void SetNewYFunc(TString formula, const Double_t *param, Double_t xMin, Double_t xMax,
                   Float_t centMin = -999., Float_t centMax = 999.,
                   Double_t pTMin = -999., Double_t pTMax = 999.,
                   Double_t yMin = -999., Double_t yMax = 999.);
  
  /// do not compute the 2D pT-y results
  void Skip2DResults() {f2DRes = kFALSE;}
  
  /// compute only the results integrated over centrality using the first provided set of weights
  void OnlyResultsWeightedOverCent() {fWResOnly = kTRUE;}
  
private:
  
  /// Not implemented
  AliAnalysisTaskJPsiAccEffCorr2(const AliAnalysisTaskJPsiAccEffCorr2& rhs);
  /// Not implemented
  AliAnalysisTaskJPsiAccEffCorr2& operator = (const AliAnalysisTaskJPsiAccEffCorr2& rhs);
  
  /// Remove the weights for the runs that have not been analyzed and warn when a weight is missing
  void CleanRunWeights();
  
  /// Compute acc*eff and binomial errors by hand, i.e. not using TGraphAsymmErrors
  TH1F* ComputeAccEff(TH1F &hGen, TH1F &hRec, const Char_t *name, const Char_t *title);
  
  /// return the number of generated and reconstructed JPsi and the acc*eff correction for the 3 matching requirements
  void GetAccEff(TString selection, Double_t gen[3][2], Double_t rec[3][2], Double_t acc[3][3]);
  
  /// return the number of generated and reconstructed JPsi and the acc*eff correction for the 3 matching requirements
  void IntegratedAccEff(TString commonKey1, const THashList &weights1, Bool_t weightRec1,
			Double_t gen[3][2], Double_t rec[3][2], Double_t acc[3][3],
			const THashList *weights2 = 0x0, Bool_t weightRec2 = kTRUE);
  
  /// compute AccEff correction integrated over the given centrality range
  void IntegratedAccEff(Int_t ipt, Int_t iy, Float_t centMin, Float_t centMax, Int_t nMatch,
			Double_t gen[2], Double_t rec[2], Double_t acc[3], Bool_t print = kTRUE);
  
  /// compute AccEff correction weighted over centrality
  void IntegratedAccEff(Int_t ipt, Int_t iy, Weights &w, Int_t nMatch,
			Double_t gen[2], Double_t rec[2], Double_t acc[3], Bool_t print = kTRUE);
  
  /// fill summary plots
  void FillHistos(Int_t ipt, Int_t iy, Double_t gen[2], Double_t rec[2], Double_t acc[3],
		  TH1F* hGenSum, TH1F* hRecSum, TH1F* hAccSum, Int_t bin);
  
  /// Draw acceptance*efficiency versus run for this given pt/y bin
  void DrawAccEffVsRun(Int_t ipt, Int_t iy, Weights *w, Int_t nMatch);
  
  /// Draw acceptance*efficiency versus centrality for this given pt/y bin
  void DrawAccEffVsCent(Int_t ipt, Int_t iy, Int_t nMatch);
  
  // get the new pT function valid for this centrality
  TF1* GetNewPtFunc(Float_t cent, Double_t pT, Double_t y);
  
  // get the new y function valid for this centrality
  TF1* GetNewYFunc(Float_t cent, Double_t pT, Double_t y);
  
  // normalize the function to its integral in the given range
  void NormFunc(TF1 *f, Double_t min, Double_t max);
  
  private:
  
  enum eList {
    kPtGen   = 0,  ///< pT distribution of generated JPsi
    kPtRec   = 1,  ///< pT distribution of reconstructed JPsi
    kYGen    = 2,  ///< y distribution of generated JPsi
    kYRec    = 3,  ///< y distribution of reconstructed JPsi
    kPtGenMu = 4,  ///< pT distribution of generated muon
    kPtRecMu = 5,  ///< pT distribution of reconstructed muon
    kYGenMu  = 6,  ///< y distribution of generated muon
    kYRecMu  = 7,  ///< y distribution of reconstructed muon
    kDzVtx   = 8,  ///< vertex resolution
    kDzVtx2  = 9,  ///< vertex resolution for event with JPsi
    kMass    = 10, ///< invariant mass distribution
    kDphi    = 11, ///< phi resolution of JPsi
    kCos2Dphi= 12  ///< cos(2.*(phi_rec - phi_MC)) of JPsi
  };
  
  TObjArray*  fList; //!< List of output object
  AliCounterCollection* fEventCounters; //!< event statistics
  AliCounterCollection* fJPsiCounters;  //!< JPsi statistics
  TObjArray*  fMassVspT; //!< List of invariant mass distributions versus pT bin
  TObjArray*  fMassVsy; //!< List of invariant mass distributions versus y bin
  
  TArrayF   fCentBinLowEdge; ///< centrality bin low-edge values
  TArrayF   fPtBinLowEdge;   ///< pT bin low-edge values
  TArrayF   fYBinLowEdge;    ///< y bin low-edge values
  Int_t     fNMatch;         ///< number of muons to be matched with the trigger (0, 1 or 2)
  Double_t  fMuLowPtCut;     ///< muon pt cut value
  Bool_t    fUseMCLabel;     ///< select tracks using MC label
  TList     *fSigWeights;    ///< number of signal versus centrality for different pt/y bins
  THashList *fRunWeights;    ///< number of interested events per run
  Bool_t    f2DRes;          ///< compute 2D pT-y results
  Bool_t    fWResOnly;       ///< only results integrated over cent using the first set of weights
  
  AliMuonTrackCuts* fMuonTrackCuts; ///< cuts to select tracks to be considered
  
  TF1     *fPtFuncOld; ///< original generated pT function with original parameters
  TList   *fPtFuncNew; ///< list of new generated pT fit function with new parameters
  TF1     *fYFuncOld;  ///< original generated y function with original parameters
  TList   *fYFuncNew;  ///< list of new generated y fit function with new parameters
  
  ClassDef(AliAnalysisTaskJPsiAccEffCorr2, 5);
};

//________________________________________________________________________
inline void AliAnalysisTaskJPsiAccEffCorr2::SetMuonTrackCuts(AliMuonTrackCuts &trackCuts)
{
  /// set standard cuts to select tracks to be considered
  delete fMuonTrackCuts;
  fMuonTrackCuts = new AliMuonTrackCuts(trackCuts);
}

#endif

