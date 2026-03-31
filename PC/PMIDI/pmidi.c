/* within JLN's dev kit :
gcc -Wall -O3 -o pmidi.exe pmidi.c -L. -lportmidi -lwinmm

Appli derivee de pm_test/test.c, hyper-simplifiee :
	- pas de test de crash
	- pas de console input
	- latence fixe, minimale
	- affichage delta-t entre events (en ms)
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#include "portmidi.h"
#include "porttime.h"

#define INPUT_BUFFER_SIZE 128
#define OUTPUT_BUFFER_SIZE 128
#define TIME_PROC ((int32_t (*)(void)) Pt_Time)
#define TIME_INFO NULL


// boucler indefiniment en affichant les messages entrants,
// et le delay entre chaque message et le precedent
void main_test_input( int idev )
{
PmStream * midi;
PmEvent buffer[1];
// PmError status;

Pt_Start( 1, 0, 0 ); /* timer started w/millisecond accuracy */

/* open input device */

Pm_OpenInput( &midi, idev, NULL, INPUT_BUFFER_SIZE, ((int32_t (*)(void *)) Pt_Time), NULL );
printf("Midi Input #%d opened\n", idev);

Pm_SetFilter( midi, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX );

// purge
while	( Pm_Poll(midi) )
	Pm_Read( midi, buffer, 1 );

/* now start paying attention to messages */
int i = 0; 		// count messages
int previous = 0;	// timestamp du message precedent

while	(1)
	{
	int status = Pm_Poll(midi);		// c'est pas bloquant ??? du gros polling !
	if	( status == TRUE )
		{
		int length, now, delta;
		length = Pm_Read(midi,buffer, 1);
		if	( length > 0 )
			{
			now = buffer[0].timestamp;				// c'est un int32
			delta = now - previous;
			previous = now;
			printf("#%6d : @ %6d (%6d), %2x %2x %2x\n",
				i, now, delta,
				Pm_MessageStatus( buffer[0].message ),		// on extrait des bytes
				Pm_MessageData1( buffer[0].message ),		// de .message, qui est
				Pm_MessageData2( buffer[0].message )   );	// aussi un int32
			i++;
			}
		}
        }

// dead code, sorry
Pm_Close(midi);
printf("done closing...");
}

// boucler indefiniment en emettant un 'note on' et un 'note off' par seconde (approx.)
// experience echo distant : 
// si idev >=0, afficher les messages entrant, avec le delai depuis le dernier sortant
void main_test_output( int odev, int idev )
{
PmStream * omidi, *imidi;
PmEvent buffer[1];
unsigned int cnt, Note;

Pt_Start( 1, 0, 0 ); /* timer started w/millisecond accuracy */

Pm_OpenOutput( &omidi, odev, NULL, OUTPUT_BUFFER_SIZE, NULL, NULL, 0 ); 
if	( idev >= 0 )
	{
	Pm_OpenInput( &imidi, idev, NULL, INPUT_BUFFER_SIZE, ((int32_t (*)(void *)) Pt_Time), NULL );
	Pm_SetFilter( imidi, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX );
	}

cnt = 0;
if	( idev < 0 )
	{
	while	(1)
		{
		Note = 59 + (cnt++) % 13;
		//PmError Pm_WriteShort( PortMidiStream *stream, PmTimestamp when, int32_t msg);
		Pm_WriteShort( omidi, 0, Pm_Message( 0x94, Note, 100 ) );	Pt_Sleep(500);
		Pm_WriteShort( omidi, 0, Pm_Message( 0x84, Note, 40 ) );	Pt_Sleep(500);
		}
	}
else	{
	struct timespec ts_now, ts_then;
	clock_getres( CLOCK_REALTIME, &ts_now );
	printf("CLOCK_REALTIME resolution is %u ns\n", (unsigned int)ts_now.tv_nsec );
	while	(1)
		{
		Note = 59 + (cnt++) % 13;
		int now = Pt_Time(); clock_gettime( CLOCK_REALTIME, &ts_now );
		Pm_WriteShort( omidi, 0, Pm_Message( 0x94, Note, 101 ) );
		do	{	// quelques ms de polling intensif
			if	( Pm_Poll(imidi) )
				{
				int delta1, delta2;
				if	( Pm_Read( imidi, buffer, 1 ) )
					{
					// delta0 = buffer[0].timestamp - now; 		// marche pas
					delta1 = Pt_Time() - now; clock_gettime( CLOCK_REALTIME, &ts_then );
					delta2 = (int)ts_then.tv_nsec - (int)ts_now.tv_nsec;
					if	( delta2 < 0 ) delta2 += 1000000000;
					printf("#%3d : dt = %3dm (%6du), %02x, %02x (%02x), %02x\n",
						cnt, delta1, delta2/1000,
						Pm_MessageStatus( buffer[0].message ),
						Pm_MessageData1( buffer[0].message ), Note,
						Pm_MessageData2( buffer[0].message )   );
					}
				break;
				}
			} while	( ( Pt_Time() - now ) < 20 );
 		Pt_Sleep(500);
		Pm_WriteShort( omidi, 0, Pm_Message( 0x84, Note, 41 ) );
 		Pt_Sleep(500);
		while	( Pm_Poll(imidi) )		// purge
			Pm_Read( imidi, buffer, 1 );
		}
	}
}

