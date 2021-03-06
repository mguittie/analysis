#ifndef ALIANALYSISMUMU_H
#define ALIANALYSISMUMU_H

#ifndef ALIANALYSISTASKSE_H
#  include "AliAnalysisTaskSE.h"
#endif

#include <set>
#include <vector>

class TList;
class AliHistogramCollection;
class TObjArray;
class AliCounterCollection;

class AliAnalysisMuMu : public AliAnalysisTaskSE
{
public:
  AliAnalysisMuMu();
  AliAnalysisMuMu(TList* triggerClassesToConsider, Bool_t fromESD);
  virtual ~AliAnalysisMuMu();
  
  void UserCreateOutputObjects();

  virtual void Terminate(Option_t *);
  
  virtual void FinishTaskOutput();
  
  virtual void NotifyRun();
  
  virtual void Print(Option_t* opt="") const;
  
  virtual void AddPairCut(const char* cutName, UInt_t maskForOneOrBothTrack, UInt_t maskForTrackPair=0);
  
  virtual void AddSingleCut(const char* cutName, UInt_t mask);

  virtual void AddGlobalEventSelection(const char* name);

  virtual void UserExec(Option_t* opt);

protected:

  virtual void MuUserExec(Option_t* opt) = 0;
  
  void AssertHistogramCollection(const char* physics, const char* triggerClassName);

  void BeautifyHistos();
  
  UInt_t CodePairCutMask(UInt_t maskForOneOrBothTrack, UInt_t maskForTrackPair) const;

  void CreateHisto(TObjArray* array,
                   const char* physics,
                   const char* triggerClassName,
                   const char* hname, const char* htitle, 
                   Int_t nbinsx, Double_t xmin, Double_t xmax,
                   Int_t nbinsy, Double_t ymin, Double_t ymax) const;
  
  void CreateEventHisto(const char* physics,
                        const char* triggerClassName,
                        const char* hname, const char* htitle, 
                        Int_t nbinsx, Double_t xmin, Double_t xmax,
                        Int_t nbinsy=0, Double_t ymin=0.0, Double_t ymax=0.0) const;
  
  void CreateSingleHisto(const char* physics,
                         const char* triggerClassName,
                         const char* hname, const char* htitle, 
                         Int_t nbinsx, Double_t xmin, Double_t xmax,
                         Int_t nbinsy=0, Double_t ymin=0.0, Double_t ymax=0.0) const;

  void CreatePairHisto(const char* physics,
                       const char* triggerClassName,
                       const char* hname, const char* htitle, 
                       Int_t nbinsx, Double_t xmin, Double_t xmax,
                       Int_t nbinsy=0, Double_t ymin=0.0, Double_t ymax=0.0) const;
  
  void DecodePairCutMask(UInt_t code, UInt_t& maskForOneOrBothTrack, UInt_t& maskForTrackPair) const;

  void DefineCentralityClasses();
  
  void DefineDefaultTriggerClasses();

  void FillHistogramCollection(const char* physics, const char* triggerClassName);

  TH1* Histo(const char* physics, const char* histoname);
  
  TH1* Histo(const char* physics, const char* triggerClassName, const char* histoname);
  
  TH1* Histo(const char* physics, const char* triggerClassName, const char* what, const char* histoname);

  TH1* Histo(const char* physics, const char* triggerClassName, const char* cent, const char* what, const char* histoname);

  Double_t MuonMass2() const;
  
  void Plot(const char* physics, const char* triggerClassName);

  enum ETrackCut
  {
    kAll=1<<0,
    kPt=1<<1,
    kRabs=1<<2,
    kMatched=1<<3,
    kMatchedLow=1<<4,
    kMatchedHigh=1<<5,
    kEta=1<<6,
    kChi2=1<<7,
    kDCA=1<<8,
    kPairRapidity=1<<9
  };
  
  const char* DefaultCentralityName() const;
  
  const char* CentralityName(Double_t centrality) const;
  
  Double_t Deg2() const { return 2.0*TMath::DegToRad(); }
  Double_t Deg3() const { return 3.0*TMath::DegToRad(); }
  Double_t Deg10() const { return 10.2*TMath::DegToRad(); }
    
  Double_t AbsZEnd() const { return 505.0; /* cm */ }
  
protected:
  AliHistogramCollection* fHistogramCollection; //! collection of histograms
  TList* fOutput; //! list of output objects
  TList* fTriggerClasses; // trigger classes to consider
  AliCounterCollection* fEventCounters; //! event counters
  Bool_t fIsFromESD; // whether we read from ESD or AOD
  TObjArray* fSingleTrackCutNames; // cut on single tracks (array of TObjString)
  TObjArray* fPairTrackCutNames; // cut on track pairs (array of TObjString)
  std::vector<double> fCentralityLimits; // centrality limits
  TObjArray* fCentralityNames; // names to create histograms
  TObjArray* fGlobalEventSelectionNames; // names of global event selections    
  Bool_t fAA; // not proton-proton
  
  ClassDef(AliAnalysisMuMu,7)
};

#endif

