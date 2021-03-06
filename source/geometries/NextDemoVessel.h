// -----------------------------------------------------------------------------
// nexus | NextDemoVessel.h
//
// Geometry of the pressure vessel of the NEXT-DEMO++ detector.
//
// The NEXT Collaboration
// -----------------------------------------------------------------------------

#ifndef NEXT_DEMO_VESSEL_H
#define NEXT_DEMO_VESSEL_H

#include "GeometryBase.h"

class G4GenericMessenger;
class G4VPhysicalVolume;

namespace nexus {

  class NextDemoVessel: public GeometryBase
  {
  public:
    NextDemoVessel();
    ~NextDemoVessel();
    void Construct() override;
    G4ThreeVector GenerateVertex(const G4String& region) const override;
    G4VPhysicalVolume* GetGasPhysicalVolume();
    G4double GetGateEndcapDistance() const;

  private:
    G4bool vessel_vis_;

    const G4double vessel_diam_, vessel_length_, vessel_thickn_;
    const G4double gate_endcap_distance_;

    G4double gas_pressure_, gas_temperature_;
    G4double sc_yield_, e_lifetime_;

    G4double calsrc_zpos_, calsrc_angle_;

    G4GenericMessenger* msg_;

    G4VPhysicalVolume* gas_phys_vol_;
  };

  inline G4VPhysicalVolume* NextDemoVessel::GetGasPhysicalVolume()
  { return gas_phys_vol_; }

  inline G4double NextDemoVessel::GetGateEndcapDistance() const
  { return gate_endcap_distance_; }

} // end namespace nexus

#endif
