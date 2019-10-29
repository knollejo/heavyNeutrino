#include "heavyNeutrino/multilep/interface/LeptonAnalyzer.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "FWCore/ParameterSet/interface/FileInPath.h"
#include "heavyNeutrino/multilep/interface/GenTools.h"
#include "heavyNeutrino/multilep/interface/TauTools.h"
#include "TLorentzVector.h"
#include <algorithm>

// TODO: we should maybe stop indentifying effective areas by year, as they are typically more connected to a specific ID than to a specific year
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! we should really take action on this todo
//     it seems at some point the default relIso/miniIso values went back to the old effective areas, simply because to be in sync with ttH leptonMva
//     we should implement ellectronsEffectiveAreas (without splitting up by year) for default values, and separate electronsEffectiveAreasTTH_RelIso2016 and electronsEffectiveAreasTTH_MiniIso2016 or something like that
LeptonAnalyzer::LeptonAnalyzer(const edm::ParameterSet& iConfig, multilep* multilepAnalyzer):
    multilepAnalyzer(multilepAnalyzer),
    electronsEffectiveAreas( ( multilepAnalyzer->is2017() || multilepAnalyzer->is2018() ) ? iConfig.getParameter<edm::FileInPath>("electronsEffectiveAreasFall17").fullPath() : iConfig.getParameter<edm::FileInPath>("electronsEffectiveAreasRelIso2016").fullPath() ),
    electronsEffectiveAreasMiniIso( ( multilepAnalyzer->is2017() || multilepAnalyzer->is2018() ) ? iConfig.getParameter<edm::FileInPath>("electronsEffectiveAreasFall17").fullPath() : iConfig.getParameter<edm::FileInPath>("electronsEffectiveAreasMiniIso2016").fullPath() ),
    muonsEffectiveAreas    ((multilepAnalyzer->is2017() || multilepAnalyzer->is2018() )? (iConfig.getParameter<edm::FileInPath>("muonsEffectiveAreasFall17")).fullPath() : (iConfig.getParameter<edm::FileInPath>("muonsEffectiveAreas")).fullPath() )
{
    leptonMvaComputerTTH = new LeptonMvaHelper(iConfig, true, !multilepAnalyzer->is2016() );
    leptonMvaComputertZq = new LeptonMvaHelper(iConfig, false, !multilepAnalyzer->is2016() );
};

LeptonAnalyzer::~LeptonAnalyzer(){
    delete leptonMvaComputerTTH;
    delete leptonMvaComputertZq;
}

