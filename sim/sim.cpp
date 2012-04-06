#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// make for index scope right even on old compilers
#define for if(false);else for

struct StateEntry
{
    int     state;      // state number
    int     Lin;        // expected left in
    int     Rin;        // expected right in

    int     next_state; // next state
    int     Lout;       // new right out
    int     Rout;       // new left out

    int     newLED;     // new LED (0 no change, 1 on, 2 off)

    int     used;       // how many times this was used
} * StateTable = 0;
int stsize = 0;

class Box
{
    int     state;      // current state
    int     Lin;        // left input
    int     Rin;        // right input
    int     Lout;       // left output
    int     Rout;       // right output
    bool    LED;        // the LED
public:
    Box() : state(0), Lin(1), Rin(1), Lout(1), Rout(1), LED(false) {}

    int     getS() { return state; }
    int     getL() { return Lout; };
    int     getR() { return Rout; };
    int     getLED() { return LED; };
    void    setS(int arg) { state = arg; }
    void    setL(int arg) { Lin = arg; }
    void    setR(int arg) { Rin = arg; }
    int     setLED( bool arg ) { LED = arg; }
    int     find_rule()
    {
        for ( int i = 0; i < stsize; ++i )
        {
            if ( StateTable[i].state == state
                && (StateTable[i].Lin == Lin || StateTable[i].Lin == 0 )
                && (StateTable[i].Rin == Rin || StateTable[i].Rin == 0 ) )
            {
                return i;
            }
        }
        return -1;
    }
    bool    tick( bool update_count )      // returns true if changed, false if no change
    {
        int save_state = state;
        int save_L = Lout;
        int save_R = Rout;
        bool save_LED = LED;
        int rule = find_rule();
        if ( rule >= 0 )
        {
            state = StateTable[rule].next_state;
            if ( StateTable[rule].Rout != 0 )
                Rout = StateTable[rule].Rout;
            if ( StateTable[rule].Lout != 0 )
                Lout = StateTable[rule].Lout;
            if ( StateTable[rule].newLED != 0 )
                LED = (StateTable[rule].newLED == 1);
            if ( update_count )
                ++StateTable[rule].used;
        }
        return (save_state != state) || (save_L != Lout) || (save_R != Rout)
            || (save_LED != LED);
    }
    bool same_as( Box * bp )
    {
        return (state == bp->state) && (Lin == bp->Lin) && (Rin == bp->Rin)
            && (Lout == bp->Lout) && (Rout == bp->Rout) && (LED == bp->LED);
    }
};

bool init_state_table( const char * np );
void show_state_table( bool compact );
void step( Box * bp, int N, int time, bool show, bool verbose, bool compact );
bool in_same_state( Box * bp, Box * fbp, int N );
void fmt_state( int state, char * out );

// -N -v state_file
int
main( int argc, char *argv[] )
{
    int N = 10;
    bool verbose = false;
    bool compact_output = false;

    for ( int i = 1; i < argc; ++i )
    {
        if ( strcmp( argv[i], "-v" ) == 0 )
            verbose = true;
        else if ( argv[i][0] == '-' )
            N = atoi(argv[i]+1);
        else
            compact_output = init_state_table( argv[i] );
    }

    if ( N < 3 )
        N = 3;

    Box * bp = new Box[N];
    int time = 0;
    Box * fbp = new Box[N];
    int ftime = 0;

    if ( compact_output )
        for ( int i = 0; i < N; ++i )
        {
            bp[i].setS(1);
            fbp[i].setS(1);
        }


    int rep_start = 0;
    bool found_rep = false;

    while ( true )
    {
        if ( ! found_rep )
        {
            step( bp, N, time, true, verbose, compact_output );
            ++time;
            step( fbp, N, ftime, false, false, false );
            ++ftime;
            step( fbp, N, ftime, false, false, false );
            ++ftime;
            if ( in_same_state( bp, fbp, N ) )
            {
                for ( int i = 0; i < N; ++i )
                    fbp[i] = bp[i];
                found_rep = true;
                rep_start = time-1;
            }
        }
        else
        {
            step( bp, N, time, true, verbose, compact_output );
            ++time;
            if ( in_same_state( bp, fbp, N ) )
            {
                printf( "Repeating: time %d to %d\n", rep_start, time-1 );
                break;
            }
        }
    }
    if ( ! verbose )
        show_state_table( compact_output );
    return 0;
}

