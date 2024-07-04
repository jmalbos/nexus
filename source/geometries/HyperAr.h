#ifndef HYPER_AR_H
#define HYPER_AR_H

#include "GeometryBase.h"
#include <G4UniformMagField.hh>
#include <G4FieldManager.hh>

class G4GenericMessenger;


namespace nexus {

  class HyperAr: public GeometryBase
  {
  public:
    /// Constructor
    HyperAr();
    /// Destructor
    ~HyperAr() = default;

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