void LeptonAnalyzer::beginJob(TTree* outputTree){
    outputTree->Branch("_nL",                           &_nL,                           "_nL/i");
    outputTree->Branch("_nMu",                          &_nMu,                          "_nMu/i");
    outputTree->Branch("_nEle",                         &_nEle,                         "_nEle/i");
    outputTree->Branch("_nLight",                       &_nLight,                       "_nLight/i");
    outputTree->Branch("_nTau",                         &_nTau,                         "_nTau/i");
    outputTree->Branch("_lPt",                          &_lPt,                          "_lPt[_nL]/D");
    outputTree->Branch("_lEta",                         &_lEta,                         "_lEta[_nL]/D");
    outputTree->Branch("_lEtaSC",                       &_lEtaSC,                       "_lEtaSC[_nLight]/D");
    outputTree->Branch("_lPhi",                         &_lPhi,                         "_lPhi[_nL]/D");
    outputTree->Branch("_lE",                           &_lE,                           "_lE[_nL]/D");
    outputTree->Branch("_lFlavor",                      &_lFlavor,                      "_lFlavor[_nL]/i");
    outputTree->Branch("_lCharge",                      &_lCharge,                      "_lCharge[_nL]/I");
    outputTree->Branch("_dxy",                          &_dxy,                          "_dxy[_nL]/D");
    outputTree->Branch("_dz",                           &_dz,                           "_dz[_nL]/D");
    outputTree->Branch("_3dIP",                         &_3dIP,                         "_3dIP[_nL]/D");
    outputTree->Branch("_3dIPSig",                      &_3dIPSig,                      "_3dIPSig[_nL]/D");
    outputTree->Branch("_lElectronSummer16MvaGP",       &_lElectronMvaSummer16GP,       "_lElectronMvaSummer16GP[_nLight]/F");
    outputTree->Branch("_lElectronSummer16MvaHZZ",      &_lElectronMvaSummer16HZZ,      "_lElectronMvaSummer16HZZ[_nLight]/F");
    outputTree->Branch("_lElectronMvaFall17v1NoIso",    &_lElectronMvaFall17v1NoIso,    "_lElectronMvaFall17v1NoIso[_nLight]/F");
    outputTree->Branch("_lElectronMvaFall17Iso",        &_lElectronMvaFall17Iso,        "_lElectronMvaFall17Iso[_nLight]/F");
    outputTree->Branch("_lElectronMvaFall17NoIso",      &_lElectronMvaFall17NoIso,      "_lElectronMvaFall17NoIso[_nLight]/F");
    outputTree->Branch("_lElectronPassEmu",             &_lElectronPassEmu,             "_lElectronPassEmu[_nLight]/O");
    outputTree->Branch("_lElectronPassConvVeto",        &_lElectronPassConvVeto,        "_lElectronPassConvVeto[_nLight]/O");
    outputTree->Branch("_lElectronChargeConst",         &_lElectronChargeConst,         "_lElectronChargeConst[_nLight]/O");
    outputTree->Branch("_lElectronMissingHits",         &_lElectronMissingHits,         "_lElectronMissingHits[_nLight]/i");
    outputTree->Branch("_lElectronPassMVAFall17NoIsoWP80",    &_lElectronPassMVAFall17NoIsoWP80,    "_lElectronPassMVAFall17NoIsoWP80[_nLight]/O");
    outputTree->Branch("_lElectronPassMVAFall17NoIsoWP90",    &_lElectronPassMVAFall17NoIsoWP90,    "_lElectronPassMVAFall17NoIsoWP90[_nLight]/O");
    outputTree->Branch("_lElectronPassMVAFall17NoIsoWPLoose", &_lElectronPassMVAFall17NoIsoWPLoose, "_lElectronPassMVAFall17NoIsoWPLoose[_nLight]/O");
    outputTree->Branch("_lElectronSigmaIetaIeta",               &_lElectronSigmaIetaIeta,               "_lElectronSigmaIetaIeta[_nLight]/D");
    outputTree->Branch("_lElectronDeltaPhiSuperClusterTrack",   &_lElectronDeltaPhiSuperClusterTrack,   "_lElectronDeltaPhiSuperClusterTrack[_nLight]/D");
    outputTree->Branch("_lElectronDeltaEtaSuperClusterTrack",   &_lElectronDeltaEtaSuperClusterTrack,   "_lElectronDeltaEtaSuperClusterTrack[_nLight]/D");
    outputTree->Branch("_lElectronEInvMinusPInv",               &_lElectronEInvMinusPInv,               "_lElectronEInvMinusPInv[_nLight]/D");
    outputTree->Branch("_lElectronHOverE",                      &_lElectronHOverE,                      "_lElectronHOverE[_nLight]/D");
    outputTree->Branch("_leptonMvaTTH",                 &_leptonMvaTTH,                 "_leptonMvaTTH[_nLight]/D");
    outputTree->Branch("_leptonMvatZq",                 &_leptonMvatZq,                 "_leptonMvatZq[_nLight]/D");
    outputTree->Branch("_lPOGVeto",                     &_lPOGVeto,                     "_lPOGVeto[_nL]/O");
    outputTree->Branch("_lPOGLoose",                    &_lPOGLoose,                    "_lPOGLoose[_nL]/O");
    outputTree->Branch("_lPOGMedium",                   &_lPOGMedium,                   "_lPOGMedium[_nL]/O");
    outputTree->Branch("_lPOGTight",                    &_lPOGTight,                    "_lPOGTight[_nL]/O");
    outputTree->Branch("_tauMuonVetoLoose",             &_tauMuonVetoLoose,             "_tauMuonVetoLoose[_nL]/O");
    outputTree->Branch("_tauEleVetoLoose",              &_tauEleVetoLoose,              "_tauEleVetoLoose[_nL]/O");
    outputTree->Branch("_tauDecayMode",                 &_tauDecayMode,                 "_tauDecayMode[_nL]/i");
    outputTree->Branch("_decayModeFinding",             &_decayModeFinding,             "_decayModeFinding[_nL]/O");
    outputTree->Branch("_tauPOGVTight2017v2",           &_tauPOGVTight2017v2,           "_tauPOGVTight2017v2[_nL]/O");
    outputTree->Branch("_tauAgainstElectronMVA6Raw",    &_tauAgainstElectronMVA6Raw,    "_tauAgainstElectronMVA6Raw[_nL]/D");
    outputTree->Branch("_tauCombinedIsoDBRaw3Hits",     &_tauCombinedIsoDBRaw3Hits,     "_tauCombinedIsoDBRaw3Hits[_nL]/D");
    outputTree->Branch("_tauIsoMVAPWdR03oldDMwLT",      &_tauIsoMVAPWdR03oldDMwLT,      "_tauIsoMVAPWdR03oldDMwLT[_nL]/D");
    outputTree->Branch("_tauIsoMVADBdR03oldDMwLT",      &_tauIsoMVADBdR03oldDMwLT,      "_tauIsoMVADBdR03oldDMwLT[_nL]/D");
    outputTree->Branch("_tauIsoMVADBdR03newDMwLT",      &_tauIsoMVADBdR03newDMwLT,      "_tauIsoMVADBdR03newDMwLT[_nL]/D");
    outputTree->Branch("_tauIsoMVAPWnewDMwLT",          &_tauIsoMVAPWnewDMwLT,          "_tauIsoMVAPWnewDMwLT[_nL]/D");
    outputTree->Branch("_tauIsoMVAPWoldDMwLT",          &_tauIsoMVAPWoldDMwLT,          "_tauIsoMVAPWoldDMwLT[_nL]/D");
    outputTree->Branch("_relIso",                       &_relIso,                       "_relIso[_nLight]/D");
    outputTree->Branch("_relIso0p4",                    &_relIso0p4,                    "_relIso0p4[_nLight]/D");
    outputTree->Branch("_relIso0p4MuDeltaBeta",         &_relIso0p4MuDeltaBeta,         "_relIso0p4MuDeltaBeta[_nMu]/D");
    outputTree->Branch("_miniIso",                      &_miniIso,                      "_miniIso[_nLight]/D");
    outputTree->Branch("_miniIsoCharged",               &_miniIsoCharged,               "_miniIsoCharged[_nLight]/D");
    outputTree->Branch("_ptRel",                        &_ptRel,                        "_ptRel[_nLight]/D");
    outputTree->Branch("_ptRatio",                      &_ptRatio,                      "_ptRatio[_nLight]/D");
    outputTree->Branch("_closestJetCsvV2",              &_closestJetCsvV2,              "_closestJetCsvV2[_nLight]/D");
    outputTree->Branch("_closestJetDeepCsv_b",          &_closestJetDeepCsv_b,          "_closestJetDeepCsv_b[_nLight]/D");
    outputTree->Branch("_closestJetDeepCsv_bb",         &_closestJetDeepCsv_bb,         "_closestJetDeepCsv_bb[_nLight]/D");
    outputTree->Branch("_closestJetDeepCsv",            &_closestJetDeepCsv,            "_closestJetDeepCsv[_nLight]/D");
    outputTree->Branch("_closestJetDeepFlavor_b",       &_closestJetDeepFlavor_b,       "_closestJetDeepFlavor_b[_nLight]/D");
    outputTree->Branch("_closestJetDeepFlavor_bb",      &_closestJetDeepFlavor_bb,      "_closestJetDeepFlavor_bb[_nLight]/D");
    outputTree->Branch("_closestJetDeepFlavor_lepb",    &_closestJetDeepFlavor_lepb,    "_closestJetDeepFlavor_lepb[_nLight]/D");
    outputTree->Branch("_closestJetDeepFlavor",         &_closestJetDeepFlavor,         "_closestJetDeepFlavor[_nLight]/D");
    outputTree->Branch("_selectedTrackMult",            &_selectedTrackMult,            "_selectedTrackMult[_nLight]/i");
    outputTree->Branch("_lMuonSegComp",                 &_lMuonSegComp,                 "_lMuonSegComp[_nMu]/D");
    outputTree->Branch("_lMuonTrackPt",                 &_lMuonTrackPt,                 "_lMuonTrackPt[_nMu]/D");
    outputTree->Branch("_lMuonTrackPtErr",              &_lMuonTrackPtErr,              "_lMuonTrackPtErr[_nMu]/D");
    if( multilepAnalyzer->isMC() ){
        outputTree->Branch("_lIsPrompt",                  &_lIsPrompt,                    "_lIsPrompt[_nL]/O");
        outputTree->Branch("_lMatchPdgId",                &_lMatchPdgId,                  "_lMatchPdgId[_nL]/I");
        outputTree->Branch("_lMatchCharge",               &_lMatchCharge,                 "_lMatchCharge[_nL]/I");
        outputTree->Branch("_tauGenStatus",               &_tauGenStatus,                 "_tauGenStatus[_nL]/i");
        outputTree->Branch("_lMomPdgId",                  &_lMomPdgId,                    "_lMomPdgId[_nL]/I");
        outputTree->Branch("_lProvenance",                &_lProvenance,                  "_lProvenance[_nL]/i");
        outputTree->Branch("_lProvenanceCompressed",      &_lProvenanceCompressed,        "_lProvenanceCompressed[_nL]/i");
        outputTree->Branch("_lProvenanceConversion",      &_lProvenanceConversion,        "_lProvenanceConversion[_nL]/i");
    }
    outputTree->Branch("_lPtCorr",                    &_lPtCorr,                      "_lPtCorr[_nLight]/D");
    outputTree->Branch("_lPtScaleUp",                 &_lPtScaleUp,                   "_lPtScaleUp[_nLight]/D");
    outputTree->Branch("_lPtScaleDown",               &_lPtScaleDown,                 "_lPtScaleDown[_nLight]/D");
    outputTree->Branch("_lPtResUp",                   &_lPtResUp,                     "_lPtResUp[_nLight]/D");
    outputTree->Branch("_lPtResDown",                 &_lPtResDown,                   "_lPtResDown[_nLight]/D");
    outputTree->Branch("_lECorr",                     &_lECorr,                       "_lECorr[_nLight]/D");
    outputTree->Branch("_lEScaleUp",                  &_lEScaleUp,                    "_lEScaleUp[_nLight]/D");
    outputTree->Branch("_lEScaleDown",                &_lEScaleDown,                  "_lEScaleDown[_nLight]/D");
    outputTree->Branch("_lEResUp",                    &_lEResUp,                      "_lEResUp[_nLight]/D");
    outputTree->Branch("_lEResDown",                  &_lEResDown,                    "_lEResDown[_nLight]/D");
    if(multilepAnalyzer->storeAllTauID){
      outputTree->Branch("_decayModeFindingNew",          &_decayModeFindingNew,          "_decayModeFindingNew[_nL]/O");
      outputTree->Branch("_tauPOGVLoose2015",             &_tauPOGVLoose2015,             "_tauPOGVLoose2015[_nL]/O");
      outputTree->Branch("_tauPOGLoose2015",              &_tauPOGLoose2015,              "_tauPOGLoose2015[_nL]/O");
      outputTree->Branch("_tauPOGMedium2015",             &_tauPOGMedium2015,             "_tauPOGMedium2015[_nL]/O");
      outputTree->Branch("_tauPOGTight2015",              &_tauPOGTight2015,              "_tauPOGTight2015[_nL]/O");
      outputTree->Branch("_tauPOGVTight2015",             &_tauPOGVTight2015,             "_tauPOGVTight2015[_nL]/O");
      outputTree->Branch("_tauVLooseMvaNew2015",          &_tauVLooseMvaNew2015,          "_tauVLooseMvaNew2015[_nL]/O");
      outputTree->Branch("_tauLooseMvaNew2015",           &_tauLooseMvaNew2015,           "_tauLooseMvaNew2015[_nL]/O");
      outputTree->Branch("_tauMediumMvaNew2015",          &_tauMediumMvaNew2015,          "_tauMediumMvaNew2015[_nL]/O");
      outputTree->Branch("_tauTightMvaNew2015",           &_tauTightMvaNew2015,           "_tauTightMvaNew2015[_nL]/O");
      outputTree->Branch("_tauVTightMvaNew2015",          &_tauVTightMvaNew2015,          "_tauVTightMvaNew2015[_nL]/O");
      outputTree->Branch("_tauPOGVVLoose2017v2",          &_tauPOGVVLoose2017v2,          "_tauPOGVVLoose2017v2[_nL]/O");
      outputTree->Branch("_tauPOGVVTight2017v2",          &_tauPOGVVTight2017v2,          "_tauPOGVVTight2017v2[_nL]/O");
      outputTree->Branch("_tauVLooseMvaNew2017v2",        &_tauVLooseMvaNew2017v2,        "_tauVLooseMvaNew2017v2[_nL]/O");
      outputTree->Branch("_tauLooseMvaNew2017v2",         &_tauLooseMvaNew2017v2,         "_tauLooseMvaNew2017v2[_nL]/O");
      outputTree->Branch("_tauMediumMvaNew2017v2",        &_tauMediumMvaNew2017v2,        "_tauMediumMvaNew2017v2[_nL]/O");
      outputTree->Branch("_tauTightMvaNew2017v2",         &_tauTightMvaNew2017v2,         "_tauTightMvaNew2017v2[_nL]/O");
      outputTree->Branch("_tauVTightMvaNew2017v2",        &_tauVTightMvaNew2017v2,        "_tauVTightMvaNew2017v2[_nL]/O");
      outputTree->Branch("_tauMuonVetoTight",             &_tauMuonVetoTight,             "_tauMuonVetoTight[_nL]/O");
      outputTree->Branch("_tauEleVetoVLoose",             &_tauEleVetoVLoose,             "_tauEleVetoVLoose[_nL]/O");
      outputTree->Branch("_tauEleVetoMedium",             &_tauEleVetoMedium,             "_tauEleVetoMedium[_nL]/O");
      outputTree->Branch("_tauEleVetoTight",              &_tauEleVetoTight,              "_tauEleVetoTight[_nL]/O");
      outputTree->Branch("_tauEleVetoVTight",             &_tauEleVetoVTight,             "_tauEleVetoVTight[_nL]/O");
     }
}

