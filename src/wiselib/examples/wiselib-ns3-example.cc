
// Author: Dizhi Zhou (dizhi.zhou@gmail.com)

#include <iostream>
#include "external_interface/ns3/ns3_os.h"
#include "external_interface/ns3/ns3_debug.h"
#include "external_interface/ns3/ns3_timer.h"
#include "external_interface/ns3/ns3_facet_provider.h"
#include "external_interface/ns3/ns3_wiselib_application.h"

#include "ns3/simulator.h"

using namespace wiselib;

typedef Ns3OsModel Os;

class Ns3ExampleApplication
{
  public:

    void init (Os::AppMainParameter& value) 
      {  
        debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
        timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
           
        debug_->debug( "Hello world!");        

        timer_->set_timer<Ns3ExampleApplication,
                          &Ns3ExampleApplication::start>( 5000, this, 0 );
      };


    void start (void*)
    {
       debug_->debug ("Start simulation \n");
    }

  private:
    Os::Debug::self_pointer_t debug_;
    Os::Timer::self_pointer_t timer_;

};

wiselib::WiselibApplication<Os, Ns3ExampleApplication> example_app;

int main(int argc, char *argv[] )
{
   Os::AppMainParameter value;  // value is an empty struct variable now

   example_app.init (value); // configure Wiselib script
 
   ns3::Simulator::Run ();   // start NS-3 simulation
}







