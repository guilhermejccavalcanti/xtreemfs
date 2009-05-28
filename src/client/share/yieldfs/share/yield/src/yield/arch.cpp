// Revision: 1493

#include "yield/arch.h"
using namespace YIELD;


// event_handler.cpp
// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).
void EventHandler::handleUnknownEvent( Event& ev )
{
  switch ( ev.get_tag() )
  {
    case YIELD_OBJECT_TAG( StageStartupEvent ):
    case YIELD_OBJECT_TAG( StageShutdownEvent ): Object::decRef( ev ); break;
    default:
    {
      std::cerr << get_type_name() << " dropping unknown event: " << ev.get_type_name() << std::endl;
      Object::decRef( ev );
    }
    break;
  }
}
bool EventHandler::send( Event& ev )
{
  if ( redirect_to_event_target )
    return redirect_to_event_target->send( ev );
  else if ( isThreadSafe() )
    handleEvent( ev );
  else
  {
    handleEvent_lock.acquire();
    handleEvent( ev );
    handleEvent_lock.release();
  }
  return true;
}


// seda_stage_group.cpp
// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).
namespace YIELD
{
  class SEDAStageGroupThread : public StageGroupThread
  {
  public:
    SEDAStageGroupThread( const std::string& stage_group_name, auto_Object<ProcessorSet> limit_logical_processor_set, auto_Object<Log> log, auto_Object<Stage> stage )
      : StageGroupThread( stage_group_name, limit_logical_processor_set, log ), stage( stage )
    { }
    auto_Object<Stage> get_stage() { return stage; }
    // Object
    YIELD_OBJECT_PROTOTYPES( SEDAStageGroupThread, 0 );
    // StageGroupThread
    void stop()
    {
      should_run = false;
      auto_Object<StageShutdownEvent> stage_shutdown_event = new StageShutdownEvent;
      for ( ;; )
      {
        stage->send( stage_shutdown_event->incRef() );
        if ( is_running )
          Thread::sleep( 50 * NS_IN_MS );
        else
          break;
      }
    }
  private:
    ~SEDAStageGroupThread() { }
    auto_Object<Stage> stage;
    // StageGroupThread
    void _run()
    {
      Thread::set_name( stage->get_stage_name() );
      stage->get_event_handler()->handleEvent( *( new StageStartupEvent( stage ) ) );
      while ( should_run )
        visitStage( *stage );
    }
  };
};
SEDAStageGroup::~SEDAStageGroup()
{
  for ( std::vector<SEDAStageGroupThread*>::iterator thread_i = threads.begin(); thread_i != threads.end(); thread_i++ )
    ( *thread_i )->stop();
  for ( std::vector<SEDAStageGroupThread*>::iterator thread_i = threads.begin(); thread_i != threads.end(); thread_i++ )
    Object::decRef( **thread_i );
}
void SEDAStageGroup::startThreads( auto_Object<Stage> stage, int16_t thread_count )
{
  for ( unsigned short thread_i = 0; thread_i < thread_count; thread_i++ )
  {
    SEDAStageGroupThread* thread = new SEDAStageGroupThread( get_name(), NULL, get_log(), stage );
    thread->start();
    this->threads.push_back( thread );
  }
}


// stage_group.cpp
// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).


#ifdef YIELD_RECORD_PERFCTRS
#ifdef __sun
#include <cstdlib>
#else
#error
#endif
#endif