bool LeptonAnalyzer::analyze(const edm::Event& iEvent, const reco::Vertex& primaryVertex){
    edm::Handle<std::vector<pat::Electron>> electrons          = getHandle(iEvent, multilepAnalyzer->eleToken);
    edm::Handle<std::vector<pat::Muon>> muons                  = getHandle(iEvent, multilepAnalyzer->muonToken);
    edm::Handle<std::vector<pat::Tau>> taus                    = getHandle(iEvent, multilepAnalyzer->tauToken);
    edm::Handle<std::vector<pat::PackedCandidate>> packedCands = getHandle(iEvent, multilepAnalyzer->packedCandidatesToken);
    edm::Handle<double> rho                                    = getHandle(iEvent, multilepAnalyzer->rhoToken);
    edm::Handle<std::vector<pat::Jet>> jets                    = getHandle(iEvent, multilepAnalyzer->jetToken);
  //edm::Handle<std::vector<pat::Jet>> jets                    = getHandle(iEvent, multilepAnalyzer->jetSmearedToken);  // Are we sure we do not want the smeared jets here???
    edm::Handle<std::vector<reco::GenParticle>> genParticles   = getHandle(iEvent, multilepAnalyzer->genParticleToken);

    _nL     = 0;
    _nLight = 0;
    _nMu    = 0;
    _nEle   = 0;
    _nTau   = 0;

    // loop over muons
    // muons need to be run first, because some ID's need to calculate a muon veto for electrons
    for(const pat::Muon& mu : *muons){
        if(_nL == nL_max)                              break;
        if(mu.innerTrack().isNull())                   continue;
        if(mu.pt() < 5)                                continue;
        if(fabs(mu.eta()) > 2.4)                       continue;
        if(!mu.isPFMuon())                             continue;
        if(!(mu.isTrackerMuon() || mu.isGlobalMuon())) continue;
        fillLeptonImpactParameters(mu);
        if(fabs(_dxy[_nL]) > 0.05)                     continue;
        if(fabs(_dz[_nL]) > 0.1)                       continue;
        fillLeptonKinVars(mu);
        if( multilepAnalyzer->isMC() ) fillLeptonGenVars(mu, *genParticles);

        _lFlavor[_nL]        = 1;
        _lMuonSegComp[_nL]    = mu.segmentCompatibility();
        _lMuonTrackPt[_nL]    = mu.innerTrack()->pt();
        _lMuonTrackPtErr[_nL] = mu.innerTrack()->ptError();

        _relIso[_nL]         = getRelIso03(mu, *rho);                     // Isolation variables
        _relIso0p4[_nL]      = getRelIso04(mu, *rho);
        _relIso0p4MuDeltaBeta[_nL] = getRelIso04(mu, *rho, true);
        _miniIso[_nL]        = getMiniIsolation( mu, *rho, false );
        _miniIsoCharged[_nL] = getMiniIsolation( mu, *rho, true );

        _lPOGVeto[_nL]       = mu.passed(reco::Muon::CutBasedIdLoose); // no veto available, so we take loose here
        _lPOGLoose[_nL]      = mu.passed(reco::Muon::CutBasedIdLoose);
        _lPOGMedium[_nL]     = mu.passed(reco::Muon::CutBasedIdMedium);
        _lPOGTight[_nL]      = mu.passed(reco::Muon::CutBasedIdTight);

        //fillLeptonJetVariables MUST be called after setting isolation variables since _relIso0p4 is used to set ptRatio in the absence of a closest jet 
        //tZq lepton MVA uses old matching scheme, first set the lepton jet variables with old matching to compute this MVA
        fillLeptonJetVariables(mu, jets, primaryVertex, *rho, true);
        _leptonMvatZq[_nL]   = leptonMvaVal(mu, leptonMvaComputertZq);

        //the TTH MVA uses a newer matching scheme, so we recompute the lepton jet variables, THIS VERSION IS STORED IN THE NTUPLES
        fillLeptonJetVariables(mu, jets, primaryVertex, *rho, false);
        _leptonMvaTTH[_nL]   = leptonMvaVal(mu, leptonMvaComputerTTH);

        ++_nMu;
        ++_nL;
        ++_nLight;
    }

    // Loop over electrons (note: using iterator we can easily get the ref too)
    for(auto ele = electrons->begin(); ele != electrons->end(); ++ele){
        if(_nL == nL_max)                                                                               break;
        if(ele->gsfTrack().isNull())                                                                    continue;
        if(ele->pt() < 7)                                                                               continue;
        if(fabs(ele->eta()) > 2.5)                                                                      continue;
        if(ele->gsfTrack()->hitPattern().numberOfLostHits(reco::HitPattern::MISSING_INNER_HITS) > 2)    continue;
        fillLeptonImpactParameters(*ele);
        if(fabs(_dxy[_nL]) > 0.05)                                                                      continue;
        if(fabs(_dz[_nL]) > 0.1)                                                                        continue;
        fillLeptonKinVars(*ele);
        if( multilepAnalyzer->isMC() ) fillLeptonGenVars(*ele, *genParticles);

        _lFlavor[_nL]                   = 0;
        _lEtaSC[_nL]                    = ele->superCluster()->eta();

        _relIso[_nL]                    = getRelIso03(*ele, *rho);
        _relIso0p4[_nL]                 = getRelIso04(*ele, *rho);
        _miniIso[_nL]                   = getMiniIsolation( *ele, *rho, false );
        _miniIsoCharged[_nL]            = getMiniIsolation( *ele, *rho, true );
        _lElectronMvaSummer16GP[_nL]    = ele->userFloat("ElectronMVAEstimatorRun2Spring16GeneralPurposeV1Values"); // OLD, do not use it
        _lElectronMvaSummer16HZZ[_nL]   = ele->userFloat("ElectronMVAEstimatorRun2Spring16HZZV1Values"); // OLD, do not use it
        _lElectronMvaFall17v1NoIso[_nL] = ele->userFloat("ElectronMVAEstimatorRun2Fall17NoIsoV1Values"); // OLD, do not use it
        _lElectronMvaFall17Iso[_nL]     = ele->userFloat("ElectronMVAEstimatorRun2Fall17IsoV2Values");
        _lElectronMvaFall17NoIso[_nL]   = ele->userFloat("ElectronMVAEstimatorRun2Fall17NoIsoV2Values");
        _lElectronPassEmu[_nL]          = passTriggerEmulationDoubleEG(&*ele);                             // Keep in mind, this trigger emulation is for 2016 DoubleEG, the SingleEG trigger emulation is different
        _lElectronPassConvVeto[_nL]     = ele->passConversionVeto();
        _lElectronChargeConst[_nL]      = ele->isGsfCtfScPixChargeConsistent();
        _lElectronMissingHits[_nL]      = ele->gsfTrack()->hitPattern().numberOfLostHits(reco::HitPattern::MISSING_INNER_HITS);

        _lPOGVeto[_nL]                  = ele->electronID("cutBasedElectronID-Fall17-94X-V1-veto");
        _lPOGLoose[_nL]                 = ele->electronID("cutBasedElectronID-Fall17-94X-V1-loose");
        _lPOGMedium[_nL]                = ele->electronID("cutBasedElectronID-Fall17-94X-V1-medium");
        _lPOGTight[_nL]                 = ele->electronID("cutBasedElectronID-Fall17-94X-V1-tight");

        _lElectronPassMVAFall17NoIsoWPLoose[_nL] = ele->electronID("mvaEleID-Fall17-noIso-V2-wpLoose");
        _lElectronPassMVAFall17NoIsoWP90[_nL] = ele->electronID("mvaEleID-Fall17-noIso-V2-wp90");
        _lElectronPassMVAFall17NoIsoWP80[_nL] = ele->electronID("mvaEleID-Fall17-noIso-V2-wp80");

        _lElectronSigmaIetaIeta[_nL] = ele->full5x5_sigmaIetaIeta();
        _lElectronDeltaPhiSuperClusterTrack[_nL] = fabs(ele->deltaPhiSuperClusterTrackAtVtx());
        _lElectronDeltaEtaSuperClusterTrack[_nL] = fabs(ele->deltaEtaSuperClusterTrackAtVtx());
        _lElectronEInvMinusPInv[_nL] = (1.0 - ele->eSuperClusterOverP())/ele->correctedEcalEnergy();
        _lElectronHOverE[_nL] = ele->hadronicOverEm();

        //fillLeptonJetVariables MUST be called after setting isolation variables since _relIso0p4 is used to set ptRatio in the absence of a closest jet 
        //tZq lepton MVA uses old matching scheme, first set the lepton jet variables with old matching to compute this MVA
        fillLeptonJetVariables(*ele, jets, primaryVertex, *rho, true);
        _leptonMvatZq[_nL]              = leptonMvaVal(*ele, leptonMvaComputertZq);

        //the TTH MVA uses a newer matching scheme, so we recompute the lepton jet variables, THIS VERSION IS STORED IN THE NTUPLES
        fillLeptonJetVariables(*ele, jets, primaryVertex, *rho, false);
        _leptonMvaTTH[_nL]              = leptonMvaVal(*ele, leptonMvaComputerTTH);

        // Note: for the scale and smearing systematics we use the overall values, assuming we are not very sensitive to these systematics
        // In case these systematics turn out to be important, need to add their individual source to the tree (and propagate to their own templates):
        // https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaMiniAODV2#Energy_Scale_and_Smearing
        _lPtCorr[_nL]                   = ele->pt()*ele->userFloat("ecalTrkEnergyPostCorr")/ele->energy();
        _lPtScaleUp[_nL]                = ele->pt()*ele->userFloat("energyScaleUp")/ele->energy();
        _lPtScaleDown[_nL]              = ele->pt()*ele->userFloat("energyScaleDown")/ele->energy();
        _lPtResUp[_nL]                  = ele->pt()*ele->userFloat("energySigmaUp")/ele->energy();
        _lPtResDown[_nL]                = ele->pt()*ele->userFloat("energySigmaDown")/ele->energy();
        _lECorr[_nL]                    = ele->userFloat("ecalTrkEnergyPostCorr");
        _lEScaleUp[_nL]                 = ele->userFloat("energyScaleUp");
        _lEScaleDown[_nL]               = ele->userFloat("energyScaleDown");
        _lEResUp[_nL]                   = ele->userFloat("energySigmaUp");
        _lEResDown[_nL]                 = ele->userFloat("energySigmaDown");

        ++_nEle;
        ++_nL;
        ++_nLight;
    }

    //Initialize with default values for those electron-only arrays which weren't filled with muons [to allow correct comparison by the test script]
    for(auto array : {_lEtaSC}) std::fill_n(array, _nMu, 0.);
    for(auto array : {_lElectronMvaSummer16GP, _lElectronMvaSummer16HZZ, _lElectronMvaFall17v1NoIso}) std::fill_n(array, _nMu, 0.); // OLD, do not use them
    for(auto array : {_lElectronMvaFall17Iso, _lElectronMvaFall17NoIso}) std::fill_n(array, _nMu, 0.);
    for(auto array : {_lElectronPassMVAFall17NoIsoWPLoose, _lElectronPassMVAFall17NoIsoWP90, _lElectronPassMVAFall17NoIsoWP80}) std::fill_n(array, _nMu, false);
    for(auto array : {_lElectronPassEmu, _lElectronPassConvVeto, _lElectronChargeConst}) std::fill_n(array, _nMu, false);
    for(auto array : {_lElectronMissingHits}) std::fill_n(array, _nMu, 0.);
    for(auto array : {_lElectronSigmaIetaIeta, _lElectronDeltaPhiSuperClusterTrack, _lElectronDeltaEtaSuperClusterTrack, _lElectronEInvMinusPInv, _lElectronHOverE} ) std::fill_n( array, _nMu, 0. );
    for(auto array : {_lPtCorr, _lPtScaleUp, _lPtScaleDown, _lPtResUp, _lPtResDown}) std::fill_n(array, _nMu, 0.);
    for(auto array : {_lECorr, _lEScaleUp, _lEScaleDown, _lEResUp, _lEResDown}) std::fill_n(array, _nMu, 0.);

    //loop over taus
    for(const pat::Tau& tau : *taus){
        if(_nL == nL_max)         break;
        if(tau.pt() < 20)         continue;          // Minimum pt for tau reconstruction
        if(fabs(tau.eta()) > 2.3) continue;
        fillLeptonKinVars(tau);
        
        if(multilepAnalyzer->isMC()) fillTauGenVars(tau, *genParticles);                    //Still needs to be tested
        fillLeptonImpactParameters(tau, primaryVertex);

        _lFlavor[_nL]  = 2;
        _tauDecayMode[_nL] = tau.decayMode();
        _tauMuonVetoLoose[_nL] = tau.tauID("againstMuonLoose3");                                        //Light lepton vetos
        _tauEleVetoLoose[_nL] = tau.tauID("againstElectronLooseMVA6");

        _decayModeFinding[_nL] = tau.tauID("decayModeFinding");                           //old tau ID

        _lPOGVeto[_nL] = tau.tauID("byVLooseIsolationMVArun2017v2DBoldDMwLT2017");
        _lPOGLoose[_nL] = tau.tauID("byLooseIsolationMVArun2017v2DBoldDMwLT2017");
        _lPOGMedium[_nL] = tau.tauID("byMediumIsolationMVArun2017v2DBoldDMwLT2017");
        _lPOGTight[_nL] = tau.tauID("byTightIsolationMVArun2017v2DBoldDMwLT2017");
        _tauPOGVTight2017v2[_nL] = tau.tauID("byVTightIsolationMVArun2017v2DBoldDMwLT2017");


        
        _tauAgainstElectronMVA6Raw[_nL] = tau.tauID("againstElectronMVA6Raw");
        _tauCombinedIsoDBRaw3Hits[_nL]  = tau.tauID("byCombinedIsolationDeltaBetaCorrRaw3Hits");
        _tauIsoMVAPWdR03oldDMwLT[_nL]   = tau.tauID("byIsolationMVArun2v1PWdR03oldDMwLTraw");
        _tauIsoMVADBdR03oldDMwLT[_nL]   = tau.tauID("byIsolationMVArun2v1DBoldDMwLTraw");
        _tauIsoMVADBdR03newDMwLT[_nL]   = tau.tauID("byIsolationMVArun2v1DBnewDMwLTraw");
        _tauIsoMVAPWnewDMwLT[_nL]       = tau.tauID("byIsolationMVArun2v1PWnewDMwLTraw");
        _tauIsoMVAPWoldDMwLT[_nL]       = tau.tauID("byIsolationMVArun2v1PWoldDMwLTraw");

        if(multilepAnalyzer->storeAllTauID){
            _tauMuonVetoTight[_nL] = tau.tauID("againstMuonTight3");                                        //Light lepton vetos
            _tauEleVetoVLoose[_nL] = tau.tauID("againstElectronVLooseMVA6");
            _tauEleVetoMedium[_nL] = tau.tauID("againstElectronMediumMVA6");
            _tauEleVetoTight[_nL] = tau.tauID("againstElectronTightMVA6");
            _tauEleVetoVTight[_nL] = tau.tauID("againstElectronVTightMVA6");
            
            _tauPOGVLoose2015[_nL] = tau.tauID("byVLooseIsolationMVArun2v1DBoldDMwLT");                        
            _tauPOGLoose2015[_nL] = tau.tauID("byLooseIsolationMVArun2v1DBoldDMwLT");
            _tauPOGMedium2015[_nL] = tau.tauID("byMediumIsolationMVArun2v1DBoldDMwLT");
            _tauPOGTight2015[_nL] = tau.tauID("byTightIsolationMVArun2v1DBoldDMwLT");
            _tauPOGVTight2015[_nL] = tau.tauID("byVTightIsolationMVArun2v1DBoldDMwLT");
            
            _tauPOGVVLoose2017v2[_nL] = tau.tauID("byVVLooseIsolationMVArun2017v2DBoldDMwLT2017");
            _tauPOGVVTight2017v2[_nL] = tau.tauID("byVVTightIsolationMVArun2017v2DBoldDMwLT2017");
            
            _decayModeFindingNew[_nL]       = tau.tauID("decayModeFindingNewDMs");                   //new Tau ID
            _tauVLooseMvaNew2015[_nL]           = tau.tauID("byVLooseIsolationMVArun2v1DBnewDMwLT");
            _tauLooseMvaNew2015[_nL]            = tau.tauID("byLooseIsolationMVArun2v1DBnewDMwLT");
            _tauMediumMvaNew2015[_nL]           = tau.tauID("byMediumIsolationMVArun2v1DBnewDMwLT");
            _tauTightMvaNew2015[_nL]            = tau.tauID("byTightIsolationMVArun2v1DBnewDMwLT");
            _tauVTightMvaNew2015[_nL]           = tau.tauID("byVTightIsolationMVArun2v1DBnewDMwLT");

            _tauVLooseMvaNew2017v2[_nL]           = tau.tauID("byVLooseIsolationMVArun2017v2DBnewDMwLT2017");
            _tauLooseMvaNew2017v2[_nL]            = tau.tauID("byLooseIsolationMVArun2017v2DBnewDMwLT2017");
            _tauMediumMvaNew2017v2[_nL]           = tau.tauID("byMediumIsolationMVArun2017v2DBnewDMwLT2017");
            _tauTightMvaNew2017v2[_nL]            = tau.tauID("byTightIsolationMVArun2017v2DBnewDMwLT2017");
            _tauVTightMvaNew2017v2[_nL]           = tau.tauID("byVTightIsolationMVArun2017v2DBnewDMwLT2017");
    
        }

        ++_nTau;
        ++_nL;
    }

    //Initialize with default values for those tau-only arrays which weren't filled with electrons and muons [to allow correct comparison by the test script]
    for(auto array : {&_tauMuonVetoLoose, &_tauEleVetoLoose, &_decayModeFinding, &_decayModeFindingNew}) std::fill_n(*array, _nLight, false);
    for(auto array : {&_tauEleVetoVLoose, &_tauEleVetoMedium, &_tauEleVetoTight, &_tauEleVetoVTight, &_tauMuonVetoTight, &_decayModeFindingNew}) std::fill_n(*array, _nLight, false);
    for(auto array : {&_tauVLooseMvaNew2015, &_tauLooseMvaNew2015, &_tauMediumMvaNew2015, &_tauTightMvaNew2015, &_tauVTightMvaNew2015}) std::fill_n(*array, _nLight, false);
    for(auto array : {&_tauVLooseMvaNew2017v2, &_tauLooseMvaNew2017v2, &_tauMediumMvaNew2017v2, &_tauTightMvaNew2017v2, &_tauVTightMvaNew2017v2}) std::fill_n(*array, _nLight, false);
    for(auto array : {&_tauPOGVLoose2015, &_tauPOGLoose2015, &_tauPOGMedium2015, &_tauPOGTight2015, &_tauPOGVTight2015}) std::fill_n(*array, _nLight, false);
    for(auto array : {&_tauPOGVVLoose2017v2, &_tauPOGVTight2017v2, &_tauPOGVVTight2017v2}) std::fill_n(*array, _nLight, false);
    for(auto array : {&_tauAgainstElectronMVA6Raw, &_tauCombinedIsoDBRaw3Hits, &_tauIsoMVAPWdR03oldDMwLT}) std::fill_n(*array, _nLight, 0.);
    for(auto array : {&_tauDecayMode}) std::fill_n(*array, _nLight, 0.);
    for(auto array : {&_tauIsoMVADBdR03oldDMwLT, &_tauIsoMVADBdR03newDMwLT, &_tauIsoMVAPWnewDMwLT, &_tauIsoMVAPWoldDMwLT}) std::fill_n(*array, _nLight, 0.);

    if(multilepAnalyzer->skim == "trilep"    &&  _nL     < 3) return false;
    if(multilepAnalyzer->skim == "dilep"     &&  _nL     < 2) return false;
    if(multilepAnalyzer->skim == "ttg"       &&  _nLight < 2) return false;
    if(multilepAnalyzer->skim == "singlelep" &&  _nL     < 1) return false;
    if(multilepAnalyzer->skim == "singletau" &&  _nTau   < 1) return false;
    if(multilepAnalyzer->skim == "FR"        &&  _nLight < 1) return false;
    
    return true;
}

