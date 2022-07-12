#ifndef ND_GAR_H
#define ND_GAR_H

#include "GeometryBase.h"
#include <G4UniformMagField.hh>
#include <G4FieldManager.hh>

class G4GenericMessenger;


namespace nexus {

  class NDGAr: public GeometryBase
  {
  public:
    /// Constructor
    NDGAr();
    /// Destructor
    ~NDGAr() = default;

    /// Return vertex within region <region> of the chamber
    virtual G4ThreeVector GenerateVertex(const G4String& region) const;

    virtual void Construct();

  private:
    /// Messenger for the definition of control commands
    std::unique_ptr<G4GenericMessenger> msg_;

    static G4ThreadLocal G4UniformMagField* magnetic_field_;
    static G4ThreadLocal G4FieldManager* field_mgr_;
  };

} // end namespace nexus

#endif