StageGroup::StageGroup( const std::string& name, auto_Object<ProcessorSet> limit_physical_processor_set, auto_Object<EventTarget> stage_stats_event_target )
: name( name ), limit_physical_processor_set( limit_physical_processor_set ), stage_stats_event_target( stage_stats_event_target )
{
  if ( limit_physical_processor_set != NULL )
  {
    limit_logical_processor_set = new ProcessorSet;
    uint16_t online_physical_processor_count = Machine::getOnlinePhysicalProcessorCount();
    uint16_t logical_processors_per_physical_processor = Machine::getOnlinePhysicalProcessorCount();

    for ( uint16_t physical_processor_i = 0; physical_processor_i < online_physical_processor_count; physical_processor_i++ )
    {
      if ( limit_physical_processor_set->isset( physical_processor_i ) )
      {
        for ( uint16_t logical_processor_i = physical_processor_i * logical_processors_per_physical_processor; logical_processor_i < ( physical_processor_i + 1 ) * logical_processors_per_physical_processor; logical_processor_i++ )
          limit_logical_processor_set->set( logical_processor_i );
      }
    }
  }

  running_stage_group_thread_tls_key = Thread::createTLSKey();
}


// stage_group_thread.cpp
// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).
#ifdef YIELD_RECORD_PERFCTRS
#ifdef __sun
#include <cstdlib>
#else
#error
#endif
#endif
StageGroupThread::StageGroupThread( const std::string& stage_group_name, auto_Object<ProcessorSet> limit_logical_processor_set, auto_Object<Log> log )
  : stage_group_name( stage_group_name ), limit_logical_processor_set( limit_logical_processor_set ), log( log )
{
  is_running = false;
  should_run = true;
#ifdef YIELD_RECORD_PERFCTRS
#ifdef __sun
  if ( ( cpc = cpc_open( CPC_VER_CURRENT ) ) != NULL &&
         ( cpc_set = cpc_set_create( cpc ) ) != NULL )
  {
    const char* pic0_str = getenv( "PIC0" );
    if ( pic0_str == NULL ) pic0_str = "L2_imiss";
    const char* pic1_str = getenv( "PIC1" );
    if ( pic1_str == NULL ) pic1_str = "L2_dmiss_ld";
    if ( ( pic0_index = cpc_set_add_request( cpc, cpc_set, pic0_str, 0, CPC_COUNT_USER, 0, NULL ) ) != -1 &&
       ( pic1_index = cpc_set_add_request( cpc, cpc_set, pic1_str, 0, CPC_COUNT_USER, 0, NULL ) ) != -1 )
    {
      if ( ( before_cpc_buf = cpc_buf_create( cpc, cpc_set ) ) != NULL &&
         ( after_cpc_buf = cpc_buf_create( cpc, cpc_set ) ) != NULL &&
         ( diff_cpc_buf = cpc_buf_create( cpc, cpc_set ) ) != NULL )
      {
        if ( cpc_bind_curlwp( cpc, cpc_set, 0 ) != -1 )
        {
          cpc_unbind( cpc, cpc_set );
          return;
        }
      }
    }
  }
  if ( cpc != NULL )
    cpc_close( cpc );
  throw Exception();
#endif
#endif
}
StageGroupThread::~StageGroupThread()
{
#ifdef YIELD_RECORD_PERFCTRS
#ifdef __sun
  cpc_unbind( cpc, cpc_set );
  cpc_set_destroy( cpc, cpc_set );
  cpc_close( cpc );
#endif
#endif
}
void StageGroupThread::start()
{
  Thread::start();
  while ( !is_running )
    Thread::yield();
}
void StageGroupThread::run()
{
  Thread::setCurrentThreadName( stage_group_name.c_str() );
  if ( log != NULL )
    log->getStream( Log::LOG_DEBUG ) << stage_group_name << ": starting thread #" << this->get_id() << ".";
  if ( limit_logical_processor_set != NULL )
  {
    if ( !this->set_processor_affinity( *limit_logical_processor_set ) && log != NULL )
      log->getStream( Log::LOG_DEBUG ) << stage_group_name << "could not set processor affinity of thread #" << this->get_id() << ", error: " << Exception::strerror();
  }
//  Thread::setTLS( stage_group.get_running_stage_group_thread_tls_key(), this );
#ifdef YIELD_RECORD_PERFCTRS
#ifdef __sun
  cpc_bind_curlwp( cpc, cpc_set, 0 );
#endif
#endif
  is_running = true;
  _run();
  is_running = false;
}