void LeptonAnalyzer::fillLeptonKinVars(const reco::Candidate& lepton){
    _lPt[_nL]     = lepton.pt();
    _lEta[_nL]    = lepton.eta();
    _lPhi[_nL]    = lepton.phi();
    _lE[_nL]      = lepton.energy();
    _lCharge[_nL] = lepton.charge();
}

template <typename Lepton> void LeptonAnalyzer::fillLeptonGenVars(const Lepton& lepton, const std::vector<reco::GenParticle>& genParticles){
    const reco::GenParticle* match = lepton.genParticle();
    if(!match || match->pdgId() != lepton.pdgId()) match = GenTools::geometricMatch(lepton, genParticles); // if no match or pdgId is different, try the geometric match

    _tauGenStatus[_nL]          = TauTools::tauGenStatus(match);        
    _lIsPrompt[_nL]             = match && (abs(lepton.pdgId()) == abs(match->pdgId()) || match->pdgId() == 22) && GenTools::isPrompt(*match, genParticles); // only when matched to its own flavor or a photon
    _lMatchPdgId[_nL]           = match != nullptr ? match->pdgId() : 0;
    _lMatchCharge[_nL]          = match != nullptr ? match->charge() : 0;
    _lProvenance[_nL]           = GenTools::provenance(match, genParticles);
    _lProvenanceCompressed[_nL] = GenTools::provenanceCompressed(match, genParticles, _lIsPrompt[_nL]);
    _lProvenanceConversion[_nL] = GenTools::provenanceConversion(match, genParticles);
    _lMomPdgId[_nL]             = match ? GenTools::getMotherPdgId(*match, genParticles) : 0;
}

