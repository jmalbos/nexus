// ----------------------------------------------------------------------------
// nexus | HyperonGenerator.cc
//
// This class is the primary generator of a Xe-136 double beta decay,
// interfacing the DECAY0 c++ code, translated from the original
// FORTRAN package, with nexus.
// It provides the primary vertex of a Xe-136 double beta decay.
// The possibility of reading a previously generated ascii file with the
// electron momenta is also allowed.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "HyperonGenerator.h"

#include "DetectorConstruction.h"
#include "GeometryBase.h"
#include "FactoryBase.h"

#include <G4GenericMessenger.hh>
#include <G4RunManager.hh>
#include <G4ParticleTable.hh>
#include <G4ParticleDefinition.hh>
#include <iostream>
using namespace nexus;

REGISTER_CLASS(HyperonGenerator, G4VPrimaryGenerator)

HyperonGenerator::HyperonGenerator():
  G4VPrimaryGenerator(), msg_(0), opened_(false), position_{}, geom_(0)
{

  msg_ = new G4GenericMessenger(this, "/Generator/HyperonGenerator/",
    "Control commands of the anti-nu-mu-hyperon vertex generator interface.");

  msg_->DeclareMethod("inputFile", &HyperonGenerator::OpenInputFile, "");
  msg_->DeclarePropertyWithUnit("position", "mm",  position_, "Set particle vertex.");

  DetectorConstruction* detConst = (DetectorConstruction*)
  G4RunManager::GetRunManager()->GetUserDetectorConstruction();
  geom_ = detConst->GetGeometry();

  myEventCounter_ = 0.;
  energyThreshold_ = 0.;

}



HyperonGenerator::~HyperonGenerator()
{
  if (file_.is_open()) file_.close();
  if (fOutDebug_.is_open()) fOutDebug_.close();
  //if (decay0_ != 0) delete decay0_;
}



void HyperonGenerator::OpenInputFile(G4String filename)
{
   if (filename.find("none") != std::string::npos) {
      std::cerr << " We will use the new interface to HyperonGenerator " << std::endl;
     opened_ = false;
     return;
   }

  file_.open(filename.data());

  if (file_.good()) {
    opened_ = true;
    ProcessHeader();
  }
  else {
    G4Exception("[HyperonGenerator]", "SetInputFile()", JustWarning,
      "Cannot open Hyperon input file.");
  }
}



/// Read an event from file and create primary particles and
/// vertices accordingly
void HyperonGenerator::GeneratePrimaryVertex(G4Event* event)
{
  myEventCounter_++;
  //G4cout << "GeneratePrimaryVertex()" << G4endl;

  // reading event-related information

  // abort if end-of-file was reached in last operation
  if (file_.eof()) {
    G4cout  << "[HyperonGenerator] End-of-File reached. "
            << "Aborting the run..." << G4endl;
    G4RunManager::GetRunManager()->AbortRun();
    return;
  }

  // generate a position in the detector
  // (all primary particles will be generated there)
  //particle_position = geom_->GenerateVertex(region_);


  // reading info for each particle in the event

  G4double kpx, kpy, kpz;    // muon momentum components in MeV
  G4double phx, phy, phz;    // hyperon momentum components in MeV
  G4double weight;           // cross-section weight of the process
  G4int hypID;               // hyperon particle ID. 1: lambda, 2: sigma-, 3: sigma0, 4: sigma+
  G4int hypPDG;              // hyperon particle PDG ID.

  //kpx, kpy, kpz, phx, phy, phz, weight, ihyp
  file_ >> kpx >> kpy >> kpz >> phx >> phy >> phz >> weight >> hypID;

  if      (hypID==1) hypPDG=3122; // Λ
  else if (hypID==2) hypPDG=3222; // Σ+
  else if (hypID==3) hypPDG=3212; // Σ0
  else if (hypID==4) hypPDG=3112; // Σ−
  
  G4ParticleDefinition* mu =
    G4ParticleTable::GetParticleTable()->FindParticle(-13);
  
  G4ParticleDefinition* hyp =
    G4ParticleTable::GetParticleTable()->FindParticle(hypPDG);

  // create a primary particle

  G4PrimaryParticle* p0_mu =
    new G4PrimaryParticle(mu, kpx*GeV, kpy*GeV, kpz*GeV);
  G4PrimaryParticle* p0_hyp =
    new G4PrimaryParticle(hyp, phx*GeV, phy*GeV, phz*GeV);

  p0_mu ->SetMass  (mu ->GetPDGMass());
  p0_mu ->SetCharge(mu ->GetPDGCharge());
  p0_hyp->SetMass  (hyp->GetPDGMass());
  p0_hyp->SetCharge(hyp->GetPDGCharge());

  // create a primary vertex for the particle
  G4PrimaryVertex* vertex =
    new G4PrimaryVertex(position_, 0.); // position, time

  vertex->SetPrimary(p0_mu);
  vertex->SetPrimary(p0_hyp);

  // add vertex to the event
  event->AddPrimaryVertex(vertex);
}

void HyperonGenerator::ProcessHeader()
{
  G4String line;
  getline(file_, line);
}