// boucler indefiniment en emettant au plus tot l'echo de chaque message entrant
void main_test_echo( int idev, int odev )
{
PmStream * imidi, * omidi;
PmEvent buffer[1];

Pt_Start( 1, 0, 0 ); /* timer started w/millisecond accuracy */
Pm_OpenInput(  &imidi, idev, NULL, INPUT_BUFFER_SIZE, ((int32_t (*)(void *)) Pt_Time), NULL );
Pm_OpenOutput( &omidi, odev, NULL, OUTPUT_BUFFER_SIZE, NULL, NULL, 0 ); 
Pm_SetFilter( imidi, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX );

while	(1)
	{
	if	( Pm_Poll(imidi) )
		{
		int length;
		length = Pm_Read( imidi, buffer, 1 );
		if	( length > 0 )
			{
			Pm_Write( omidi, buffer, 1 );
			printf("echoed %2x %2x %2x\n",
				Pm_MessageStatus( buffer[0].message ),		// on extrait des bytes
				Pm_MessageData1( buffer[0].message ),		// de .message, qui est
				Pm_MessageData2( buffer[0].message )   );	// aussi un int32
			}
		}
	}
}

/* list device information */
void list()
{
int idev; const PmDeviceInfo *info;
int default_in = Pm_GetDefaultInputDeviceID();
int default_out = Pm_GetDefaultOutputDeviceID();
for	( idev = 0; idev < Pm_CountDevices(); idev++ )
	{
	info = Pm_GetDeviceInfo( idev );
	printf("%d ", idev );
	if	( info->input )
                printf("input ");
	else if	( info->output )
                printf("output");
	printf(" : %s : %s %s\n", info->interf, info->name, (((idev==default_in)||(idev==default_out))?("default"):("")) );
        }
}

void usage()
{
printf(
"\nTest MIDI I/O\n"
"  Avec 1 numero de port en argument :\n"
"   - emission continue (1 note/s env.) si c'est un port out\n"
"   - reception continue si c'est un port in\n"
"  Avec 2 numeros de port en argument, un in, un out :\n"
"   - emission continue (1 note/s env.), reception des echos, mesure delai\n" 
"  Avec 2 numeros de port suivis de la lettre 'E'\n"
"   - reception continue et emisson d'echo au plus tot\n" );
}

int main( int argc, char *argv[] )
{
list();

int i, idev = -1, odev = -1, echoflag = 0;
const char * iname, * oname;
const PmDeviceInfo *info;
i = 1;
if	( argc == 1 )
	usage();
while	( i < argc )
	{
	if	( isdigit(argv[i][0]) )
		{
		int dev = atoi(argv[i]);
		if	( ( dev >= 0 ) && ( dev < Pm_CountDevices() ) )
			{
			info = Pm_GetDeviceInfo( dev );
			if	( info->input )
				{ idev = dev; iname = info->name; }
			else if	( info->output )
				{ odev = dev; oname = info->name; }
			}
		}
	else	{
		if	( argv[i][0] == 'E' )
			echoflag = 1;
		}
	i++;
	}
if	( odev >= 0 )
	printf(" opening MIDI output %d : %s\n", odev, oname );
if	( idev >= 0 )
	printf(" opening MIDI input  %d : %s\n", idev, iname );
if	( ( idev < 0 ) && ( odev < 0 ) )
	{
	idev = Pm_GetDefaultInputDeviceID();
	if	( idev >= 0 )
		{
		printf(" opening MIDI default input  %d \n", idev );
		main_test_input( idev );
		}
	else	{
		printf("no input MIDI input device\n");
		Pt_Sleep(5000);
		}
	}
else if	( ( idev >= 0 ) && ( odev < 0 ) )
	main_test_input( idev );
else if	( ( idev <  0 ) && ( odev >= 0 ) )
	main_test_output( odev, idev );
else if	( ( idev >= 0 ) && ( odev >= 0 ) )
	{
	if	( echoflag )
		main_test_echo( idev, odev );
	else	main_test_output( odev, idev );
	}
return 0;
}
