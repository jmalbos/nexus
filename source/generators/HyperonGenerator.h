// ----------------------------------------------------------------------------
// nexus | HyperonGenerator.h
//
// This class is the primary generator of a Xe-136 double beta decay,
// interfacing the DECAY0 c++ code, translated from the original
// FORTRAN package, with nexus.
// The possibility of reading a previously generated ascii file with the
// electron momenta is also allowed.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef HYPERON_GENERATOR_H
#define HYPERON_GENERATOR_H

#include <G4VPrimaryGenerator.hh>
#include <G4VUserPrimaryVertexInformation.hh>
#include <fstream>

class G4GenericMessenger;
class G4Event;
class G4PrimaryParticle;
class decay0;

namespace nexus {

  class GeometryBase;

  class basePrimaryVertexUserInfo : public G4VUserPrimaryVertexInformation
  {
    public:
      basePrimaryVertexUserInfo();
      virtual ~basePrimaryVertexUserInfo();
    
    private:
      void Print() const;
  };

  inline basePrimaryVertexUserInfo::basePrimaryVertexUserInfo() {}
  inline basePrimaryVertexUserInfo::~basePrimaryVertexUserInfo() {}
  inline void basePrimaryVertexUserInfo::Print() const {}

  class PrimaryVertexUserInfo : public basePrimaryVertexUserInfo
  {
    public:
      PrimaryVertexUserInfo() {}
      PrimaryVertexUserInfo(G4double weight) {fWeight = weight;}
      ~PrimaryVertexUserInfo() {}

      void SetWeight(G4double weight) {fWeight = weight;}
      G4double GetWeight() const {return fWeight;}
    
    private:
      G4double fWeight;
  };


  /// This primary generator sets the G4Event objects according to the
  /// information read from an ascii file produced by the Decay0
  /// Monte-Carlo event generator.

  class HyperonGenerator : public G4VPrimaryGenerator
  {
  public:
    /// Constructor
    HyperonGenerator();
    /// Destructor
    ~HyperonGenerator();

    /// Read an event from file and generate primary particles
    /// and primary vertices accordingly
    void GeneratePrimaryVertex(G4Event*);

  private:
    /// Open the Decay0 input file selected by the user
    void OpenInputFile(G4String);
    /// Parse information in the file header
    void ProcessHeader();

    /// Return the PDG code equivalent to a given GEANT3 particle code
    G4int G3toPDG(const G4int);

  private:
    G4GenericMessenger* msg_;

    std::ifstream file_; ///< ASCII file produced by Decay0
    G4String region_; ///< region of generation of vertices in geometry

    G4bool opened_;

    G4ThreeVector position_;

    int myEventCounter_;
    G4int nevents_;

    decay0 *decay0_;
    int Xe136DecayMode_; // See method printDecayModeList  Default is 1
    int Ba136FinalState_; // labeled by energy level, in keV .
                          // Valid list: 0, 819,  1551, 1579, 2080, 2129, 2141, 2223, 2315, 2400)
			  // default is 0 (ground state)

    double energyThreshold_;

    std::ofstream fOutDebug_; // for debugging...
    const GeometryBase* geom_;


    inline void SetEnergyThreshold(double e) { energyThreshold_ = e;}
    inline void SetXe136DecayMode(int dcm) { Xe136DecayMode_ = dcm;}
    inline void SetBa136FinalState(int fs) { Ba136FinalState_ = fs;}
  };


} //end namespace nexus




#endif
