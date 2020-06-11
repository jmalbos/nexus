// ----------------------------------------------------------------------------
// nexus | GeneratorFactory.cc
//
// This class instantiates the generator of the simulation that the user
// specifies via configuration parameter.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "GeneratorFactory.h"

#include <G4GenericMessenger.hh>
#include <G4VPrimaryGenerator.hh>

using namespace nexus;


GeneratorFactory::GeneratorFactory(): msg_(0)
{
  msg_ = new G4GenericMessenger(this, "/Generator/");
  msg_->DeclareProperty("RegisterGenerator", name_, "");
}


GeneratorFactory::~GeneratorFactory()
{
  delete msg_;
  delete p_;
}


//////////////////////////////////////////////////////////////////////

#include "SingleParticle.h"
#include "IonGun.h"
#include "Na22Generation.h"
#include "Kr83mGeneration.h"
#include "ScintillationGenerator.h"
#include "DoubleParticle.h"
#include "Back2backGammas.h"
#include "SensMap.h"

G4VPrimaryGenerator* GeneratorFactory::CreateGenerator()
{
  //G4VPrimaryGenerator* p = 0;

  if (name_ == "SINGLE_PARTICLE") p_ = new SingleParticle();

  else if (name_ == "ION_GUN") p_ = new IonGun();

  else if (name_ == "NA22") p_ = new Na22Generation();

  else if (name_ == "Kr83m") p_ = new Kr83mGeneration();

  else if (name_ == "SCINT_GENERATOR") p_ = new ScintillationGenerator();

  else if (name_ == "DOUBLE_PARTICLE") p_ = new DoubleParticle();

  else if (name_ == "BACK2BACK") p_ = new Back2backGammas();

  else if (name_ == "SENSMAP") p_ = new SensMap();

  else {
    G4String err = "The user specified an unknown generator: " + name_;
    G4Exception("[GeneratorFactory]", "CreateGenerator()",
        FatalException, err);
  }

  return p_;
}