void LeptonAnalyzer::fillTauGenVars(const pat::Tau& tau, const std::vector<reco::GenParticle>& genParticles){
    
    const reco::GenParticle* match = TauTools::findMatch(tau, genParticles);

    _tauGenStatus[_nL]          = TauTools::tauGenStatus(match);        
    _lIsPrompt[_nL]             = match && _tauGenStatus[_nL] != 6; 
    _lMatchPdgId[_nL]           = match ? match->pdgId() : 0;
    _lProvenance[_nL]           = GenTools::provenance(match, genParticles);
    _lProvenanceCompressed[_nL] = GenTools::provenanceCompressed(match, genParticles, _lIsPrompt[_nL]);
    _lProvenanceConversion[_nL] = GenTools::provenanceConversion(match, genParticles);
    _lMomPdgId[_nL]             = match ? GenTools::getMotherPdgId(*match, genParticles) : 0;

}

/*
 * Impact parameters:
 * Note: dB function seems to be preferred and more accurate over track->dxy and dz functions 
 * as the latter ones have some simplified extrapolation used (leading to slightly different values or opposite sign)
 * For taus: dxy is pre-computed with PV it was constructed with
 */
void LeptonAnalyzer::fillLeptonImpactParameters(const pat::Electron& ele){
    _dxy[_nL]     = ele.dB(pat::Electron::PV2D);
    _dz[_nL]      = ele.dB(pat::Electron::PVDZ);
    _3dIP[_nL]    = ele.dB(pat::Electron::PV3D);
    _3dIPSig[_nL] = fabs(ele.dB(pat::Electron::PV3D)/ele.edB(pat::Electron::PV3D));
}

