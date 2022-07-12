#include "NDGAr.h"

#include "MaterialsList.h"
#include "GenericPhotosensor.h"
#include "Visibilities.h"

#include "OpticalMaterialProperties.h"
#include "UniformElectricDriftField.h"
#include "IonizationSD.h"
#include "FactoryBase.h"

#include <G4Box.hh>
#include <G4SystemOfUnits.hh>
#include <G4GenericMessenger.hh>
#include <G4Tubs.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4NistManager.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4VisAttributes.hh>


namespace nexus {

  REGISTER_CLASS(NDGAr, GeometryBase)

  G4ThreadLocal G4UniformMagField* NDGAr::magnetic_field_ = nullptr;
  G4ThreadLocal G4FieldManager* NDGAr::field_mgr_ = nullptr;

  NDGAr::NDGAr():
    GeometryBase()
  {
  }


  void NDGAr::Construct()
  {
    // HALL /////////////////////////////////////////////////////////

    const G4double hall_size = 20. * m;

    G4Box* hall_solid_vol = 
      new G4Box("HALL", hall_size/2., hall_size/2., hall_size/2.);

    G4LogicalVolume* hall_logic_vol =
      new G4LogicalVolume(hall_solid_vol, materials::Air(), "HALL");

    hall_logic_vol->SetVisAttributes(G4VisAttributes::GetInvisible());

    GeometryBase::SetLogicalVolume(hall_logic_vol);

    // NDGAR ////////////////////////////////////////////////////////

    const G4double ndgar_diam   = 8.0 * m;
    const G4double ndgar_length = 8.0 * m;

    G4Tubs* ndgar_solid_vol =
      new G4Tubs("NDGAR", 0., ndgar_diam/2., ndgar_length/2., 0., 360.*deg);

    G4Material* argon = materials::GAr(10.*bar);
    auto argon_mpt = opticalprops::GAr(0./MeV);
    argon->SetMaterialPropertiesTable(argon_mpt);

    G4LogicalVolume* ndgar_logic_vol =
      new G4LogicalVolume(ndgar_solid_vol, argon, "NDGAR");

    G4RotationMatrix* ndgar_rot = new G4RotationMatrix();
    ndgar_rot->rotateY(90.*deg);

    new G4PVPlacement(ndgar_rot, G4ThreeVector(0.,0.,0.), 
                      ndgar_logic_vol, "NDGAR", hall_logic_vol, 
                      false, 0, true);


    magnetic_field_ = new G4UniformMagField(G4ThreeVector(0.5*tesla, 0.0, 0.0));
    field_mgr_ = new G4FieldManager();
    field_mgr_->SetDetectorField(magnetic_field_);
    field_mgr_->CreateChordFinder(magnetic_field_);
    G4bool forceToAllDaughters = true;
    ndgar_logic_vol->SetFieldManager(field_mgr_, forceToAllDaughters);

    // ACTIVE VOLUME ////////////////////////////////////////////////

    const G4double active_diam   = 5.0 * m;
    const G4double active_length = 5.0 * m;

    G4Tubs* active_solid_vol =
      new G4Tubs("ACTIVE", 0., active_diam/2., active_length/2., 0., 360.*deg);

    G4LogicalVolume* active_logic_vol =
      new G4LogicalVolume(active_solid_vol, argon, "ACTIVE");

    new G4PVPlacement(nullptr, G4ThreeVector(0.,0.,0.), 
                      active_logic_vol, "ACTIVE", ndgar_logic_vol, 
                      false, 0, true);

    // Define this volume as an ionization sensitive detector
    IonizationSD* sensdet = new IonizationSD("/NDGAR/ACTIVE");
    active_logic_vol->SetSensitiveDetector(sensdet);
    G4SDManager::GetSDMpointer()->AddNewDetector(sensdet);

    // Define an electric drift field for this volume
    UniformElectricDriftField* drift_field = 
      new UniformElectricDriftField(-active_length/2., active_length/2.+1.*mm, kXAxis);
    //drift_field->SetCathodePosition(active_length/2.);
    //drift_field->SetAnodePosition(-active_length/2.);
    drift_field->SetDriftVelocity(1.*cm/microsecond);
    //drift_field->SetTransverseDiffusion(.1*mm/sqrt(cm));
    //drift_field->SetLongitudinalDiffusion(.1*mm/sqrt(cm));

    G4Region* drift_region = new G4Region("DRIFT_REGION");
    drift_region->SetUserInformation(drift_field);
    drift_region->AddRootLogicalVolume(active_logic_vol);

    // REFLECTOR ////////////////////////////////////////////////////

    const G4double reflector_thickn = 1. * cm;

    G4Tubs* reflector_solid_vol =
      new G4Tubs("REFLECTOR", active_diam/2.-reflector_thickn, active_diam/2., 
                 active_length/2., 0., 360.*deg);

    auto teflon = G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON");

    G4LogicalVolume* reflector_logic_vol =
      new G4LogicalVolume(reflector_solid_vol, teflon, "REFLECTOR");

    // G4VisAttributes white = White();
    // white.SetForceSolid(true);
    // reflector_logic_vol->SetVisAttributes(white);

    new G4PVPlacement(nullptr, G4ThreeVector(0.,0.,0.), 
                      reflector_logic_vol, "REFLECTOR", active_logic_vol, 
                      false, 0, true);

    // Adding the optical surface
    G4OpticalSurface* light_tube_optSurf =
      new G4OpticalSurface("REFLECTOR", unified, ground, dielectric_metal);
    light_tube_optSurf->SetMaterialPropertiesTable(opticalprops::PTFE());

    new G4LogicalSkinSurface("REFLECTOR", reflector_logic_vol, light_tube_optSurf);

   // EL GAP ////////////////////////////////////////////////////////

    const G4double elgap_diam   = active_diam;
    const G4double elgap_length = 1. * cm;

    G4Tubs* elgap_solid_vol =
      new G4Tubs("EL_GAP", 0., elgap_diam/2., elgap_length/2., 0, twopi);

    G4LogicalVolume* elgap_logic_vol =
      new G4LogicalVolume(elgap_solid_vol, argon, "EL_GAP");

    G4double pos_z = active_length/2. + elgap_length/2.;

    new G4PVPlacement(0, G4ThreeVector(0.,0.,pos_z), elgap_logic_vol, "EL_GAP",
		      ndgar_logic_vol, false, 0, true);

    //Define an EL field for this volume
    UniformElectricDriftField* el_field = 
      new UniformElectricDriftField(-active_length/2.-elgap_length, -active_length/2., kXAxis);
    el_field->SetDriftVelocity(5.*mm/microsecond);
    //el_field->SetTransverseDiffusion(1.*mm/sqrt(cm));
    //el_field->SetLongitudinalDiffusion(.5*mm/sqrt(cm));
    el_field->SetLightYield(100./cm);

    G4Region* el_region = new G4Region("EL_REGION");
    el_region->SetUserInformation(el_field);
    el_region->AddRootLogicalVolume(elgap_logic_vol);

    // PHOTOSENSORS /////////////////////////////////////////////////

    GenericPhotosensor photosensor("PHOTOSENSORS", 25.*mm);
    photosensor.SetWithWLSCoating(false);
    photosensor.SetWindowRefractiveIndex(argon_mpt->GetProperty("RINDEX"));

    // Optical Properties of the sensor
    G4MaterialPropertiesTable* photosensor_mpt = new G4MaterialPropertiesTable();
    G4double energy[]       = {0.2 * eV, 3.5 * eV, 3.6 * eV, 11.5 * eV};
    G4double reflectivity[] = {0.0     , 0.0     , 0.0     ,  0.0     };
    G4double efficiency[]   = {1.0     , 1.0     , 1.0     ,  1.0     };
    photosensor_mpt->AddProperty("REFLECTIVITY", energy, reflectivity, 4);
    photosensor_mpt->AddProperty("EFFICIENCY",   energy, efficiency,   4);
    photosensor.SetOpticalProperties(photosensor_mpt);

    photosensor.SetSensorDepth(1);
    photosensor.SetMotherDepth(0);
    photosensor.SetNamingOrder(0);

    // Set visibility
    photosensor.SetVisibility(true);

    // Construct
    photosensor.Construct();
    G4LogicalVolume* photosensor_logic_vol = photosensor.GetLogicalVolume();

    G4RotationMatrix* photosensor_rot = nullptr; // new G4RotationMatrix();
    //    photosensor_rot->rotateY(180.*deg);

    const G4int num_photosensors_row = 209;
    G4int num_photosensor = 0;


    for (G4int i=0; i<num_photosensors_row; i++) {

      G4double xpos = -2.5*m + i * 25.*mm;

      for (G4int j=0; j<num_photosensors_row; j++) {
        
      G4double ypos = -2.5*m + j * 25.*mm;
    
      new G4PVPlacement(photosensor_rot, G4ThreeVector(xpos, ypos, -2.5*m-1.*cm), 
                        photosensor_logic_vol, "PHOTOSENSOR", ndgar_logic_vol, 
                        false, num_photosensor, false);

      num_photosensor++;
      }
    }



  /*
    // GAS //////////////////////////////////////////////////////////

    G4Tubs* gas_solid_vol =
      new G4Tubs("GAS", 0., (active_diam/2. + buffer_size),
                 (active_length/2. + buffer_size), 0., 360.*deg);

    G4LogicalVolume* gas_logic_vol =
      new G4LogicalVolume(gas_solid_vol, materials::GAr(10.*bar), "GAS");
    
    GeometryBase::SetLogicalVolume(gas_logic_vol);

    // ACTIVE ///////////////////////////////////////////////////////

    G4Tubs* active_solid_vol =
      new G4Tubs("ACTIVE", 0., active_diam/2., active_length/2., 0, 360.*deg);

    G4LogicalVolume* active_logic_vol =
      new G4LogicalVolume(active_solid_vol, materials::GAr(10.*bar), "ACTIVE");

    new G4PVPlacement(nullptr, G4ThreeVector(0.,0.,0.), 
                      active_logic_vol, "ACTIVE", gas_logic_vol, 
                      false, 0, true);


    // ACTIVE ////////////////////////////////////////////////////////

    const G4double active_diam   = chamber_diam;
    const G4double active_length = chamber_length/2.;

    G4Tubs* active_solid =
      new G4Tubs("ACTIVE", 0., active_diam/2., active_length/2., 0, twopi);

    G4LogicalVolume* active_logic =
      new G4LogicalVolume(active_solid, gxe, "ACTIVE");

    new G4PVPlacement(0, G4ThreeVector(0.,0.,0.), active_logic, "ACTIVE",
		      gas_logic, false, 0, true);

    // Define this volume as an ionization sensitive detector
    IonizationSD* sensdet = new IonizationSD("/CYLINDRIC_CHAMBER/ACTIVE");
    active_logic->SetSensitiveDetector(sensdet);
    G4SDManager::GetSDMpointer()->AddNewDetector(sensdet);

    // Define an electric drift field for this volume
    UniformElectricDriftField* drift_field = new UniformElectricDriftField();
    drift_field->SetCathodePosition(-active_length/2.);
    drift_field->SetAnodePosition(active_length/2.);
    drift_field->SetDriftVelocity(1.*mm/microsecond);
    drift_field->SetTransverseDiffusion(1.*mm/sqrt(cm));
    drift_field->SetLongitudinalDiffusion(.5*mm/sqrt(cm));

    G4Region* drift_region = new G4Region("DRIFT_REGION");
    drift_region->SetUserInformation(drift_field);
    drift_region->AddRootLogicalVolume(active_logic);


    // EL GAP ////////////////////////////////////////////////////////

    const G4double elgap_diam   = active_diam;
    const G4double elgap_length = 1. * cm;

    G4Tubs* elgap_solid =
      new G4Tubs("EL_GAP", 0., elgap_diam/2., elgap_length/2., 0, twopi);

    G4LogicalVolume* elgap_logic =
      new G4LogicalVolume(elgap_solid, gxe, "EL_GAP");

    G4double pos_z = active_length/2. + elgap_length/2.;

    new G4PVPlacement(0, G4ThreeVector(0.,0.,pos_z), elgap_logic, "EL_GAP",
		      gas_logic, false, 0, true);

    // Define an EL field for this volume
    UniformElectricDriftField* el_field = new UniformElectricDriftField();
    el_field->SetCathodePosition(active_length/2.);
    el_field->SetAnodePosition(active_length/2. + elgap_length);
    el_field->SetDriftVelocity(5.*mm/microsecond);
    el_field->SetTransverseDiffusion(1.*mm/sqrt(cm));
    el_field->SetLongitudinalDiffusion(.5*mm/sqrt(cm));
    el_field->SetLightYield(1000./cm);

    G4Region* el_region = new G4Region("EL_REGION");
    el_region->SetUserInformation(el_field);
    el_region->AddRootLogicalVolume(elgap_logic);


    // PHOTOMULTIPLIER ///////////////////////////////////////////////

    PmtR11410 pmt_geom;
    pmt_geom.SetSensorDepth(0);
    pmt_geom.Construct();
    G4LogicalVolume* pmt_logic = pmt_geom.GetLogicalVolume();

    pos_z = -30. * cm;

    new G4PVPlacement(0, G4ThreeVector(0., 0., pos_z), pmt_logic,
      "PMT", gas_logic, false, 0, true);


    // DICE BOARD ////////////////////////////////////////////////////

    NextNewKDB kdb_geom(5,5);
    kdb_geom.Construct();

    G4LogicalVolume* kdb_logic = kdb_geom.GetLogicalVolume();

    pos_z = active_length/2. + elgap_length + 5.0*mm;

    new G4PVPlacement(0, G4ThreeVector(0., 0., pos_z), kdb_logic,
      "KDB", gas_logic, false, 0, true);
  */
  }



  G4ThreeVector NDGAr::GenerateVertex(const G4String& /*region*/) const
  {
    return G4ThreeVector(0.,0.,0.);
  }


} // end namespace nexus
