#ifndef __CAR_H
#define __CAR_H

#include <doolp.h>
#include <string>

class Car;

class Radio : public Doolp::Object
{
#define DoolpObject_CurrentObject Radio
  DoolpObject_stdFeatures ( );
 public:
  Radio () { Log ( "New Radio\n" ); }
  doolpuniquelink<Car> myCar;
#undef DoolpObject_CurrentObject
};


class CarWindow : public Doolp::Object
{
#define DoolpObject_CurrentObject CarWindow
  DoolpObject_stdFeatures ( );
 public:
  CarWindow () { Log ( "New CarWindow\n" ); }
  doolpuniquelink<Car> myCar;
#undef DoolpObject_CurrentObject
};


class Car : public Doolp::Object
{
#define DoolpObject_CurrentObject Car
  DoolpObject_stdFeatures ( );

 public:
  Car::Car ( )
    {
      Log ( "New Car at %p\n", this );
    }
  Car::~Car ( )
    {
      Log ( "Deleting Car at %p\n", this );
      // free ( (char*) name );
      // free ( (char*) serial );
    }
  doolpparam<string*> name;
  doolpparam<char *> serial;
  doolpparam<bool> switchedOn;
  doolpparam<float> oilGauge;
  doolpparam<float> fuelGauge;
  doolpuniquelink<Radio> myRadio; // ( Radio::myCar );
  DoolpObjectLink_setReverse ( myRadio, myCar );
  doolpmultilink<CarWindow>  myWindows;
  DoolpObjectLink_setReverse ( myWindows, myCar );
  //  DoolpObject_Options ( setReverseLink, myRadio, myCar );

  doolpfunc<bool,unsigned int> switchOn;
  doolpfunc<bool,unsigned int> switchOff;
  doolpfunc<bool,Doolp::Stream<int>*,Doolp::Stream<char*>*> getRadioList;
#undef DoolpObject_CurrentObject
};

#endif // __CAR_H