void LeptonAnalyzer::fillLeptonImpactParameters(const pat::Muon& muon){
    _dxy[_nL]     = muon.dB(pat::Muon::PV2D);
    _dz[_nL]      = muon.dB(pat::Muon::PVDZ);
    _3dIP[_nL]    = muon.dB(pat::Muon::PV3D);
    _3dIPSig[_nL] = fabs(muon.dB(pat::Muon::PV3D)/muon.edB(pat::Muon::PV3D));
}

void LeptonAnalyzer::fillLeptonImpactParameters(const pat::Tau& tau, const reco::Vertex& vertex){
    _dxy[_nL]     = (double) tau.dxy();                                      // warning: float while dxy of tracks are double; could also return -1000
    _dz[_nL]      = tau_dz(tau, vertex.position());
    _3dIP[_nL]    = tau.ip3d();
    _3dIPSig[_nL] = tau.ip3d_Sig();
}

//Function returning tau dz
double LeptonAnalyzer::tau_dz(const pat::Tau& tau, const reco::Vertex::Point& vertex) const{
    const reco::Candidate::Point& tauVtx = tau.leadChargedHadrCand()->vertex();
    return (tauVtx.Z() - vertex.z()) - ((tauVtx.X() - vertex.x())*tau.px()+(tauVtx.Y()-vertex.y())*tau.py())/tau.pt()*tau.pz()/tau.pt();
}


