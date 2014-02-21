#include "g_local.h"
#include "g_as_local.h"
#include "../matchmaker/mm_query.h"
#include "../qalgo/base64.h"
#include "../qalgo/sha2.h"

stat_query_api_t *rs_sqapi;

cvar_t *rs_statsEnabled;
cvar_t *rs_statsUrl;
cvar_t *rs_statsKey;
cvar_t *rs_grenade_minKnockback;
cvar_t *rs_grenade_maxKnockback;
cvar_t *rs_grenade_splash;
cvar_t *rs_grenade_speed;
cvar_t *rs_grenade_timeout;
cvar_t *rs_grenade_gravity;
cvar_t *rs_grenade_friction;
cvar_t *rs_grenade_prestep;
cvar_t *rs_rocket_minKnockback;
cvar_t *rs_rocket_maxKnockback;
cvar_t *rs_rocket_splash;
cvar_t *rs_rocket_speed;
cvar_t *rs_rocket_prestep;
cvar_t *rs_rocket_antilag;
cvar_t *rs_plasma_minKnockback;
cvar_t *rs_plasma_maxKnockback;
cvar_t *rs_plasma_splash;
cvar_t *rs_plasma_speed;
cvar_t *rs_plasma_prestep;
cvar_t *rs_plasma_hack;
cvar_t *rs_gunblade_minKnockback;
cvar_t *rs_gunblade_maxKnockback;
cvar_t *rs_gunblade_splash;
cvar_t *rs_splashfrac;
void RS_Init( void );
void RS_Shutdown( void );
void RS_removeProjectiles( edict_t *owner );

/**
 * RS_Init
 * Initializes the racesow specific variables
 */
void RS_Init( void )
{
	rs_grenade_minKnockback = trap_Cvar_Get( "rs_grenade_minKnockback", "1", CVAR_ARCHIVE );
	rs_grenade_maxKnockback = trap_Cvar_Get( "rs_grenade_maxKnockback", "120", CVAR_ARCHIVE );
	rs_grenade_splash = trap_Cvar_Get( "rs_grenade_splash", "170", CVAR_ARCHIVE );
	rs_grenade_speed = trap_Cvar_Get( "rs_grenade_speed", "800", CVAR_ARCHIVE );
	rs_grenade_timeout = trap_Cvar_Get( "rs_grenade_timeout", "1650", CVAR_ARCHIVE );
	rs_grenade_gravity = trap_Cvar_Get( "rs_grenade_gravity", "1.22", CVAR_ARCHIVE );
	rs_grenade_friction = trap_Cvar_Get( "rs_grenade_friction", "0.85", CVAR_ARCHIVE );
	rs_grenade_prestep = trap_Cvar_Get( "rs_grenade_prestep", "24", CVAR_ARCHIVE );
	rs_rocket_minKnockback = trap_Cvar_Get( "rs_rocket_minKnockback", "1", CVAR_ARCHIVE );
	rs_rocket_maxKnockback = trap_Cvar_Get( "rs_rocket_maxKnockback", "100", CVAR_ARCHIVE );
	rs_rocket_splash = trap_Cvar_Get( "rs_rocket_splash", "120", CVAR_ARCHIVE );
	rs_rocket_speed = trap_Cvar_Get( "rs_rocket_speed", "950", CVAR_ARCHIVE );
	rs_rocket_prestep = trap_Cvar_Get( "rs_rocket_prestep", "10", CVAR_ARCHIVE );
	rs_rocket_antilag = trap_Cvar_Get( "rs_rocket_antilag", "0", CVAR_ARCHIVE );
	rs_plasma_minKnockback = trap_Cvar_Get( "rs_plasma_minKnockback", "1", CVAR_ARCHIVE );
	rs_plasma_maxKnockback = trap_Cvar_Get( "rs_plasma_maxKnockback", "23", CVAR_ARCHIVE );
	rs_plasma_splash = trap_Cvar_Get( "rs_plasma_splash", "45", CVAR_ARCHIVE );
	rs_plasma_speed = trap_Cvar_Get( "rs_plasma_speed", "1700", CVAR_ARCHIVE );
	rs_plasma_prestep = trap_Cvar_Get( "rs_plasma_prestep", "32", CVAR_ARCHIVE );
	rs_plasma_hack = trap_Cvar_Get( "rs_plasma_hack", "1", CVAR_ARCHIVE );
	rs_gunblade_minKnockback = trap_Cvar_Get( "rs_gunblade_minKnockback", "10", CVAR_ARCHIVE ); // TODO: decide gunblade values
	rs_gunblade_maxKnockback = trap_Cvar_Get( "rs_gunblade_maxKnockback", "60", CVAR_ARCHIVE );
	rs_gunblade_splash = trap_Cvar_Get( "rs_gunblade_splash", "80", CVAR_ARCHIVE );
	rs_splashfrac = trap_Cvar_Get( "rs_splashfrac", "0", CVAR_ARCHIVE );

	// TODO: Decide what these flags should really be
	rs_statsEnabled = trap_Cvar_Get( "rs_statsEnabled", "0", CVAR_ARCHIVE );
	rs_statsUrl = trap_Cvar_Get( "rs_statsUrl", "localhost", CVAR_ARCHIVE );
	rs_statsKey = trap_Cvar_Get( "rs_statsKey", "", CVAR_ARCHIVE );
	rs_sqapi = trap_GetStatQueryAPI();
	if( !rs_sqapi )
		trap_Cvar_ForceSet( rs_statsEnabled->name, "0" );
}

