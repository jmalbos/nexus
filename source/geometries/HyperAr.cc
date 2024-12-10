#include "HyperAr.h"

#include "MaterialsList.h"
#include "GenericPhotosensor.h"
#include "Visibilities.h"

#include "OpticalMaterialProperties.h"
#include "UniformElectricDriftField.h"
#include "IonizationSD.h"
#include "FactoryBase.h"

#include <G4Box.hh>
#include <G4Polyhedra.hh>
#include <G4SystemOfUnits.hh>
#include <G4GenericMessenger.hh>
#include <G4Tubs.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4PVReplica.hh>
#include <G4PVDivision.hh>
#include <G4SDManager.hh>
#include <G4NistManager.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4VisAttributes.hh>
#include <G4UserLimits.hh>
#include <Randomize.hh>

namespace nexus {

  REGISTER_CLASS(HyperAr, GeometryBase)

  G4ThreadLocal G4UniformMagField* HyperAr::magnetic_field_ = nullptr;
  G4ThreadLocal G4FieldManager* HyperAr::field_mgr_ = nullptr;

  HyperAr::HyperAr():
    GeometryBase()
  {
  }


  void HyperAr::Construct()
  {
    // HALL /////////////////////////////////////////////////////////

    const G4double hall_size = 100. * m;

    G4Box* hall_solid_vol = 
      new G4Box("HALL", hall_size/2., hall_size/2., hall_size/2.);

    G4LogicalVolume* hall_logic_vol =
      new G4LogicalVolume(hall_solid_vol, materials::Air(), "HALL");

    hall_logic_vol->SetVisAttributes(G4VisAttributes::GetInvisible());

    GeometryBase::SetLogicalVolume(hall_logic_vol);
    
    G4Material* gar = materials::GAr(10.*bar);
    G4Material* lar = G4NistManager::Instance()->FindOrBuildMaterial("G4_lAr");
    
    //test box to check whether materials work as expected
    G4Box* testbox_solid = new G4Box("TESTBOX", 50.*m,50.*m,50.*m);
    G4LogicalVolume* testbox_lv = new G4LogicalVolume(testbox_solid, lar, "TESTBOX_LV");
    
    //G4UserLimits* fStepLimit = new G4UserLimits(1.0*m);
    //fStepLimit->SetUserMinRange(1.0 * m);
    //testbox_lv->SetUserLimits(fStepLimit);
    new G4PVPlacement (nullptr, G4ThreeVector(0., 0., 0.), testbox_lv, "TESTBOX", hall_logic_vol, false, 0, true);

    // Define this volume as an ionization sensitive detector
    IonizationSD* sensdet_test = new IonizationSD("/HYPER_AR/TESTACTIVE");
    testbox_lv->SetSensitiveDetector(sensdet_test);
    G4SDManager::GetSDMpointer()->AddNewDetector(sensdet_test);


  }



  G4ThreeVector HyperAr::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector theVertex(1.*cm,0.,0.);
    
    if(region=="CENTER") theVertex = G4ThreeVector(0.,0.,0.);
    
    else if(region=="CENTRAL_AXIS") {
      G4double x_r0axis = -2500. * mm + G4UniformRand()*5000.*mm;
      theVertex = G4ThreeVector(x_r0axis, 0., 0.);
    }
    
    return theVertex;
  }


} // end namespace nexus
