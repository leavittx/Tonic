//
//  ControlParameter.h
//  Tonic 
//
//  Created by Nick Donaldson on 5/14/13.
//
// See LICENSE.txt for license and usage information.
//


#ifndef TONIC_CONTROLPARAMETER_H
#define TONIC_CONTROLPARAMETER_H

#include "ControlValue.h"

namespace Tonic {
  
  typedef enum{
    
    ControlParameterTypeContinuous = 0,
    ControlParameterTypeToggle,
    ControlParameterTypeMomentary,
    ControlParameterTypeInteger,
    
  } ControlParameterType;
  
  namespace Tonic_ {
      
    
    //! Parameter for controlling a Synth instance
    /*! 
        ControlParameter acts like a ControlValue but provides an interface for exposing itself to
        a synth, including min, max, type, etc
     */
    class ControlParameter_ : public ControlValue_{
      
    protected:
      
      std::string           name_;
      std::string           displayName_;
      TonicFloat            min_;
      TonicFloat            max_;
      ControlParameterType  type_;
      
      bool                isLogarithmic_;
      bool                isDb_;
      
    public:
      
      ControlParameter_();
      
      void        setName( std::string name ) { name_ = name; };
      std::string getName() { return name_; };
      
      void        setDisplayName( std::string displayName ) { displayName_ = displayName; };
      std::string getDisplayName() { return displayName_; };
      
      void        setMin( TonicFloat min ) { min_ = min; };
      TonicFloat  getMin() { return min_; };
      
      void        setMax( TonicFloat max ) { max_ = max; };
      TonicFloat  getMax() { return max_; };
      
      void                  setType( ControlParameterType type ) { type_ = type; };
      ControlParameterType  getType() { return type_; };
      
      void        setIsLogarithmic(bool isLogarithmic) { isLogarithmic_ = isLogarithmic; };
      bool        getIsLogarithmic() { return isLogarithmic_; };

      bool        getIsDb() { return isDb_; }
      void        setIsDb(bool isDb) { isDb_ = isDb; }
    
      void        setNormalizedValue(TonicFloat normVal);
      TonicFloat  getNormalizedValue();
    
    };
    
  }
  
  class ControlParameter : public TemplatedControlGenerator<Tonic_::ControlParameter_>{
    
  public:
        
    std::string         getName();
    ControlParameter &  name(std::string name);

    std::string         getDisplayName();
    ControlParameter &  displayName(std::string displayName);
    
    TonicFloat          getValue();
    ControlParameter &  value(TonicFloat value);

    TonicFloat          getMin();
    ControlParameter &  min(TonicFloat min);
    
    TonicFloat          getMax();
    ControlParameter &  max(TonicFloat max);
    
    ControlParameterType  getParameterType();
    ControlParameter &    parameterType(ControlParameterType type);
    
    bool                getIsLogarithmic();
    ControlParameter &  logarithmic(bool isLogarithmic);

    bool                getIsDb();
    ControlParameter &  db(bool isDb);
    
    // Convenience methods for setting/getting value normalized linearly 0-1,
    // mapped to min/max range, with log applied if necessary
    TonicFloat          getNormalizedValue();
    ControlParameter &  setNormalizedValue(TonicFloat value);
  };
}

#endif


