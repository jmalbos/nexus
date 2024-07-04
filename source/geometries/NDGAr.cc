#include "NDGAr.h"

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

    const G4double hall_size = 100. * m;

    G4Box* hall_solid_vol = 
      new G4Box("HALL", hall_size/2., hall_size/2., hall_size/2.);

    G4LogicalVolume* hall_logic_vol =
      new G4LogicalVolume(hall_solid_vol, materials::Air(), "HALL");

    hall_logic_vol->SetVisAttributes(G4VisAttributes::GetInvisible());

    GeometryBase::SetLogicalVolume(hall_logic_vol);

    // NDGAR ////////////////////////////////////////////////////////
    // NDGAR VESSEL
    const G4double ndgar_vessel_length = 6.0*m;

    const G4double ndgar_vessel_diam = ndgar_vessel_length+200.*cm;
    
    const G4double ndgar_vessel_thickness = 4.0*cm;

    G4Tubs* ndgar_vessel_solid_vol = 
      new G4Tubs("NDGAR_VESSEL", ndgar_vessel_diam/2.-ndgar_vessel_thickness, ndgar_vessel_diam/2., ndgar_vessel_length/2., 0., 360.*deg);
    
    G4Material* aw5083 = materials::aw5083();
    G4Material* StainlessSteel = materials::StainlessSteel();
    G4LogicalVolume* ndgar_vessel_logic_vol = 
      new G4LogicalVolume(ndgar_vessel_solid_vol, /*aw5083*/StainlessSteel, "NDGAR_VESSEL");
    
    G4RotationMatrix* ndgar_vessel_rot = new G4RotationMatrix();
    ndgar_vessel_rot->rotateY(90.*deg);
    new G4PVPlacement(ndgar_vessel_rot, G4ThreeVector(0.,0.,0.),
                      ndgar_vessel_logic_vol, "NDGAR_VESSEL", hall_logic_vol,
                      false, 0, true);

    // ALUMINIUM PRESSURE VESSEL
    const G4double p_vessel_diam = ndgar_vessel_length;//6.0*m;
    const G4double p_vessel_length = ndgar_vessel_length;//6.0*m;
    const G4double p_vessel_thickness = 10*mm;//4.0*mm;

    G4Tubs* p_vessel_solid_vol = 
      new G4Tubs("PRESSURE_VESSEL", 0., p_vessel_diam/2., p_vessel_length/2., 0., 360.*deg);
    
    G4LogicalVolume* p_vessel_logic_vol = 
      new G4LogicalVolume(p_vessel_solid_vol, /*aw5083*/StainlessSteel, "PRESSURE_VESSEL");
    
    G4RotationMatrix* p_vessel_rot = new G4RotationMatrix();
    p_vessel_rot->rotateY(90.*deg);
    new G4PVPlacement(p_vessel_rot, G4ThreeVector(0.,0.,0.),
                      p_vessel_logic_vol, "PRESSURE_VESSEL", hall_logic_vol,
                      false, 0, true);
    
    
    /*
    //test box to check whether materials work as expected
    G4Box* testbox_solid = new G4Box("TESTBOX", 50*cm,50*cm,0.5*cm/2.);
    //G4Box* testbox_solid = new G4Box("TESTBOX", 50*cm,50*cm,0.2*cm/2.);
    G4LogicalVolume* testbox_lv = new G4LogicalVolume(testbox_solid, materials::Scintillator(), "TESTBOX_LV");
    //G4LogicalVolume* testbox_lv = new G4LogicalVolume(testbox_solid, materials::Copper(), "TESTBOX_LV");
    
    new G4PVPlacement (nullptr, G4ThreeVector(0., 0., 8.*m),testbox_lv,"TESTBOX", hall_logic_vol,false,0,true);

    // Define this volume as an ionization sensitive detector
    IonizationSD* sensdet_test = new IonizationSD("/NDGAR/TESTACTIVE");
    testbox_lv->SetSensitiveDetector(sensdet_test);
    G4SDManager::GetSDMpointer()->AddNewDetector(sensdet_test);
    */

    // OCTAGONAL ECAL
    const G4int ecal_nlayers = 60; //60
    const G4double ecal_converter_thickness = 2.*mm; //2*mm
    const G4double ecal_scint_thickness = 5.*mm; //5*mm
    const G4double ecal_layer_thickness = ecal_converter_thickness+ecal_scint_thickness;
    const G4double ecal_thickness = ecal_nlayers*ecal_layer_thickness+10.*cm;
    const G4int ecal_nsides = 8; // octagonal ecal
    const G4int ecal_z_nplanes = 2; //only two planes
    const G4double p_vessel_offset = 1.*mm;
    const G4double ecal_rmin = p_vessel_length/2. + p_vessel_offset;
    const G4double ecal_rmax = ecal_rmin + ecal_thickness;
    
    const G4double ecal_z_planes[ecal_z_nplanes] = {-p_vessel_length/2., p_vessel_length/2.};
    const G4double ecal_r_inners[ecal_z_nplanes] = {ecal_rmin, ecal_rmin};
    const G4double ecal_r_outers[ecal_z_nplanes] = {ecal_rmax, ecal_rmax};
    
    // one layer
    const G4double ecal_r_inners_layer[ecal_z_nplanes]           = {ecal_rmin,                      ecal_rmin};
    const G4double ecal_r_outers_layer[ecal_z_nplanes]           = {ecal_rmin+ecal_layer_thickness, ecal_rmin+ecal_layer_thickness};

    // converter component of layer
    const G4double ecal_r_inners_layer_converter[ecal_z_nplanes] = {ecal_rmin,                          ecal_rmin};
    const G4double ecal_r_outers_layer_converter[ecal_z_nplanes] = {ecal_rmin+ecal_converter_thickness, ecal_rmin+ecal_converter_thickness};

    // scintillator component of layer
    const G4double ecal_r_inners_layer_sensdet[ecal_z_nplanes]   = {ecal_rmin+ecal_converter_thickness,                      ecal_rmin+ecal_converter_thickness};
    const G4double ecal_r_outers_layer_sensdet[ecal_z_nplanes]   = {ecal_rmin+ecal_converter_thickness+ecal_scint_thickness, ecal_rmin+ecal_converter_thickness+ecal_scint_thickness};

    // ecal materials
    G4Material* copper = materials::Copper(); // converter layer
    G4Material* scinti = materials::Scintillator(); // sensitive layer

    //define sensitive detectors
    IonizationSD* sensdet_scint[ecal_nlayers];
    IonizationSD* mc_converter_sd[ecal_nlayers];

    // full ecal volume
    G4Polyhedra* ecal_solid_vol = 
      new G4Polyhedra(
        "OCTAGONAL_ECAL", 
        0., 360.*deg, 
        ecal_nsides, 
        ecal_z_nplanes, 
        ecal_z_planes, 
        ecal_r_inners, 
        ecal_r_outers);

    G4LogicalVolume* ecal_logic_vol = 
      new G4LogicalVolume(ecal_solid_vol, materials::Air(), "OCTAGONAL_ECAL");

    // build full ecal
    G4RotationMatrix* ecal_rot = new G4RotationMatrix();
    ecal_rot->rotateY(90. * deg);
    new G4PVPlacement(ecal_rot, G4ThreeVector(0., 0., 0.),
        ecal_logic_vol, "OCTAGONAL_ECAL", hall_logic_vol,
        false, 0, true);

    // ecal layer
    std::vector<G4Polyhedra*>     ecal_layers_solid_vol;
    std::vector<G4LogicalVolume*> ecal_layers_logic_vol;
    for (G4int i = 0; i < ecal_nlayers; i++) {
        const G4double temp_ecal_r_inners_layer[ecal_z_nplanes] = { ecal_rmin +  i      * ecal_layer_thickness, ecal_rmin +  i      * ecal_layer_thickness };
        const G4double temp_ecal_r_outers_layer[ecal_z_nplanes] = { ecal_rmin + (i + 1) * ecal_layer_thickness, ecal_rmin + (i + 1) * ecal_layer_thickness };
        std::string temp_name = "OCTAGONAL_ECAL_LAYER_" + std::to_string(i);
        ecal_layers_solid_vol.push_back(new G4Polyhedra(
            temp_name,
            0., 360. * deg,
            ecal_nsides,
            ecal_z_nplanes,
            ecal_z_planes,
            temp_ecal_r_inners_layer,
            temp_ecal_r_outers_layer));
        ecal_layers_logic_vol.push_back(new G4LogicalVolume(ecal_layers_solid_vol.back(), materials::Air(), temp_name));
        new G4PVPlacement(nullptr, G4ThreeVector(0,0,0),
                          ecal_layers_logic_vol.back(), temp_name, ecal_logic_vol, false, i, false);
    }
    
    // ecal converter layer
    std::vector<G4Polyhedra*>     ecal_layers_converter_solid_vol;
    std::vector<G4LogicalVolume*> ecal_layers_converter_logic_vol;
    for (G4int i = 0; i < ecal_nlayers; i++) {
        const G4double temp_ecal_converter_r_inners_layer[ecal_z_nplanes] = { ecal_rmin + i*ecal_layer_thickness,                            ecal_rmin + i*ecal_layer_thickness };
        const G4double temp_ecal_converter_r_outers_layer[ecal_z_nplanes] = { ecal_rmin + i*ecal_layer_thickness + ecal_converter_thickness, ecal_rmin + i*ecal_layer_thickness + ecal_converter_thickness };
        std::string temp_name = "OCTAGONAL_ECAL_LAYER_CONVERTER_" + std::to_string(i);
        ecal_layers_converter_solid_vol.push_back(new G4Polyhedra(
            temp_name,
            0., 360. * deg,
            ecal_nsides,
            ecal_z_nplanes,
            ecal_z_planes,
            temp_ecal_converter_r_inners_layer,
            temp_ecal_converter_r_outers_layer));
        ecal_layers_converter_logic_vol.push_back(new G4LogicalVolume(ecal_layers_converter_solid_vol.back(), copper, temp_name));
        mc_converter_sd[i] = new IonizationSD("/NDGAR/ECAL_MC_CONVERTER"+std::to_string(i));
        ecal_layers_converter_logic_vol[i]->SetSensitiveDetector(mc_converter_sd[i]);
        G4SDManager::GetSDMpointer()->AddNewDetector(mc_converter_sd[i]);
        new G4PVPlacement(nullptr, G4ThreeVector(0,0,0),
                          ecal_layers_converter_logic_vol.back(), temp_name, ecal_layers_logic_vol[i], false, i, false);
    }
    
    // ecal sensitive layer
    std::vector<G4Polyhedra*>     ecal_layers_sensdet_solid_vol;
    std::vector<G4LogicalVolume*> ecal_layers_sensdet_logic_vol;
    for (G4int i = 0; i < ecal_nlayers; i++) {
        const G4double temp_ecal_sensdet_r_inners_layer[ecal_z_nplanes] = { ecal_rmin + i*ecal_layer_thickness + ecal_converter_thickness,                        ecal_rmin + i*ecal_layer_thickness + ecal_converter_thickness };
        const G4double temp_ecal_sensdet_r_outers_layer[ecal_z_nplanes] = { ecal_rmin + i*ecal_layer_thickness + ecal_converter_thickness + ecal_scint_thickness, ecal_rmin + i*ecal_layer_thickness + ecal_converter_thickness + ecal_scint_thickness };
        std::string temp_name = "OCTAGONAL_ECAL_LAYER_SENSDET_" + std::to_string(i);
        ecal_layers_sensdet_solid_vol.push_back(new G4Polyhedra(
            temp_name,
            0., 360. * deg,
            ecal_nsides,
            ecal_z_nplanes,
            ecal_z_planes,
            temp_ecal_sensdet_r_inners_layer,
            temp_ecal_sensdet_r_outers_layer));
        ecal_layers_sensdet_logic_vol.push_back(new G4LogicalVolume(ecal_layers_sensdet_solid_vol.back(), scinti, temp_name));
        sensdet_scint[i] = new IonizationSD("/NDGAR/ECAL_ACTIVE"+std::to_string(i));
        ecal_layers_sensdet_logic_vol[i]->SetSensitiveDetector(sensdet_scint[i]);
        G4SDManager::GetSDMpointer()->AddNewDetector(sensdet_scint[i]);
        new G4PVPlacement(nullptr, G4ThreeVector(0,0,0),
                          ecal_layers_sensdet_logic_vol.back(), temp_name, ecal_layers_logic_vol[i], false, i, false);
    }

    // caps for hermeticity at anode and cathode, only for hits checks
    G4Tubs* sv_sens_disc1 = new G4Tubs("SV_MC_CAP1", 0., p_vessel_diam/2., 3.0*cm, 0., 360.*deg);
    G4LogicalVolume* lv_sens_disc1 = new G4LogicalVolume(sv_sens_disc1, scinti, "LV_MC_CAP1");
    G4Tubs* sv_sens_disc2 = new G4Tubs("SV_MC_CAP2", 0., p_vessel_diam/2., 3.0*cm, 0., 360.*deg);
    G4LogicalVolume* lv_sens_disc2 = new G4LogicalVolume(sv_sens_disc2, scinti, "LV_MC_CAP2");
    IonizationSD * sd_disc1 = new IonizationSD("/NDGAR/CAP1_ACTIVE");
    IonizationSD * sd_disc2 = new IonizationSD("/NDGAR/CAP2_ACTIVE");
    lv_sens_disc1->SetSensitiveDetector(sd_disc1);
    G4SDManager::GetSDMpointer()->AddNewDetector(sd_disc1);
    lv_sens_disc2->SetSensitiveDetector(sd_disc2);
    G4SDManager::GetSDMpointer()->AddNewDetector(sd_disc2);
    G4RotationMatrix* cap_rot = new G4RotationMatrix();
    cap_rot->rotateY(90.*deg);

    // GASEOUS ARGON FILLING
    const G4double ndgar_diam   = p_vessel_diam-2*p_vessel_thickness;
    const G4double ndgar_length = p_vessel_length-2*p_vessel_thickness;

    G4Tubs* ndgar_solid_vol =
      new G4Tubs("NDGAR", 0., ndgar_diam/2., ndgar_length/2., 0., 360.*deg);

    G4Material* argon = materials::GAr(10.*bar);
    auto argon_mpt = opticalprops::GAr(714./MeV); //714ph/MeV if all corrections were considered
    argon->SetMaterialPropertiesTable(argon_mpt);

    G4LogicalVolume* ndgar_logic_vol =
      new G4LogicalVolume(ndgar_solid_vol, argon, "NDGAR");

    //G4RotationMatrix* ndgar_rot = new G4RotationMatrix();
    //ndgar_rot->rotateY(90.*deg);

    new G4PVPlacement(nullptr, G4ThreeVector(0.,0.,0.), 
                      ndgar_logic_vol, "NDGAR", p_vessel_logic_vol, 
                      false, 0, true);


    magnetic_field_ = new G4UniformMagField(G4ThreeVector(0.5*tesla, 0.0, 0.0)); //0.5 tesla in x direction
    field_mgr_ = new G4FieldManager();
    field_mgr_->SetDetectorField(magnetic_field_);
    field_mgr_->CreateChordFinder(magnetic_field_);
    G4bool forceToAllDaughters = true;
    ndgar_logic_vol->SetFieldManager(field_mgr_, forceToAllDaughters);

    // ACTIVE VOLUME ////////////////////////////////////////////////

    const G4double active_diam   = 5.0 * m; 
    const G4double active_length = active_diam; //5.0 * m;

    G4Tubs* active_solid_vol =
      new G4Tubs("ACTIVE", 0., active_diam/2., active_length/2., 0., 360.*deg);

    G4LogicalVolume* active_logic_vol =
      new G4LogicalVolume(active_solid_vol, argon, "ACTIVE");
    G4UserLimits* ulims = new G4UserLimits(1.0*m);
    active_logic_vol->SetUserLimits(ulims);

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
    drift_field->SetDriftVelocity(3.*cm/microsecond);
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
      //new G4OpticalSurface("REFLECTOR", glisur, ground, dielectric_metal);
    //light_tube_optSurf->SetPolish(0.2); // if model is glisur
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
    el_field->SetLightYield(1./cm);

    G4Region* el_region = new G4Region("EL_REGION");
    el_region->SetUserInformation(el_field);
    el_region->AddRootLogicalVolume(elgap_logic_vol);

    // PMMA WINDOW ///////////////////////////////////////////////
    G4double r_pmma_window = active_diam;
    G4double d_pmma_window = 2.*cm;

    G4Tubs* pmma_window_solid_vol =
      new G4Tubs("PMMA_WINDOW_SOLID", 0, r_pmma_window/2., 
                 d_pmma_window/2., 0., 360.*deg);

    G4Material* pmma_window_mat = materials::PMMA();
    pmma_window_mat->SetMaterialPropertiesTable(opticalprops::PMMA());
    G4LogicalVolume* pmma_window_logic_vol =
      new G4LogicalVolume(pmma_window_solid_vol, pmma_window_mat, "PMMA_WINDOW_LOGIC");
    
    
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., -active_length / 2. - 1. * mm - d_pmma_window / 2),
                      pmma_window_logic_vol, "PMMA_WINDOW", ndgar_logic_vol, 
                      false, 0, true);
    new G4PVPlacement(nullptr, G4ThreeVector(0.,0., -active_length / 2. - 1.*mm - d_pmma_window/2 - 10.*cm), 
                      pmma_window_logic_vol, "PMMA_WINDOW", ndgar_logic_vol, 
                      false, 1, true);
    
    
    
    // PHOTOSENSORS /////////////////////////////////////////////////
    const G4int n_cells = 4;

    const G4double sens_cell_x = 6 * mm; //6*mm
    const G4double sens_cell_y = 6 * mm; //6*mm
    const G4double sens_cell_z = 2. * mm;

    const G4double sens_size_x = n_cells * sens_cell_x; //1.3 mm; //5000.*mm;
    const G4double sens_size_y = n_cells * sens_cell_y; //1.3 mm
    //const G4double sens_size_x = 5000.*mm; //1.3 mm; //5000.*mm;
    //const G4double sens_size_y = 5000.*mm; //1.3 mm
    const G4double sens_size_z = sens_cell_z; //2.0 mm

    GenericPhotosensor photosensor("PHOTOSENSORS", sens_size_x, sens_size_y, sens_size_z);
    photosensor.SetWithWLSCoating(false);
    photosensor.SetWindowRefractiveIndex(argon_mpt->GetProperty("RINDEX"));

    // Optical Properties of the sensor
    G4MaterialPropertiesTable* photosensor_mpt = new G4MaterialPropertiesTable();
    G4double energy[]       = {0.2 * eV, 3.5 * eV, 3.6 * eV, 11.5 * eV};
    G4double reflectivity[] = {0.0     , 0.0     , 0.0     ,  0.0     };
    //G4double efficiency[]   = {1.0     , 1.0     , 1.0     ,  1.0     };

    std::vector<G4double> energyEff;
    std::vector<G4double> efficiency;

    // read efficiencies from CSV ///////////////////////////////////////
    std::ifstream effs;
    std::string line;
    std::string delimiter = ",";
    std::string s1;
    std::string s2;
    G4double energyAtVal;
    G4double effAtVal;
    effs.open("data/HAM14160SensDetEff.csv");
    while(getline(effs,line)) {
      s1 = line.substr(0,line.find(delimiter));
      line.erase(0,line.find(delimiter)+delimiter.length());
      s2 = line;
      energyAtVal = (G4double)std::atof(s1.c_str())*nm;
      effAtVal = (G4double)std::atof(s2.c_str());
      if (effAtVal<0) effAtVal=0;
      energyEff.push_back(h_Planck*c_light /energyAtVal);
      efficiency.push_back(effAtVal / 100.);
      //G4cout << "wl: " << energyAtVal/nm << " nm, eff: " << efficiency.back() << G4endl;
    }
    effs.close();
    std::reverse(energyEff.begin(), energyEff.end());
    std::reverse(efficiency.begin(), efficiency.end());
    energyEff.push_back(h_Planck*c_light/(270.*nm));
    energyEff.push_back(h_Planck*c_light/(200.*nm));
    efficiency.push_back(0.);
    efficiency.push_back(0.);


    photosensor_mpt->AddProperty("REFLECTIVITY", energy, reflectivity, 4);
    photosensor_mpt->AddProperty("EFFICIENCY",   energyEff, efficiency,  energyEff.size());
    photosensor.SetOpticalProperties(photosensor_mpt);
    
    
    // Set time binning
    photosensor.SetTimeBinning(.001 * us);

    photosensor.SetSensorDepth(1);
    photosensor.SetMotherDepth(0);
    photosensor.SetNamingOrder(0);

    // Set visibility
    photosensor.SetVisibility(true);

    // Construct
    photosensor.Construct();
    
    
    G4LogicalVolume* photosensor_logic_vol = photosensor.GetLogicalVolume();

    
    G4double theOffset = 3.403 * mm;
    G4double sens_spacing_x = sens_size_x + theOffset;
    G4double sens_spacing_y = sens_size_y + theOffset;

    G4RotationMatrix* photosensor_rot = nullptr; // new G4RotationMatrix();
    //    photosensor_rot->rotateY(180.*deg);

    
    G4int N_x = (G4int)(active_diam / (sens_spacing_x));
    G4int N_y = (G4int)(active_diam / (sens_spacing_y));
    G4int N = N_x * N_y;
    
    G4double sens_pos_pv_x[N_x - 1];
    G4double sens_pos_pv_y[N_y - 1];

    std::cout << "number of cells in x direction: " << N_x << std::endl;
    std::cout << "number of cells in y direction: " << N_y << std::endl;

    for (G4int i = 0; i < N_x - 1; i++) {
        sens_pos_pv_x[i] = (i - (N_x - 1) / 2.) * sens_spacing_x + sens_spacing_x / 2.;
    }
    for (G4int i = 0; i < N_y - 1; i++) {
        sens_pos_pv_y[i] = (i - (N_y - 1) / 2.) * sens_spacing_y + sens_spacing_y / 2.;
    }
    
    G4int sens_id = 0;

    G4double sens_full_size;
    G4ThreeVector sens_pos;
    // test sipm to avoid all kinds of effects. Just one single big sipm covering the whole area
    //sens_pos = G4ThreeVector(0. * mm, 0. * mm, -active_length / 2. - 1. * mm); // - d_pmma_window / 2 - 10. * cm - d_pmma_window / 2 - 1. * mm - sens_size_z / 2. - 1. * cm);
    //new G4PVPlacement(photosensor_rot, sens_pos, photosensor_logic_vol, photosensor.GetName(), ndgar_logic_vol, false, 0, true);
    
    
    for (G4int i = 0; i < N_x - 1; i++) {
        for (G4int j = 0; j < N_y - 1; j++) {
            sens_full_size = std::sqrt(std::pow(std::abs(sens_pos_pv_x[i]), 2) + std::pow(std::abs(sens_pos_pv_y[j]), 2));
            if (sens_full_size < active_diam / 2.) {
                sens_pos = G4ThreeVector(sens_pos_pv_x[i], sens_pos_pv_y[j], -active_length / 2. - 1. * mm -d_pmma_window / 2 - 10. * cm - d_pmma_window / 2 - 1. * mm - sens_size_z / 2. - 1. * cm);
                new G4PVPlacement(photosensor_rot, sens_pos, photosensor_logic_vol, photosensor.GetName(), ndgar_logic_vol, false, sens_id, false);
                ++sens_id;
            }
        }
    }
    

    G4double sipmTotSurface = sens_id*sens_size_x*sens_size_y;
    G4double TotalCathodeSurface = std::pow((active_diam/2.),2)*twopi/2.;
    G4double Ffill = sipmTotSurface/TotalCathodeSurface;

    std::cout << std::endl;
    std::cout << "Total number of SiPMs: " << sens_id << std::endl;
    std::cout << "Fill factor of SiPMs on Cathode plane: " << Ffill << std::endl;
    std::cout << std::endl;

  }



  G4ThreeVector NDGAr::GenerateVertex(const G4String& region) const
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
