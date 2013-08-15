// Author: Dizhi Zhou (dizhi.zhou@gmail.com)

#include <iostream>
#include "external_interface/ns3/ns3_os.h"
#include "external_interface/ns3/ns3_debug.h"
#include "external_interface/ns3/ns3_timer.h"
#include "external_interface/ns3/ns3_radio.h"
#include "external_interface/ns3/ns3_clock.h"
#include "external_interface/ns3/ns3_position.h"
#include "external_interface/ns3/ns3_distance.h"
#include "external_interface/ns3/ns3_rand.h"
#include "external_interface/ns3/ns3_facet_provider.h"
#include "external_interface/ns3/ns3_wiselib_application.h"
#include "algorithms/localization/distance_based/math/vec.h"

#include "ns3/simulator.h"
#include "ns3/ptr.h"

using namespace wiselib;

typedef Ns3OsModel Os;
#define MAX_NODES 3

class Ns3ExampleApplication
{
  public:

    void init (Os::AppMainParameter& value) 
      {  
        debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
        timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
        clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
        rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet( value );

        // each node has one radio instance
        // sender: radio[0]    receiver: radio[1], radio[2]
        for (uint16_t i = 0; i < MAX_NODES; i++)
          {
            radio[i] = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
            radio[i]->enable_radio ();
            
            // use different receive callbacks for receiver nodes
            if ( i == 1)
             radio[i]->reg_recv_callback <Ns3ExampleApplication,
                          &Ns3ExampleApplication::receive_radio1_message>(this);
            else if ( i == 2)
             radio[i]->reg_recv_callback <Ns3ExampleApplication,
                          &Ns3ExampleApplication::receive_radio2_message>(this);



            position[i] = &wiselib::FacetProvider<Os, Os::Position>::get_facet( value );
            position[i]->bind (radio[i]->id ()); // bind position facet and radio facet on the same node

            distance[i] = &wiselib::FacetProvider<Os, Os::Distance>::get_facet( value );
            distance[i]->bind (radio[i]->id ());
          }

        debug_->debug( "%f: Init simulation", clock_->time ());        

        timer_->set_timer<Ns3ExampleApplication,
                          &Ns3ExampleApplication::start_radio_facet>( 5000, this, 0 );

        timer_->set_timer<Ns3ExampleApplication,    
                      &Ns3ExampleApplication::start_clock_facet>( 6543.2, this, 0 );

        timer_->set_timer<Ns3ExampleApplication,
                          &Ns3ExampleApplication::start_distance_position_facet>( 7000, this, 0 );

        timer_->set_timer<Ns3ExampleApplication,
                          &Ns3ExampleApplication::start_rand_facet>( 8000, this, 0 );

      };


    void start_radio_facet (void*)
    {
       debug_->debug ("\n%f: Radio facet test", clock_->time () );
       debug_->debug( "  %f: Broadcast message at node %d", clock_->time (), radio[0]->id() );
       Os::Radio::block_data_t message1[] = "hello world broadcast!\0";
               radio[0]->send( Os::Radio::BROADCAST_ADDRESS, sizeof(message1), message1 );

       debug_->debug( "  %f: Send unicast message at node %d to %d",clock_->time (), radio[0]->id(), radio[1]->id() );
       Os::Radio::block_data_t message2[] = "hello world unicast!\0";
               radio[0]->send( radio[1]->id(), sizeof(message2), message2 );
    }

    void receive_radio1_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "  %f: Received msg1 at %u from %u",clock_->time (), radio[1]->id(), from );
         debug_->debug( "    message is %s", buf );
      }

    void receive_radio2_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "  %f: received msg2 at %u from %u",clock_->time (),  radio[2]->id(), from );
         debug_->debug( "    message is %s", buf );
      }

    void start_clock_facet (void*)
      {
         debug_->debug ("\n%f: Clock facet test", clock_->time () );
         debug_->debug( "  %d seconds since second %f", clock_->seconds (clock_->time ()), 0.0);
         debug_->debug( "  %d milliseconds from last second %d", clock_->milliseconds (clock_->time ()), 
                                                            (uint16_t)(clock_->time ()));
         debug_->debug( "  %d microseconds from last millisecond %d", clock_->microseconds (clock_->time () * 1000),
                                                            (uint16_t)(clock_->time () * 1000));
      }

    void start_distance_position_facet (void*)
      {
        debug_->debug ("\n%f: Positioan and distance facet test", clock_->time () );

        for (uint16_t i = 0; i < MAX_NODES; i++)
          {
            debug_->debug ("  Create Node %d at position ( %f, %f, %f )", 
                                  radio[i]->id (), 
                                  (position[i]->position ()).x(), 
                                  (position[i]->position ()).y(),
                                  (position[i]->position ()).z());
            debug_->debug ("    Distance from %d to %d is %f", radio[i]->id (), radio[0]->id (), 
                                                      distance[i]->distance(radio[0]->id ()) );
          }
      }

    void start_rand_facet (void*)
      {
         debug_->debug ("\n%f: Rand facet test", clock_->time () );
         debug_->debug( "  Generate random number (0,100): %d", (*rand_)(100));
      }

  private:
    Os::Debug::self_pointer_t debug_;
    Os::Timer::self_pointer_t timer_;
    Os::Radio::self_pointer_t radio[MAX_NODES];
    Os::Clock::self_pointer_t clock_;
    Os::Position::self_pointer_t position[MAX_NODES];
    Os::Distance::self_pointer_t distance[MAX_NODES];
    Os::Rand::self_pointer_t rand_;
};

wiselib::WiselibApplication<Os, Ns3ExampleApplication> example_app;

int main(int argc, char *argv[] )
{
   std::cout << std::endl; 

   Os::AppMainParameter value;  // the external interface instance

   example_app.init (value); // configure Wiselib script

   ns3::Simulator::Run ();   // start NS-3 simulation

   std::cout << std::endl;
}