void
step( Box * bp, int N, int time, bool show, bool verbose, bool compact )
{
    // latch inputs
    bp[0].setR( bp[1].getL() );
    bp[0].setL( time ? 32 : 31 );
    for ( int i = 1; i < N-1; ++i )
    {
        bp[i].setL( bp[i-1].getR() );
        bp[i].setR( bp[i+1].getL() );
    }
    bp[N-1].setR( 30 );
    bp[N-1].setL( bp[N-2].getR() );

    if ( show )
    {
        if ( verbose )
        {
            system( "clear" );
            printf( "Time=%d\n", time );
            for ( int i = 0; i < N; ++i )
                printf( "       %c  ", bp[i].getLED() ? '*' : '-' );
            printf( "\n" );
            for ( int i = 0; i < N; ++i )
                printf( "<-%2.2d--[  ]", bp[i].getL() );
            printf( "<-30--" );
            printf( "\n" );
            for ( int i = 0; i < N; ++i )
                printf( "      <%2.2d>", bp[i].getS() );
            printf( "\n" );
            printf( "--%2.2d->", time ? 32 : 31 );
            for ( int i = 0; i < N; ++i )
                printf( "[  ]--%2.2d->", bp[i].getR() );
            printf( "\n" );
            printf( " " );
            for ( int i = 0; i < N; ++i )
            {
                int rule = bp[i].find_rule();
                if ( rule < 0 )
                    printf( "          " );
                else
                    printf( "   rule=%2.2d", rule );
            }
            printf( "\n\n" );
            show_state_table( compact );
            getchar();
        }
        else if ( compact )
        {
            printf( "%8d: %2.2d", time, time ? 32 : 31 );
            for ( int i = 0; i < N; ++i )
                if ( bp[i].getS() == 99 )
                    printf( "  *" );
                else
                    printf( " %2d", bp[i].getS() );
            printf( " 30\n" );
        }
        else
        {
            // show current state
            printf( "%8d: ", time );
            for ( int i = 0; i < N; ++i )
                printf( " %2.2d", bp[i].getS() );
            printf( "\n" );
            printf( "          " );
            for ( int i = 0; i < N; ++i )
                printf( " %c ", bp[i].getLED() ? '*' : '-' );
            printf( "\n" );
        }
    }

    // compute next state
    for ( int i = 0; i < N; ++i )
        bp[i].tick( show );
}

bool
in_same_state( Box * bp, Box * fbp, int N )
{
    for ( int i = 0; i < N; ++i )
        if ( ! bp[i].same_as( &fbp[i] ) )
            return false;
    return true;
}

bool
init_state_table( const char * np )
{
    FILE * fp = fopen( np, "r" );
    if ( fp == NULL )
    {
        fprintf( stderr, "Error: cannot open %s\n", np );
        exit(1);
    }

    char * lp = new char[4096];
    int stalloc = 16;
    stsize = 0;
    StateTable = new StateEntry[stalloc];

    bool had_short = false;
    bool had_long = false;
    while ( fgets(lp, 4096, fp) != NULL )
    {
        int s, L, R, os, oL, oR, oLED;
        char * cp = strchr( lp, '#' );
        if ( cp )
            *cp = '\0';
        if ( sscanf( lp, "%d %d %d %d %d %d %d", &s, &L, &R, &os, &oL, &oR, &oLED ) != 7 )
        {
            if ( sscanf( lp, "%d %d %d %d", &L, &s, &R, &os ) != 4 )
                continue;
            oL = oR = os;
            oLED = (os == 99) ? 1 : 2;
            had_short = true;
        }
        else
            had_long = true;
        StateEntry e;
        e.state = s;
        e.Lin = L;
        e.Rin = R;
        e.next_state = os;
        e.Lout = oL;
        e.Rout = oR;
        e.newLED = oLED;
        e.used = 0;
        if ( stsize >= stalloc )
        {
            stalloc += 16;
            StateEntry * old = StateTable;
            StateTable = new StateEntry[stalloc];
            for ( int i = 0; i < stsize; ++i )
                StateTable[i] = old[i];
            delete[] old;
        }
        StateTable[stsize++] = e;
    }
    fclose( fp );
    return had_short && ! had_long;
}

void
show_state_table( bool compact )
{
// NN: LL SS RR -> OO    used %d
    if ( compact )
    {
        for ( int i = 0; i < stsize; ++i )
        {
            if ( i && StateTable[i].state != StateTable[i-1].state )
                printf( "\n" );
            char left[8];
            char right[8];
            char cur[8];
            char nxt[8];
            fmt_state( StateTable[i].Lin, left );
            fmt_state( StateTable[i].Rin, right );
            fmt_state( StateTable[i].state, cur );
            fmt_state( StateTable[i].next_state, nxt );
            printf( "%2.2d:  %s <%s> %s  ==>  <%s>    used %d\n",
                    i, left, cur, right, nxt, StateTable[i].used );
        }
    }
    else
    {
        for ( int i = 0; i < stsize; ++i )
        {
            if ( i && StateTable[i].state != StateTable[i-1].state )
                printf( "\n" );
            printf( "%2.2d: ", i );
            if ( StateTable[i].Lin != 0 )
                printf( "--%2.2d->", StateTable[i].Lin );
            else
                printf( "      " );
            printf( "[%2.2d]", StateTable[i].state );
            if ( StateTable[i].Rin != 0 )
                printf( "<-%2.2d--", StateTable[i].Rin );
            else
                printf( "      " );

            printf( "  ==>  " );

            if ( StateTable[i].Lout != 0 )
                printf( "<-%2.2d--", StateTable[i].Lout );
            else
                printf( "      " );
            printf( "[%2.2d]", StateTable[i].next_state );
            if ( StateTable[i].Rout != 0 )
                printf( "--%2.2d->", StateTable[i].Rout );
            else
                printf( "      " );
            char led = ' ';
            if ( StateTable[i].newLED == 1 )
                led = '*';
            else if ( StateTable[i].newLED == 2 )
                led = '-';
            printf( " %c  used %d\n", led, StateTable[i].used );
        }
    }
}

void fmt_state( int state, char * out )
{
    if ( state == 0 )
        strcpy( out, "  " );
    else if (state == 99 )
        strcpy( out, "* " );
    else
        sprintf( out, "%2.2d", state );
}