//fucking disgusting lepton-jet matching based on obscure pointer defined somewhere in the abyss of CMSSW. (I feel ashamed for using this, but one has to be in sync with the POG - Willem )
template< typename T1, typename T2 > bool isSourceCandidatePtrMatch( const T1& lhs, const T2& rhs ){
    for( size_t lhsIndex = 0; lhsIndex < lhs.numberOfSourceCandidatePtrs(); ++lhsIndex ){
        auto lhsSourcePtr = lhs.sourceCandidatePtr( lhsIndex );
        for( size_t rhsIndex = 0; rhsIndex < rhs.numberOfSourceCandidatePtrs(); ++rhsIndex ){
            auto rhsSourcePtr = rhs.sourceCandidatePtr( rhsIndex );
            if( lhsSourcePtr == rhsSourcePtr ){
                return true;
            }
        }
    }
    return false;
}


const pat::Jet* findMatchedJet( const reco::Candidate& lepton, const edm::Handle< std::vector< pat::Jet > >& jets, const bool oldMatching ){
    
    //Look for jet that matches with lepton
    const pat::Jet* matchedJetPtr = nullptr;

    //old matching scheme looks for closest jet in terms of delta R, and required this to be within delta R 0.4 of the lepton
    if( oldMatching ){
        for( auto& jet : *jets ){
            if( jet.pt() <= 5 || fabs( jet.eta() ) >= 3 ) continue;
            if( ( matchedJetPtr == nullptr) || reco::deltaR( jet, lepton ) < reco::deltaR( *matchedJetPtr, lepton ) ){
                matchedJetPtr = &jet;
            }
        }
        if( matchedJetPtr != nullptr && reco::deltaR( lepton, *matchedJetPtr ) > 0.4 ){
            matchedJetPtr = nullptr;
        }
    } else {
        for( auto& jet : *jets ){
            if( isSourceCandidatePtrMatch( lepton, jet ) ){

                //immediately returning guarantees that the leading jet matched to the lepton is returned
                return &jet;
            }
        }
    }
    return matchedJetPtr;
}


