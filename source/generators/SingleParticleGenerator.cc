// ----------------------------------------------------------------------------
// nexus | SingleParticleGenerator.cc
//
// This class is the primary generator for events consisting of
// a single particle. The user must specify via configuration
// parameters the particle type, a kinetic energy interval and, optionally,
// a momentum direction.
// Particle energy is generated with flat random probability
// between E_min and E_max.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "SingleParticleGenerator.h"

#include "DetectorConstruction.h"
#include "GeometryBase.h"
#include "RandomUtils.h"
#include "FactoryBase.h"

#include <G4GenericMessenger.hh>
#include <G4ParticleDefinition.hh>
#include <G4RunManager.hh>
#include <G4ParticleTable.hh>
#include <G4PrimaryVertex.hh>
#include <G4Event.hh>
#include <G4RandomDirection.hh>
#include <Randomize.hh>
#include <G4OpticalPhoton.hh>

#include "CLHEP/Units/SystemOfUnits.h"


using namespace nexus;
using namespace CLHEP;

REGISTER_CLASS(SingleParticleGenerator, G4VPrimaryGenerator)


SingleParticleGenerator::SingleParticleGenerator():
G4VPrimaryGenerator(), msg_(0), particle_definition_(0),
energy_min_(0.), energy_max_(0.), geom_(0), momentum_{}, position_{},
costheta_min_(-1.), costheta_max_(1.), phi_min_(0.), phi_max_(2.*pi)
{
  msg_ = new G4GenericMessenger(this, "/Generator/SingleParticle/",
    "Control commands of single-particle generator.");

  msg_->DeclareMethod("particle", &SingleParticleGenerator::SetParticleDefinition,
    "Set particle to be generated.");

  G4GenericMessenger::Command& min_energy =
    msg_->DeclareProperty("min_energy", energy_min_,
      "Set minimum kinetic energy of the particle.");
  min_energy.SetUnitCategory("Energy");
  min_energy.SetParameterName("min_energy", false);
  min_energy.SetRange("min_energy>0.");

  G4GenericMessenger::Command& max_energy =
    msg_->DeclareProperty("max_energy", energy_max_,
      "Set maximum kinetic energy of the particle");
  max_energy.SetUnitCategory("Energy");
  max_energy.SetParameterName("max_energy", false);
  max_energy.SetRange("max_energy>0.");

  msg_->DeclareProperty("region", region_,
    "Set the region of the geometry where the vertex will be generated.");


  msg_->DeclarePropertyWithUnit("momentum", "mm",  momentum_, "Set particle 3-momentum.");
  msg_->DeclarePropertyWithUnit("position", "mm",  position_, "Set particle vertex.");

  msg_->DeclareProperty("min_costheta", costheta_min_,
			"Set minimum cosTheta for the direction of the particle.");
  msg_->DeclareProperty("max_costheta", costheta_max_,
			"Set maximum cosTheta for the direction of the particle.");
  msg_->DeclareProperty("min_phi", phi_min_,
			"Set minimum phi for the direction of the particle.");
  msg_->DeclareProperty("max_phi", phi_max_,
			"Set maximum phi for the direction of the particle.");


  DetectorConstruction* detconst = (DetectorConstruction*) G4RunManager::GetRunManager()->GetUserDetectorConstruction();
  geom_ = detconst->GetGeometry();
}



SingleParticleGenerator::~SingleParticleGenerator()
{
  delete msg_;
}



void SingleParticleGenerator::SetParticleDefinition(G4String particle_name)
{
  particle_definition_ =
    G4ParticleTable::GetParticleTable()->FindParticle(particle_name);

  if (!particle_definition_)
    G4Exception("[SingleParticleGenerator]", "SetParticleDefinition()",
      FatalException, "User gave an unknown particle name.");
}



