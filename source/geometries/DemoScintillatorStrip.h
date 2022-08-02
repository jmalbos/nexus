// ----------------------------------------------------------------------------
// nexus | DemoScintillatorStrip.h
//
// 
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef DEMO_SCINTILLATOR_STRIP_H
#define DEMO_SCINTILLATOR_STRIP_H

#include "GeometryBase.h"

class G4GenericMessenger;

namespace nexus {

  class DemoScintillatorStrip : public GeometryBase
  {
  public:
    DemoScintillatorStrip();
    ~DemoScintillatorStrip();
    void Construct();

  private:
    G4GenericMessenger* msg_;
  };
}

#endif
