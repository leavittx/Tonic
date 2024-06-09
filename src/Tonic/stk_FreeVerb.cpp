//
//  stk_FreeVerb.cpp
//  Tonic
//


#include "stk_FreeVerb.h"

namespace Tonic { namespace Tonic_{

  FreeVerb_::FreeVerb_(){
    
    setIsStereoOutput(true);
    
    // Default to 50% wet
    setDryLevelGen(FixedValue(1.0f));
    setWetLevelGen(FixedValue(0.5f));
  }

} // Namespace Tonic_
  
  
  
} // Namespace Tonic
