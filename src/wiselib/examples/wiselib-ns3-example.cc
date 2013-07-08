
// Author: Dizhi Zhou (dizhi.zhou@gmail.com)

#include <iostream>
#include "external_interface/ns3/ns3_os.h"
#include "external_interface/ns3/ns3_debug.h"
#include "external_interface/ns3/ns3_timer.h"
#include "external_interface/ns3/ns3_radio.h"
#include "external_interface/ns3/ns3_facet_provider.h"
#include "external_interface/ns3/ns3_wiselib_application.h"

#include "ns3/simulator.h"
#include "ns3/ptr.h"

using namespace wiselib;

typedef Ns3OsModel Os;

class Ns3ExampleApplication
{
  public:

    void init (Os::AppMainParameter& value) 
      {  
        debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
        timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
        radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
          
        debug_->debug( "Init simulation");        

        radio_->init_radio (); // create wifi ad hoc network with default parameters

        radio_->reg_recv_callback <Ns3ExampleApplication,
                          &Ns3ExampleApplication::receive_radio_message>(this);

        timer_->set_timer<Ns3ExampleApplication,
                          &Ns3ExampleApplication::start>( 5000, this, 0 );
      };


    void start (void*)
    {
       debug_->debug ("Start simulation \n");
       debug_->debug( "broadcast message at node %d \n", radio_->id() );
       Os::Radio::block_data_t message[] = "hello world!\0";
               radio_->send( Os::Radio::BROADCAST_ADDRESS, sizeof(message), message );
    }

    void receive_radio_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "received msg at %u from %u\n", radio_->id(), from );
         debug_->debug( "  message is %s\n", buf );
      }


  private:
    Os::Debug::self_pointer_t debug_;
    Os::Timer::self_pointer_t timer_;
    Os::Radio::self_pointer_t radio_;
};

wiselib::WiselibApplication<Os, Ns3ExampleApplication> example_app;

int main(int argc, char *argv[] )
{
   Os::AppMainParameter value;  // value is an empty struct variable now

   example_app.init (value); // configure Wiselib script
 
   ns3::Simulator::Run ();   // start NS-3 simulation
}







