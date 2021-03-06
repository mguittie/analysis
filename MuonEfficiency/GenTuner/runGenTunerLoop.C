/*
 *  runGenTunerLoop.C
 *  aliroot_dev
 *
 *  Created by philippe pillot on 08/03/13.
 *  Copyright 2013 SUBATECH. All rights reserved.
 *
 */

#if __has_include("/Users/pillot/Work/Alice/Macros/Facilities/runTaskFacilities.C")
#include "/Users/pillot/Work/Alice/Macros/Facilities/runTaskFacilities.C"
#else
#include "runTaskFacilities.C"
#endif

void ShowResults(Int_t nStep);

//______________________________________________________________________________
void runGenTunerLoop(TString smode = "local", TString inputFileName = "AliAOD.root", Int_t nStep = 5)
{
  /// run the generator tuner in a loop
  
  if (nStep <= 0) return;
  
  // --- copy files needed for this analysis ---
  TList pathList; pathList.SetOwner();
  pathList.Add(new TObjString("$WORK/Macros/MuonEfficiency/GenTuner"));
  TList fileList; fileList.SetOwner();
  fileList.Add(new TObjString("runGenTunerLoop.C"));
  fileList.Add(new TObjString("runGenTuner.C"));
  CopyFileLocally(pathList, fileList);
  TString referenceDataFile = gSystem->GetFromPipe("egrep '^[ ]*TString referenceDataFile' runGenTuner.C | cut -d'\"' -f2");
  CopyInputFileLocally(referenceDataFile.Data(), "ReferenceResults.root");
  fileList.Add(new TObjString("ReferenceResults.root"));
  TString runWeight = gSystem->GetFromPipe("egrep '^[ ]*TString runWeight' runGenTuner.C | cut -d'\"' -f2");
  if (!runWeight.IsNull()) fileList.Add(new TObjString(runWeight.Data()));
  
  // --- general analysis setup ---
  TString aliphysicsVersion = gSystem->GetFromPipe("egrep '^[ ]*TString aliphysicsVersion' runGenTuner.C | cut -d'\"' -f2");
  
  // --- saf3 specific setup ---
  Bool_t splitDataset = kFALSE;
  Bool_t overwriteDataset = kFALSE;
  
  // --- Check runing mode ---
  Int_t mode = GetMode(smode, inputFileName);
  if(mode < 0) {
    Error("runGenTunerLoop","Please provide either an AOD root file a collection of AODs or a dataset.");
    return;
  }
  
  // --- run the analysis (saf3 is a special case as the analysis is launched on the server) ---
  if (mode == kSAF3Connect && !splitDataset) {
    
    if (smode == "saf3" && !RunAnalysisOnSAF3(fileList, aliphysicsVersion, inputFileName, splitDataset)) return;
    else if (smode == "vaf" && !RunAnalysisOnVAF(fileList, aliphysicsVersion, inputFileName, splitDataset)) return;
    
  } else {
    
    TString resume = "";
    TGraphErrors *gOldPtParam[100], *gOldPtParamMC[100], *gNewPtParam[100];
    TGraphErrors *gOldYParam[100], *gOldYParamMC[100], *gNewYParam[100];
    TGraphErrors *gOldMuPlusFrac, *gNewMuPlusFrac;
    Int_t nPtParam = 0, nYParam = 0;
    for (Int_t iStep = 0; iStep < nStep; iStep++) {
      
      // resume or not
      TString inFileName = Form("Results_step%d.root",iStep);
      if (resume != "n") {
        if (!gSystem->AccessPathName(inFileName.Data())) {
          if (resume != "y") {
            cout<<"Results already exist. Do you want to resume? [y/n] (if not previous results will be deleted) "<<flush;
            do {resume.Gets(stdin,kTRUE);} while (resume != "y" && resume != "n");
            if (resume == "n") gSystem->Exec("rm -f Results_step*.root");
          }
        } else resume = "n";
      }
      
      // synchronize the existing results in case each step is run one-by-one on aaf
      if (iStep == 0 && mode == kSAF3Connect) {
        TString aafdir = gSystem->ExpandPathName(Form("$HOME/%s", smode.Data()));
        TString remoteLocation = gSystem->pwd();
        remoteLocation.ReplaceAll(gSystem->Getenv("HOME"),aafdir.Data());
        gSystem->Exec(Form("rm -f %s/Results_step*.root", remoteLocation.Data()));
        gSystem->Exec(Form("cp Results_step*.root %s/.", remoteLocation.Data()));
      }
      
      // run the generator tuner
      if (resume != "y") gSystem->Exec(Form("root -b -q runGenTuner.C\\(\\\"%s\\\",\\\"%s\\\",%d,%d,%d,\\\'k\\\'\\)",smode.Data(), inputFileName.Data(), iStep, splitDataset, overwriteDataset));
      
      // get the new generator parameters and fill trending plots
      TFile *inFile = TFile::Open(inFileName.Data(),"READ");
      if (inFile && inFile->IsOpen()) {
        
        // prepare trending plots for pT parameters
        TF1 *fNewPtFunc = static_cast<TF1*>(inFile->FindObjectAny("fPtFuncNew"));
        if (iStep == 0 && fNewPtFunc) {
          nPtParam = fNewPtFunc->GetNpar();
          for (Int_t i = 0; i < nPtParam; i++) {
            gOldPtParam[i] = new TGraphErrors(nStep-1);
            gOldPtParam[i]->SetNameTitle(Form("gOldPtParam%d",i), Form("p%d",i));
            gOldPtParamMC[i] = new TGraphErrors(nStep-1);
            gOldPtParamMC[i]->SetNameTitle(Form("gOldPtParamMC%d",i), Form("p%d",i));
            gNewPtParam[i] = new TGraphErrors(nStep);
            gNewPtParam[i]->SetNameTitle(Form("gNewPtParam%d",i), Form("p%d",i));
          }
        }
        
        // fill trending plots with pT parameters
        TF1 *fOldPtFunc = static_cast<TF1*>(inFile->FindObjectAny("fPtFunc"));
        TF1 *fOldPtFuncMC = static_cast<TF1*>(inFile->FindObjectAny("fPtFuncMC"));
        for (Int_t j = 0; j < nPtParam; j++) {
          if (iStep > 0 && fOldPtFunc) {
            gOldPtParam[j]->SetPoint(iStep-1, iStep-1, fOldPtFunc->GetParameter(j));
            //gOldPtParam[j]->SetPointError(iStep-1, 0., fOldPtFunc->GetParError(j));
          }
          if (iStep > 0 && fOldPtFuncMC) {
            gOldPtParamMC[j]->SetPoint(iStep-1, iStep-1, fOldPtFuncMC->GetParameter(j));
            //gOldPtParamMC[j]->SetPointError(iStep-1, 0., fOldPtFuncMC->GetParError(j));
          }
          if (fNewPtFunc) {
            gNewPtParam[j]->SetPoint(iStep, iStep, fNewPtFunc->GetParameter(j));
            //gNewPtParam[j]->SetPointError(iStep, 0., fNewPtFunc->GetParError(j));
          }
        }
        
        // prepare trending plots for y parameters
        TF1 *fNewYFunc = static_cast<TF1*>(inFile->FindObjectAny("fYFuncNew"));
        if (iStep == 0 && fNewYFunc) {
          nYParam = fNewYFunc->GetNpar();
          for (Int_t i = 0; i < nYParam; i++) {
            gOldYParam[i] = new TGraphErrors(nStep-1);
            gOldYParam[i]->SetNameTitle(Form("gOldYParam%d",i), Form("p%d",i));
            gOldYParamMC[i] = new TGraphErrors(nStep-1);
            gOldYParamMC[i]->SetNameTitle(Form("gOldYParamMC%d",i), Form("p%d",i));
            gNewYParam[i] = new TGraphErrors(nStep);
            gNewYParam[i]->SetNameTitle(Form("gNewYParam%d",i), Form("p%d",i));
          }
        }
        
        // fill trending plots with y parameters
        TF1 *fOldYFunc = static_cast<TF1*>(inFile->FindObjectAny("fYFunc"));
        TF1 *fOldYFuncMC = static_cast<TF1*>(inFile->FindObjectAny("fYFuncMC"));
        for (Int_t j = 0; j < nYParam; j++) {
          if (iStep > 0 && fOldYFunc) {
            gOldYParam[j]->SetPoint(iStep-1, iStep-1, fOldYFunc->GetParameter(j));
            //gOldYParam[j]->SetPointError(iStep-1, 0., fOldYFunc->GetParError(j));
          }
          if (iStep > 0 && fOldYFuncMC) {
            gOldYParamMC[j]->SetPoint(iStep-1, iStep-1, fOldYFuncMC->GetParameter(j));
            //gOldYParamMC[j]->SetPointError(iStep-1, 0., fOldYFuncMC->GetParError(j));
          }
          if (fNewYFunc) {
            gNewYParam[j]->SetPoint(iStep, iStep, fNewYFunc->GetParameter(j));
            //gNewYParam[j]->SetPointError(iStep, 0., fNewYFunc->GetParError(j));
          }
        }
        
        // prepare trending plots for mu+ fraction
        TParameter<Double_t> *newMuPlusFrac = static_cast<TParameter<Double_t>*>(inFile->FindObjectAny("newMuPlusFrac"));
        if (iStep == 0 && newMuPlusFrac) {
          gOldMuPlusFrac = new TGraphErrors(nStep-1);
          gOldMuPlusFrac->SetNameTitle("gOldMuPlusFrac", "mu+ fraction");
          gNewMuPlusFrac = new TGraphErrors(nStep);
          gNewMuPlusFrac->SetNameTitle("gNewMuPlusFrac", "mu+ fraction");
        }
        
        // fill trending plots with mu+ fractions
        TParameter<Double_t> *oldMuPlusFrac = static_cast<TParameter<Double_t>*>(inFile->FindObjectAny("currentMuPlusFrac"));
        if (iStep > 0 && oldMuPlusFrac) {
          gOldMuPlusFrac->SetPoint(iStep-1, iStep-1, oldMuPlusFrac->GetVal());
        }
        if (newMuPlusFrac) {
          gNewMuPlusFrac->SetPoint(iStep, iStep, newMuPlusFrac->GetVal());
        }
        
        inFile->Close();
        
      }
      
    }
    
    // display trending plots
    Int_t cPtnx = TMath::Max(1,(nPtParam+1)/2);
    TCanvas *cPtParams = new TCanvas("cPtParams", "cPtParams", 200*cPtnx, 400);
    cPtParams->Divide(cPtnx,2);
    for (Int_t i = 0; i < nPtParam; i++) {
      cPtParams->cd(i+1);
      gNewPtParam[i]->SetMarkerStyle(kFullDotMedium);
      gNewPtParam[i]->SetMarkerColor(2);
      gNewPtParam[i]->SetLineColor(2);
      if (nStep > 1) {
        gOldPtParam[i]->SetMarkerStyle(kFullDotMedium);
        gOldPtParam[i]->SetMarkerColor(4);
        gOldPtParam[i]->SetLineColor(4);
        gOldPtParam[i]->Draw("ap");
        gOldPtParam[i]->GetXaxis()->SetLimits(-1., nStep+1.);
        gOldPtParamMC[i]->SetMarkerStyle(kFullDotMedium);
        gOldPtParamMC[i]->SetMarkerColor(3);
        gOldPtParamMC[i]->SetLineColor(3);
        gOldPtParamMC[i]->Draw("p");
        gNewPtParam[i]->Draw("p");
      } else {
        gNewPtParam[i]->Draw("ap");
        gNewPtParam[i]->GetXaxis()->SetLimits(-1., nStep+1.);
      }
    }
    Int_t cYnx = TMath::Max(1,(nYParam+1)/2);
    TCanvas *cYParams = new TCanvas("cYParams", "cYParams", 200*cYnx, 400);
    cYParams->Divide(cYnx,2);
    for (Int_t i = 0; i < nYParam; i++) {
      cYParams->cd(i+1);
      gNewYParam[i]->SetMarkerStyle(kFullDotMedium);
      gNewYParam[i]->SetMarkerColor(2);
      gNewYParam[i]->SetLineColor(2);
      if (nStep > 1) {
        gOldYParam[i]->SetMarkerStyle(kFullDotMedium);
        gOldYParam[i]->SetMarkerColor(4);
        gOldYParam[i]->SetLineColor(4);
        gOldYParam[i]->Draw("ap");
        gOldYParam[i]->GetXaxis()->SetLimits(-1., nStep+1.);
        gOldYParamMC[i]->SetMarkerStyle(kFullDotMedium);
        gOldYParamMC[i]->SetMarkerColor(3);
        gOldYParamMC[i]->SetLineColor(3);
        gOldYParamMC[i]->Draw("p");
        gNewYParam[i]->Draw("p");
      } else {
        gNewYParam[i]->Draw("ap");
        gNewYParam[i]->GetXaxis()->SetLimits(-1., nStep+1.);
      }
    }
    TCanvas *cMuPlusFrac = new TCanvas("cMuPlusFrac", "cMuPlusFrac", 200, 200);
    cMuPlusFrac->cd();
    gNewMuPlusFrac->SetMarkerStyle(kFullDotMedium);
    gNewMuPlusFrac->SetMarkerColor(2);
    gNewMuPlusFrac->SetLineColor(2);
    if (nStep > 1) {
      gOldMuPlusFrac->SetMarkerStyle(kFullDotMedium);
      gOldMuPlusFrac->SetMarkerColor(4);
      gOldMuPlusFrac->SetLineColor(4);
      gOldMuPlusFrac->Draw("ap");
      gOldMuPlusFrac->GetXaxis()->SetLimits(-1., nStep+1.);
      gNewMuPlusFrac->Draw("p");
    } else {
      gNewMuPlusFrac->Draw("ap");
      gNewMuPlusFrac->GetXaxis()->SetLimits(-1., nStep+1.);
    }
    
    // save trending plots
    TString inFileName = Form("Results_step%d.root",nStep-1);
    TFile *inFile = TFile::Open(inFileName.Data(),"UPDATE");
    if (inFile && inFile->IsOpen()) {
      cPtParams->Write(0x0, TObject::kOverwrite);
      cYParams->Write(0x0, TObject::kOverwrite);
      cMuPlusFrac->Write(0x0, TObject::kOverwrite);
    }
    inFile->Close();
    
  }
  
  // print and plot last results
  if (gSystem->GetFromPipe("hostname") != "nansafmaster3.in2p3.fr") ShowResults(nStep);
  
}