void SingleParticleGenerator::GeneratePrimaryVertex(G4Event* event)
{
  // Generate uniform random energy in [E_min, E_max]
  G4double kinetic_energy = nexus::UniformRandomInRange(energy_max_, energy_min_);

  // Calculate cartesian components of momentum
  G4double mass   = particle_definition_->GetPDGMass();
  G4double energy = kinetic_energy + mass;
  G4double pmod = std::sqrt(energy*energy - mass*mass);

  G4int nparts = 10;
  G4double times[nparts];
  bool fixed_position = position_ != G4ThreeVector{};
  bool fixed_momentum = momentum_ != G4ThreeVector{};
  bool restrict_angle = costheta_min_ != -1. || costheta_max_ != 1. || phi_min_ != 0. || phi_max_ !=2.*pi;
  G4double Phi;
  G4double Theta;

  //G4cout << costheta_max_ << " " << costheta_min_ << " " << phi_min_ << " " << phi_max_ << G4endl;
  //G4cout << momentum_.x() << " " << momentum_.y() << " " << momentum_.z() << G4endl;

  G4ThreeVector p_dir; // it will be set in the if branches below
  if (fixed_momentum) { // if the user provides a momentum direction
    p_dir = momentum_.unit();
  } else if (restrict_angle) { // if the user provides a range of angles
    p_dir = RandomDirectionInRange(costheta_min_, costheta_max_, phi_min_, phi_max_);
  } else {

    p_dir = G4RandomDirection(); // for isotropic emission
    //costheta_min_ = std::cos(160. * 2. * pi / 360.);
    //costheta_max_ = std::cos(180. * 2. * pi / 360.);
    //costheta_min_ = std::cos(0. * 2. * pi / 360.);
    //costheta_max_ = std::cos(20. * 2. * pi / 360.);
    
    // set phi randomly to either -pi/2 or +pi/2
    int signum = 0;
    G4double signTest = G4UniformRand();
    if(signTest<0.5) signum = 1;
    else signum = -1;
    //phi_min_ = signum*90.*2*pi/360.;
    //phi_max_ = signum*90.*2.*pi/360.;
    
    //p_dir = RandomDirectionInRange(costheta_min_, costheta_max_, phi_min_, phi_max_); // for directed emission
    //p_dir = G4ThreeVector(0.,0.,-1.);
    //p_dir = G4ThreeVector(0.,0.,1.);

    /*
    // ANGLE LIMITS
    //costheta_min_ = std::cos(9. * 2. * pi / 360.);
    //costheta_max_ = std::cos(10. * 2. * pi / 360.);
    G4double theta_min_ = 90. * 2. * pi / 360.; //conds for dir in pz<0 and px,py arbitrary
    G4double theta_max_ = 270. * 2. * pi / 360.;
    
    //phi_min_ = 90.*2*pi/360.;
    //phi_max_ = 180.*2*pi/360.;
    phi_min_ = pi/2.;
    phi_max_ = pi/2.;

    // RANDOM ANGLE FOR EVENT
    Phi = phi_min_+(phi_max_-phi_min_)*G4UniformRand();
    Theta = theta_min_+(theta_max_-theta_min_)*G4UniformRand(); 
    //Phi = 90.*2*pi/360.; //phi_min_+(phi_max_-phi_min_)*G4UniformRand();
    //Theta = theta_min_+(theta_max_-theta_min_)*G4UniformRand(); //180. * 2.*pi/360.;

    // MOMENTUM DIRECTION
    p_dir = G4ThreeVector(std::sin(Theta)*std::cos(Phi), std::sin(Theta)*std::sin(Phi), std::cos(Theta));
    //p_dir = RandomDirectionInRange(costheta_min_, costheta_max_, phi_min_, phi_max_); //G4ThreeVector(1., 0. , 0.); //G4RandomDirection(); // CHANGED+
    //p_dir = G4ThreeVector(-1.,0.,0.);
    //p_dir = G4ThreeVector(1.,0.,0.);
    */
    
  }

  G4ThreeVector p = pmod * p_dir;
  //momentum_ = G4ThreeVector(0,0,0);

  // Create the new primary particle and set it some properties
  auto particle = new G4PrimaryParticle(particle_definition_, p.x(), p.y(), p.z());

  // Set random polarization
  if (particle_definition_ == G4OpticalPhoton::Definition()) {
    G4ThreeVector polarization =  G4RandomDirection();
    //G4ThreeVector polarization = G4ThreeVector(1.,0.,0.);
    particle->SetPolarization(polarization);
  }

  // Generate an initial position for the particle using the geometry
  G4double x_pos_start = -2500.*mm + G4UniformRand()*5000.*mm;
  G4double y_pos_start = -2500.*mm + G4UniformRand()*5000.*mm;
  G4double r_rand = G4UniformRand() * 2489. * mm;
  G4double phi = G4UniformRand()*twopi;
  G4double y_rad = r_rand*sin(phi); //sin(Theta);
  G4double z_rad = r_rand*cos(phi); //cos(Theta);
  G4double x_r0axis = -2500. * mm + G4UniformRand()*5000.*mm;
  //G4ThreeVector position = G4ThreeVector(x_pos_start, y_pos_start, -3050.*mm); //plane outside NDGAr //geom_->GenerateVertex(region_); // CHANGED
  G4ThreeVector position = G4ThreeVector(0., 0., -4850.*mm);
  //G4ThreeVector position = G4ThreeVector(0., 0., 7.*m); //testbox check
  //G4ThreeVector position = G4ThreeVector(-2450*mm, 0., 0.);
  //G4ThreeVector position = G4ThreeVector(0.*mm, y_rad, z_rad); // disc around origin
  //G4ThreeVector position = G4ThreeVector(x_r0axis, 0., 0.); //along central axis of NDGAr
  //G4ThreeVector position = G4ThreeVector(x_r0axis, y_rad, z_rad); //randomly distributed in whole volume
  //G4ThreeVector position = G4ThreeVector(0., 0., 0.);

  // Particle generated at start-of-event
  G4double time = G4UniformRand()*10*us;
  //G4double time = 1500.*ns;
  G4double decayTime = 5*nanosecond;
  //CLHEP::HepRandomEngine * randEng = new CLHEP::DualRand();
  //for (int i = 0; i<nparts; i++) {
    //double decayDistr = CLHEP::RandExponential::shoot(decayTime);
    //times[i] = G4UniformRand() * 10 * microsecond;
    //std::cout << decayDistr << std::endl;
  //}

  //std::cout << fixed_position << G4endl;
  //std::cout << position_ << G4endl;

  if(fixed_position) {
    position=position_;
  }

  // Create a new vertex
  G4PrimaryVertex* vertex = new G4PrimaryVertex(position, time);

    // Add particle to the vertex and this to the event
  vertex->SetPrimary(particle);
  event->AddPrimaryVertex(vertex);
}




