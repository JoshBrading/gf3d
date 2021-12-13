#include <stdlib.h>
#include <string.h>

#include "simple_logger.h"

#include "entity.h"

EntityManager entity_manager = { 0 };

void entity_system_close( )
{
    Uint32 i;
    for ( i = 0; i < entity_manager.entity_count; i++ )
    {
        entity_free( &entity_manager.entity_list[i] );
    }
    free( entity_manager.entity_list );
    memset( &entity_manager, 0, sizeof( EntityManager ) );
    slog( "entity_system: closed" );
}

void entity_system_init( Uint32 maxEntities )
{
    entity_manager.entity_list = gfc_allocate_array( sizeof( Entity ), maxEntities );
    if ( entity_manager.entity_list == NULL )
    {
        slog( "failed to allocate entity list, cannot allocate ZERO entities" );
        return;
    }
    entity_manager.entity_count = maxEntities;
    atexit( entity_system_close );
    slog( "entity_system: initialized" );
}

Entity* entity_new( )
{
    int i;
    for ( i = 0; i < entity_manager.entity_count; i++ )
    {
        if ( !entity_manager.entity_list[i]._inuse )// not used yet, so we can!
        {
            entity_manager.entity_list[i]._inuse = 1;
            gfc_matrix_identity( entity_manager.entity_list[i].modelMat );
            entity_manager.entity_list[i].scale.x = 1;
            entity_manager.entity_list[i].scale.y = 1;
            entity_manager.entity_list[i].scale.z = 1;
            return &entity_manager.entity_list[i];
        }
    }
    slog( "entity_new: no free space in the entity list" );
    return NULL;
}

void entity_free( Entity* self )
{
    if ( !self )return;
    //MUST DESTROY
    gf3d_model_free( self->model );
    memset( self, 0, sizeof( Entity ) );
}

void entity_draw( Entity* self, Uint32 frame )
{
    if ( !self )return;
    gf3d_model_draw( self->model, self->modelMat, frame );
}

void entity_draw_all( )
{
    int i;
    for ( i = 0; i < entity_manager.entity_count; i++ )
    {
        if ( !entity_manager.entity_list[i]._inuse )// not used yet
        {
            continue;// skip this iteration of the loop
        }      
        entity_draw( &entity_manager.entity_list[i], 1 );
    }
}

void entity_think( Entity* self )
{
    if ( !self )return;
    if ( self->think )self->think( self );
}

void entity_think_fixed( Entity* self )
{
    if ( !self )return;
    if ( self->thinkFixed )self->thinkFixed( self );
}

void entity_think_all( )
{
    int i;
    for ( i = 0; i < entity_manager.entity_count; i++ )
    {
        if ( !entity_manager.entity_list[i]._inuse )// not used yet
        {
            continue;// skip this iteration of the loop
        }
        entity_think( &entity_manager.entity_list[i] );
    }
}

void entity_think_fixed_all( )
{
    int i;
    for ( i = 0; i < entity_manager.entity_count; i++ )
    {
        if ( !entity_manager.entity_list[i]._inuse )// not used yet
        {
            continue;// skip this iteration of the loop
        }
        entity_think_fixed( &entity_manager.entity_list[i] );
    }
}

void entity_update( Entity* self )
{
    if ( !self )return;
    // HANDLE ALL COMMON UPDATE STUFF

    //vector3d_add( self->position, self->position, self->velocity );
    ////vector3d_add(self->velocity,self->acceleration,self->velocity);
    //
    //gfc_matrix_identity( self->modelMat );
    //gfc_matrix_scale( self->modelMat, self->scale );
    //
    //gfc_matrix_rotate( self->modelMat, self->modelMat, self->rotation.z, vector3d( 0, 0, 1 ) );
    //gfc_matrix_rotate( self->modelMat, self->modelMat, self->rotation.y, vector3d( 0, 1, 0 ) );
    //gfc_matrix_rotate( self->modelMat, self->modelMat, self->rotation.x, vector3d( 1, 0, 0 ) );
    //
    //gfc_matrix_translate( self->modelMat, self->position );

    if ( self->update )self->update( self );
}