//______________________________________________________________________________
void ShowResults(Int_t nStep)
{
  /// print and plot last results
  
  TString inFileName = Form("Results_step%d.root",nStep-1);
  TFile *inFile = TFile::Open(inFileName.Data(),"READ");
  if (inFile && inFile->IsOpen()) {
    
    TF1 *fPtFuncNew = static_cast<TF1*>(inFile->FindObjectAny("fPtFuncNew"));
    TF1 *fYFuncNew = static_cast<TF1*>(inFile->FindObjectAny("fYFuncNew"));
    if (fPtFuncNew && fYFuncNew) {
      printf("\npT parameters for single muon generator:\n");
      printf("Double_t p[%d] = {", fPtFuncNew->GetNpar());
      for (Int_t i = 0; i < fPtFuncNew->GetNpar()-1; i++) printf("%g, ", fPtFuncNew->GetParameter(i));
      printf("%g};\n", fPtFuncNew->GetParameter(fPtFuncNew->GetNpar()-1));
      printf("\ny parameters for single muon generator:\n");
      printf("Double_t p[%d] = {", fYFuncNew->GetNpar());
      for (Int_t i = 0; i < fYFuncNew->GetNpar()-1; i++) printf("%g, ", fYFuncNew->GetParameter(i));
      printf("%g};\n", fYFuncNew->GetParameter(fYFuncNew->GetNpar()-1));
    }
    
    TParameter<Double_t> *newMuPlusFrac = static_cast<TParameter<Double_t>*>(inFile->FindObjectAny("newMuPlusFrac"));
    if (newMuPlusFrac) {
      printf("\nfraction of mu+ for single muon generator:\n");
      printf("Double_t newMuPlusFrac = %f\n\n", newMuPlusFrac->GetVal());
    }
    
    TCanvas *cRes = static_cast<TCanvas*>(inFile->FindObjectAny("cRes"));
    if (cRes) cRes->Draw();
    
    TCanvas *cRat = static_cast<TCanvas*>(inFile->FindObjectAny("cRat"));
    if (cRat) cRat->Draw();
    
    TCanvas *cPtParams = static_cast<TCanvas*>(inFile->FindObjectAny("cPtParams"));
    if (cPtParams) cPtParams->Draw();
    
    TCanvas *cYParams = static_cast<TCanvas*>(inFile->FindObjectAny("cYParams"));
    if (cYParams) cYParams->Draw();
    
    TCanvas *cMuPlusFrac = static_cast<TCanvas*>(inFile->FindObjectAny("cMuPlusFrac"));
    if (cMuPlusFrac) cMuPlusFrac->Draw();
    
  }
  inFile->Close();
  
}

