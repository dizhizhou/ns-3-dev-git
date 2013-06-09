
// Author: Dizhi Zhou (dizhi.zhou@gmail.com)

#include <iostream>
#include "external_interface/ns3/ns3_os.h"
#include "external_interface/ns3/ns3_debug.h"
#include "external_interface/ns3/ns3_facet_provider.h"
#include "external_interface/ns3/ns3_wiselib_application.h"


using namespace wiselib;

typedef Ns3OsModel Os;

class Ns3ExampleApplication
{
  public:

    void init (Os::AppMainParameter& value) 
      {  
        debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

        debug_->debug( "Hello world!");
   
      };

  private:
    Os::Debug::self_pointer_t debug_;
};

wiselib::WiselibApplication<Os, Ns3ExampleApplication> example_app;

int main(int argc, char *argv[] )
{
   Os::AppMainParameter value;  // value is an empty struct variable now

   example_app.init (value);
}