void entity_update_fixed( Entity* self )
{
    if ( !self )return;
    // HANDLE ALL COMMON UPDATE STUFF

    vector3d_add( self->position, self->position, self->velocity );
    //vector3d_add(self->velocity,self->acceleration,self->velocity);

    gfc_matrix_identity( self->modelMat );
    gfc_matrix_scale( self->modelMat, self->scale );

    gfc_matrix_rotate( self->modelMat, self->modelMat, self->rotation.z, vector3d( 0, 0, 1 ) );
    gfc_matrix_rotate( self->modelMat, self->modelMat, self->rotation.y, vector3d( 0, 1, 0 ) );
    gfc_matrix_rotate( self->modelMat, self->modelMat, self->rotation.x, vector3d( 1, 0, 0 ) );

    gfc_matrix_translate( self->modelMat, self->position );

    if ( self->updateFixed )self->updateFixed( self );
}

void entity_update_all( )
{
    int i;
    for ( i = 0; i < entity_manager.entity_count; i++ )
    {
        if ( !entity_manager.entity_list[i]._inuse )// not used yet
        {
            continue;// skip this iteration of the loop
        }
        entity_update( &entity_manager.entity_list[i] );
    }
}

void entity_update_fixed_all( )
{
    int i;
    for ( i = 0; i < entity_manager.entity_count; i++ )
    {
        if ( !entity_manager.entity_list[i]._inuse )// not used yet
        {
            continue;// skip this iteration of the loop
        }
        entity_update_fixed( &entity_manager.entity_list[i] );
    }
}

//Entity* entity_get_by_tag( char* tag )
//{
//    for ( int i = 0; i < entity_manager.entity_count; i++ )
//    {
//        if ( entity_manager.entity_list[i].tag == tag )
//        {
//            return &entity_manager.entity_list[i];
//        }
//    }
//    slog( "entity_get_tag: No entity with tag %s", tag );
//    return NULL;
//}

Entity* entity_get_closest( Entity* self, float range, Uint8 teamMask, char* tagMask )
{
    if ( !self ) return;
    Entity* ent = NULL;
    float distance;
    for ( int i = 0; i < entity_manager.entity_count; i++ )
    {
        if ( self != &entity_manager.entity_list[i] && entity_manager.entity_list[i].tag != tagMask && entity_manager.entity_list[i].team != teamMask )
        {
            distance = vector3d_magnitude_between( self->position, entity_manager.entity_list[i].position );

            if ( distance < range )
            {
                range = distance;
                ent = &entity_manager.entity_list[i];
            }
        }
    }
    return ent;
}

Entity* entity_get_in_row( Entity* self, float range, Uint8 teamMask, char* tagMask )
{
    if ( !self ) return;
    Entity* ent = NULL;
    float distance;
    for ( int i = 0; i < entity_manager.entity_count; i++ )
    {
        if ( self != &entity_manager.entity_list[i] && entity_manager.entity_list[i].tag != tagMask && entity_manager.entity_list[i].team != teamMask )
        {
            if ( self->team == 0 && self->position.y == entity_manager.entity_list[i].position.y && self->position.x <= entity_manager.entity_list[i].position.x )
            {
                distance = vector3d_magnitude_between( self->position, entity_manager.entity_list[i].position );

                if ( distance < range )
                {
                    range = distance;
                    ent = &entity_manager.entity_list[i];
                }
            }
            if ( self->team == 1 && self->position.y == entity_manager.entity_list[i].position.y && self->position.x >= entity_manager.entity_list[i].position.x )
            {
                distance = vector3d_magnitude_between( self->position, entity_manager.entity_list[i].position );

                if ( distance < range )
                {
                    range = distance;
                    ent = &entity_manager.entity_list[i];
                }
            }
        }
    }
    return ent;
}

EntityManager* entity_get_manager( )
{
    return &entity_manager;
}

/*eol@eof*/
