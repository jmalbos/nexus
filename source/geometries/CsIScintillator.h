// ----------------------------------------------------------------------------
// nexus | CsIScintillator.h
//
// Sphere filled with mineral oil.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef CSI_SCINT_H
#define CSI_SCINT_H

#include "GeometryBase.h"

class G4Material;
class G4GenericMessenger;
namespace nexus { class CylinderPointSampler; }


namespace nexus {

  /// Spherical chamber filled with mineral oil

  class CsIScintillator: public GeometryBase
  {
  public:
    /// Constructor
    CsIScintillator();
    /// Destructor
    ~CsIScintillator();

    /// Return vertex within region <region> of the chamber
    G4ThreeVector GenerateVertex(const G4String& region) const;

    void Construct();

  private:
    G4double radius_;   ///< Radius of the cylinder
    G4double length_;   ///< Length of the cylinder

    /// Vertexes random generator
    CylinderPointSampler* cylinder_vertex_gen_;

    /// Messenger for the definition of control commands
    G4GenericMessenger* msg_;
  };

} // end namespace nexus

#endif
