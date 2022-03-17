// ----------------------------------------------------------------------------
// nexus | CsIScintillator.cc
//
// Caesium Iodide Scintillator.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "CsIScintillator.h"

#include "CylinderPointSampler.h"
#include "HexagonPointSampler.h"
#include "MaterialsList.h"
#include "IonizationSD.h"
#include "FactoryBase.h"
#include "PmtR7378A.h"

#include <G4GenericMessenger.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <G4NistManager.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Material.hh>
#include <G4VisAttributes.hh>
#include <G4SDManager.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4UserLimits.hh>
#include <G4SubtractionSolid.hh>
#include <G4Polyhedra.hh>

#include <CLHEP/Units/SystemOfUnits.h>

using namespace nexus;
using namespace CLHEP;

REGISTER_CLASS(CsIScintillator, GeometryBase)

namespace nexus {

  CsIScintillator::CsIScintillator():
    GeometryBase(), radius_(1.*m),length_(1.*m), cylinder_vertex_gen_(0)
  {
    msg_ = new G4GenericMessenger(this, "/Geometry/CsIScintillator/",
      "Control commands of geometry CsI scintillator.");

    G4GenericMessenger::Command& radius_cmd =
      msg_->DeclareProperty("radius", radius_, "Radius of the CsI cylinder.");

    radius_cmd.SetUnitCategory("Length");
    radius_cmd.SetParameterName("radius", false);
    radius_cmd.SetRange("radius>0.");

    G4GenericMessenger::Command& length_cmd =
      msg_->DeclareProperty("length", length_, "Length of the CsI cylinder.");
    length_cmd.SetUnitCategory("Length");
    length_cmd.SetParameterName("length", false);
    length_cmd.SetRange("length>0.");

    // Create a vertex generator for a cylinder
    cylinder_vertex_gen_ = new CylinderPointSampler(radius_, length_, 0., 0.);

  }



  CsIScintillator::~CsIScintillator()
  {
    delete cylinder_vertex_gen_;
    delete msg_;
  }



  void CsIScintillator::Construct()
  {

   G4double world_hx = 1.0*m;
   G4double world_hy = 1.0*m;
   G4double world_hz = 1.0*m;

   G4Box* worldBox
      = new G4Box("World", world_hx, world_hy, world_hz);

    G4Material* air =
      G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");

    G4LogicalVolume* worldLog
    = new G4LogicalVolume(worldBox, air, "World");

    G4String name = "CSI_CYLINDER";

    // Define solid volume as a sphere
    G4Tubs* cylinder_solid = new G4Tubs("NaI", 0, radius_, length_/2., 0., twopi);

    G4Material* mat =
      G4NistManager::Instance()->FindOrBuildMaterial("G4_CESIUM_IODIDE");
    G4LogicalVolume* sc_logic = new G4LogicalVolume(cylinder_solid, mat, name);
    sc_logic->SetUserLimits(new G4UserLimits(1.*mm));

    this->SetLogicalVolume(sc_logic);


    // Set the logical volume of the cylinder as an ionization
    // sensitive detector, i.e. position, time and energy deposition
    // will be stored for each step of any charged particle crossing
    // the volume.
    G4SDManager* sdmgr = G4SDManager::GetSDMpointer();
    
    IonizationSD* ionisd = new IonizationSD("/CsI");
    ionisd->IncludeInTotalEnergyDeposit(false);
    sdmgr->AddNewDetector(ionisd);
    sc_logic->SetSensitiveDetector(ionisd);

    // Add the Volume to the world volume
    G4VPhysicalVolume* CsIPhys = new G4PVPlacement(0,                       // no rotation
                      G4ThreeVector(0, 0, 0),
                                               // translation position
                      sc_logic,              // its logical volume
                      "CsI",               // its name
                      worldLog,                // its mother (logical) volume
                      false,                   // no boolean operations
                      0);                      // its copy number

    // PMT HOLDER //////////////////////////////////////////////////////

    // Z position of the PMT holder
    G4double transl_z = length_/2.0-10*cm;

    // Load the geometry model of the PMT and get the pointer
    // to its logical volume
    PmtR7378A pmt;
    G4LogicalVolume* pmt_logic_;
    pmt.Construct();
    pmt_logic_ = pmt.GetLogicalVolume();

    // The PMTs are placed in the holder in a honeycomb arrangement.
    // We use the hexagon point sampler to calculate the positions of the
    // PMTs given the pitch.
    const G4double ltube_diam_ = 160. * mm;
    const G4double pmt_pitch_ = 34.59 * mm;
    std::vector<G4ThreeVector> pmt_positions_;
    HexagonPointSampler hexsampler(ltube_diam_/2., 0., 0., G4ThreeVector(0.,0.,0.));
    hexsampler.TesselateWithFixedPitch(pmt_pitch_, pmt_positions_);

    // Loop over the vector of positions
    for (unsigned int i = 0; i<pmt_positions_.size(); i++) {


      // Place the PMT.
      // Notice that the PMT is not positioned inside the holder but
      // inside the gas. Therefore, the position (to be specific, the Z
      // coordinate) must be given in coordinates relative to the world.
      new G4PVPlacement(0, G4ThreeVector(pmt_positions_[i].x(),
                  pmt_positions_[i].y(),
                  transl_z),
              pmt_logic_, "PMT", sc_logic, false, i, false);
    }

    // Finish with the positioning of the PMT holder

    // G4LogicalVolume* pmtholder_logic =
    //   new G4LogicalVolume(pmtholder_solid, teflon_, "PMT_HOLDER_CATHODE");

    // new G4PVPlacement(0, G4ThreeVector(0.,0.,transl_z), pmtholder_logic,
    //         "PMT_HOLDER_CATHODE", sc_logic, false, 0);

  }



  G4ThreeVector CsIScintillator::GenerateVertex(const G4String& region) const
  {
    return cylinder_vertex_gen_->GenerateVertex(region);
  }


} // end namespace nexus