//compute closest jet variables using new matching scheme 
void LeptonAnalyzer::fillLeptonJetVariables( const reco::Candidate& lepton, edm::Handle< std::vector< pat::Jet > >& jets, const reco::Vertex& vertex, const double rho, const bool oldMatching ){

    //find closest jet based on source candidate pointer matching
    const pat::Jet* matchedJetPtr = findMatchedJet( lepton, jets, oldMatching );

    if( matchedJetPtr == nullptr ){
        if( _lFlavor[_nL] == 1 ){
            _ptRatio[_nL] = ( oldMatching ? 1. : 1. / ( 1. + _relIso0p4MuDeltaBeta[_nL] ) );
        } else{
            _ptRatio[_nL] = ( oldMatching ? 1. : 1. / ( 1. + _relIso0p4[_nL] ) );
        }
        _ptRel[_nL] = 0;
        _selectedTrackMult[_nL] = 0;
        _closestJetDeepFlavor_b[_nL] = 0;
        _closestJetDeepFlavor_bb[_nL] = 0;
        _closestJetDeepFlavor_lepb[_nL] = 0;
        _closestJetDeepFlavor[_nL] = 0;
        _closestJetDeepCsv_b[_nL] = 0;
        _closestJetDeepCsv_bb[_nL] = 0;
        _closestJetDeepCsv[_nL] = 0;
        _closestJetCsvV2[_nL] = 0;
    } else {
        const pat::Jet& jet = *matchedJetPtr;

        auto rawJetP4 = jet.correctedP4("Uncorrected"); 
        auto leptonP4 = lepton.p4();

        bool leptonEqualsJet = ( ( rawJetP4 - leptonP4 ).P() < 1e-4 );

        //if lepton and jet vector are equal set _ptRatio, _ptRel and track multipliticy to defaults 
        if( leptonEqualsJet && !oldMatching ){
            _ptRatio[_nL] = 1;
            _ptRel[_nL] = 0;
            _selectedTrackMult[_nL] = 0;
        } else {

            //remove all corrections above L1 from the lepton
            auto L1JetP4 = jet.correctedP4("L1FastJet");
            double L2L3JEC = jet.pt()/L1JetP4.pt(); 
            auto lepAwareJetP4 = ( L1JetP4 - leptonP4 )*L2L3JEC + leptonP4;

            _ptRatio[_nL] = lepton.pt() / lepAwareJetP4.pt();

            //lepton momentum orthogonal to the jet axis
            //magnitude of cross-product between lepton and rest of jet 
            _ptRel[_nL ] = leptonP4.Vect().Cross( (lepAwareJetP4 - leptonP4 ).Vect().Unit() ).R();

            _selectedTrackMult[_nL] = 0;
            for( const auto daughterPtr : jet.daughterPtrVector() ){
                const pat::PackedCandidate& daughter = *( (const pat::PackedCandidate*) daughterPtr.get() );
            
                if( daughter.charge() == 0 ) continue;
                if( daughter.fromPV() < 2 ) continue;
                if( reco::deltaR( daughter, lepton ) > 0.4 ) continue;
                if( !daughter.hasTrackDetails() ) continue;

                auto daughterTrack = daughter.pseudoTrack();
                if( daughterTrack.pt() <= 1 ) continue;
                if( daughterTrack.hitPattern().numberOfValidHits() < 8 ) continue;
                if( daughterTrack.hitPattern().numberOfValidPixelHits() < 2 ) continue;
                if( daughterTrack.normalizedChi2() >= 5 ) continue;
                if( std::abs( daughterTrack.dz( vertex.position() ) ) >= 17 ) continue;
                if( std::abs( daughterTrack.dxy( vertex.position() ) ) >= 0.2 ) continue;
                ++_selectedTrackMult[_nL];
            }

        }

        //CSVv2 of closest jet
        _closestJetCsvV2[_nL]      = jet.bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags");

        //DeepCSV of closest jet
        _closestJetDeepCsv_b[_nL]  = jet.bDiscriminator("pfDeepCSVJetTags:probb");
        _closestJetDeepCsv_bb[_nL] = jet.bDiscriminator("pfDeepCSVJetTags:probbb");
        _closestJetDeepCsv[_nL]    = _closestJetDeepCsv_b[_nL] + _closestJetDeepCsv_bb[_nL];
        if( std::isnan( _closestJetDeepCsv[_nL] ) ) _closestJetDeepCsv[_nL] = 0.;

        //DeepFlavor b-tag values of closest jet
        _closestJetDeepFlavor_b[_nL] = jet.bDiscriminator("pfDeepFlavourJetTags:probb");
        _closestJetDeepFlavor_bb[_nL] = jet.bDiscriminator("pfDeepFlavourJetTags:probbb");
        _closestJetDeepFlavor_lepb[_nL] = jet.bDiscriminator("pfDeepFlavourJetTags:problepb");
        _closestJetDeepFlavor[_nL] = _closestJetDeepFlavor_b[_nL] + _closestJetDeepFlavor_bb[_nL] + _closestJetDeepFlavor_lepb[_nL];
        if( std::isnan( _closestJetDeepFlavor[_nL] ) ) _closestJetDeepFlavor[_nL] = 0.;
    }
}