/**
 * RS_Shutdown
 * Racesow cleanup
 */
void RS_Shutdown( void )
{
}

/**
 * RS_removeProjectiles
 * Removes all projectiles for a given player
 * @param owner The player whose projectiles to remove
 */
void RS_removeProjectiles( edict_t *owner )
{
	edict_t *ent;

	for( ent = game.edicts + gs.maxclients; ENTNUM( ent ) < game.numentities; ent++ )
	{
		if( ent->r.inuse && !ent->r.client && ent->r.svflags & SVF_PROJECTILE && ent->r.solid != SOLID_NOT && ent->r.owner == owner )
			G_FreeEdict( ent );
	}
}

/**
 * RS_SplashFrac
 * Racesow version of G_SplashFrac by Weqo
 */
void RS_SplashFrac( const vec3_t origin, const vec3_t mins, const vec3_t maxs, const vec3_t point, float maxradius, vec3_t pushdir, float *kickFrac, float *dmgFrac )
{
	vec3_t boxcenter = { 0, 0, 0 };
	float distance = 0;
	int i;
	float innerradius;
	float outerradius;
	float g_distance;
	float h_distance;

	if( maxradius <= 0 )
	{
		if( kickFrac )
			*kickFrac = 0;
		if( dmgFrac)
			*dmgFrac = 0;

		return;
	}

	innerradius = ( maxs[0] + maxs[1] - mins[0] - mins[1] ) * 0.25;
	outerradius = ( maxs[2] - mins[2] ); // cylinder height

	// find center of the box
	for( i = 0; i < 3; i++ )
		boxcenter[i] = origin[i] + maxs[i] + mins[i];

	// find box radius to explosion origin direction
	VectorSubtract( boxcenter, point, pushdir );

	g_distance = sqrt( pushdir[0]*pushdir[0] + pushdir[1]*pushdir[1] ); // distance on virtual ground
	h_distance = fabs( pushdir[2] );				    // corrected distance in height

	if( ( h_distance <= outerradius / 2 ) || ( g_distance > innerradius ) )
		distance = g_distance - innerradius;

	if( ( h_distance > outerradius / 2 ) || ( g_distance <= innerradius ) )
		distance = h_distance - outerradius / 2;

	if( ( h_distance > outerradius / 2 ) || ( g_distance > innerradius ) )
		distance = sqrt( ( g_distance - innerradius ) * ( g_distance - innerradius ) + ( h_distance - outerradius / 2 ) * ( h_distance - outerradius / 2 ) );

	if( dmgFrac )
	{
		// soft sin curve
		*dmgFrac = sin( DEG2RAD( ( distance / maxradius ) * 80 ) );
		clamp( *dmgFrac, 0.0f, 1.0f );
	}

	if( kickFrac )
	{
		*kickFrac = 1.0 - fabs( distance / maxradius );
		clamp( *kickFrac, 0.0f, 1.0f );
	}

	VectorSubtract( boxcenter, point, pushdir );
	VectorNormalizeFast( pushdir );
}

/**
 * RS_GenToken
 * Generate the server token for the given salt
 * @return The generated token
 */
const char *RS_GenToken( const char *salt )
{
	unsigned char digest[SHA256_DIGEST_SIZE];
	size_t *outlen;

	const char *message = va( "%s|%s", salt, rs_statsKey->string );

	sha256( (const unsigned char*)message, strlen( message ), digest );
	return (const char*)base64_encode( digest, (size_t)SHA256_DIGEST_SIZE, outlen );
}

/**
 * AuthPlayer callback function
 * @param query Query calling this function
 * @param success True on any response
 * @param customp Extra parameters, should be NULL
 * @return void
 */
void RS_AuthPlayer_Done( stat_query_t *query, qboolean success, void *customp )
{
	int error;
	asIScriptContext *ctx;

	if( !level.gametype.authPlayerDone )
		return;

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.authPlayerDone) );
	if( error < 0 ) 
		return;

	// Set the parameters
	ctx->SetArgDWord( 0, rs_sqapi->GetStatus( query ) );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		GT_asShutdownScript();
}

/**
 * Authenticate a player
 * @param name The player's auth name
 * @param ctoken The client's signed token
 * @param realTime Time tokens were generated with
 * @return void
 */
void RS_AuthPlayer( const char *name, const char *ctoken, uint authTime )
{
	stat_query_t *query;
	const char *stoken;

	if( !name || !strlen( name ) || !ctoken || !strlen( ctoken ) )
		return;

	stoken = RS_GenToken( va( "%d", authTime ) );

	G_Printf( "QueryPlayer %s %s %d\n", name, ctoken, authTime );
	G_Printf( "player/%s?time=%d?stoken=%s?ctoken=%s\n", name, authTime,	stoken,	ctoken );

	query = rs_sqapi->CreateQuery(
		va( "player/%s?time=%d?stoken=%s?ctoken=%s", name, authTime, stoken, ctoken ),
		qtrue );
	rs_sqapi->SetCallback( query, RS_AuthPlayer_Done, NULL );
	trap_MM_SendQuery( query );
	query = NULL;
}