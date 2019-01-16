#ifndef __CAR_IMPLEMENTED_H
#define __CAR_IMPLEMENTED_H

#include <car.h>

class CarImplemented : public Car
{
#define DoolpObject_CurrentObject CarImplemented
  DoolpObject_stdFeatures ( );
  DoolpObject_Option ( forceObjectAs, Car );
  DoolpObject_Options ( forceConstructorForObjects, Car );
 public:
  bool doolpfunclocal(switchOn) (unsigned int);
  bool doolpfunclocal(switchOff) (unsigned int);
  bool doolpfunclocal(getRadioList) (Doolp::Stream<int>*,Doolp::Stream<char*>*);

#undef DoolpObject_CurrentObject
};






#endif // __CAR_IMPLEMENTED_H