/*void SingleParticleGenerator::GeneratePrimaryVertex(G4Event* event)
{
  // Generate uniform random energy in [E_min, E_max]
  G4double kinetic_energy = nexus::UniformRandomInRange(energy_max_, energy_min_);

    // Calculate cartesian components of momentum
  G4double mass   = particle_definition_->GetPDGMass();
  G4double energy = kinetic_energy + mass;
  G4double pmod = std::sqrt(energy*energy - mass*mass);

  bool fixed_momentum = momentum_ != G4ThreeVector{};
  bool restrict_angle = costheta_min_ != -1. || costheta_max_ != 1. || phi_min_ != 0. || phi_max_ !=2.*pi;

  G4ThreeVector p_dir; // it will be set in the if branches below
  if (fixed_momentum) { // if the user provides a momentum direction
    p_dir = momentum_.unit();
  } else if (restrict_angle) { // if the user provides a range of angles
    p_dir = RandomDirectionInRange(costheta_min_, costheta_max_, phi_min_, phi_max_);
  } else {
    p_dir = G4RandomDirection();
  }

  G4ThreeVector p = pmod * p_dir;

  // Create the new primary particle and set it some properties
  auto particle = new G4PrimaryParticle(particle_definition_, p.x(), p.y(), p.z());

  // Set random polarization
  if (particle_definition_ == G4OpticalPhoton::Definition()) {
    G4ThreeVector polarization = G4RandomDirection();
    particle->SetPolarization(polarization);
  }

  // Generate an initial position for the particle using the geometry
  G4ThreeVector position = geom_->GenerateVertex(region_);

  // Particle generated at start-of-event
  G4double time = 0.;

  // Create a new vertex
  G4PrimaryVertex* vertex = new G4PrimaryVertex(position, time);

    // Add particle to the vertex and this to the event
  vertex->SetPrimary(particle);
  event->AddPrimaryVertex(vertex);
}*/