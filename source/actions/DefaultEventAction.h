// ----------------------------------------------------------------------------
///  \file   DefaultEventAction.h
///  \brief  A general-purpose user event action.
///
///  \author   <justo.martin-albo@ific.uv.es>
///  \date     15 Apr 2009
///  \version  $Id$ 
///
///  Copyright (c) 2009-2012 NEXT Collaboration. All rights reserved.
// ----------------------------------------------------------------------------

#ifndef __DEFAULT_EVENT_ACTION__
#define __DEFAULT_EVENT_ACTION__

#include <G4UserEventAction.hh>
#include <globals.hh>

class G4Event;


namespace nexus {
    
  /// General-purpose user event action
  
  class DefaultEventAction: public G4UserEventAction
  {
  public:
    /// Constructor
    DefaultEventAction();
    /// Destructor
    ~DefaultEventAction();
    
    /// Hook at the beginning of the event loop
    void BeginOfEventAction(const G4Event*);
    /// Hook at the end of the event loop
    void EndOfEventAction(const G4Event*);

  private:
    G4int _nevt; /// Event number
    G4int _nupd; /// 
  };
  
} // namespace nexus

#endif