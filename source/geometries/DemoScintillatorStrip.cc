// ----------------------------------------------------------------------------
// nexus | DemoScintillatorStrip.cc
//
// 
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "DemoScintillatorStrip.h"
#include "FactoryBase.h"

#include <G4GenericMessenger.hh>
#include <GeometryBase.h>
#include <G4Box.hh>
#include <G4Material.hh>
#include <G4NistManager.hh>
#include <G4LogicalVolume.hh>
#include <G4VisAttributes.hh>


namespace nexus {

  REGISTER_CLASS(DemoScintillatorStrip, GeometryBase)


  DemoScintillatorStrip::DemoScintillatorStrip(): GeometryBase()
  {
    msg_ = new G4GenericMessenger(this, "/Geometry/DemoScintillatorStrip/",
                                  "Control commands of the DEMO scintillator strip geometry.");
  }


  DemoScintillatorStrip::~DemoScintillatorStrip()
  {
    delete msg_;
  }


 void DemoScintillatorStrip::Construct()
 {
  G4double strip_length = 40. * cm;
  G4double strip_width  =  5. * cm;
  G4double strip_thickn =  1. * cm;

  G4Box* strip_solid_vol = 
    new G4Box("SCINT_STRIP", strip_width/2., strip_thickn/2., strip_length/2.);
 
  auto strip_material = 
    G4NistManager::Instance()->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

  G4LogicalVolume* strip_logic_vol = 
    new G4LogicalVolume(strip_solid_vol, strip_material, "SCINT_STRIP");
  GeometryBase::SetLogicalVolume(strip_logic_vol);
 }
 
}
