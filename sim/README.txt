PORTING

    There is one place where I use

        system("clear");

    to clear the screen.  Replace that with whatever clears the screen on
    your platform.  Or just remove it--some people might prefer to not have
    the screen cleared at that point anyway.


COMPILING

    This program just uses basic C++, so should compile easily.  I've only
    compiled it on Mac and Linux, though, and that's what the Makefile
    assumes.

USAGE

    "sim arg ..."

    The following arguments are available:

        -N      (N is a number)  Causes the simulation to include N boxes.
                NOTE: if N is less than 3, it is bumped automatically to 3.

        -v      Turn on verbose mode (see DISPLAY section below).

        file    Name of file containing the state machine specification.

STATE MACHINE SPECIFICATION

    The state machine specification file consists of lines that contain 7
    numbers.  You may include as much whitespace as you want on each line,
    and also comments, indicated by a '#'.

    Each line specifies a "condition" and an "action".  Each cycle for each
    box, the simulator scans the state machine specification looking for the
    first line whose condition matches that box, and then applies the
    specified action to that box.

    The first 3 numbers specify the condition.  They are the state number,
    and the desired left and right inputs.  A box matches if the box is in
    that state, with those inputs.

    The state number is a non-negative integer, assigned by you.  Each box
    starts in state 0, so unless you want an exceptionally boring
    simulation, you will want to at least have state 0.  The display is
    formatted assuming only 2 digits for the state number, so not going over
    99 is a good idea, too.

    The inputs can be integers from 1 to 32, or the special value 0, which
    means "don't care".

    The remaining 4 numbers are the action.  They are the next state number,
    the left and right outputs, and the LED output.

    The left and right outputs can be integers from 1 to 32, or the special
    value 0, which means leave the output unchanged.  The LED output can be
    1 to turn the LED on, 2 to turn the LED off, and 0 to leave the LED as
    it is.

    Here is an example:

        0   1   0       2   1   1   2       # line 1
        0   0   0       1   1   2   1       # line 2

        1   0   0       0   1   1   2       # line 3

        2   2   0       3   1   2   1       # line 4
        2   0   0       2   1   1   2       # line 5

        3   2   0       2   1   1   2       # line 6
        3   0   0       3   1   1   1       # line 7

    Line #1 matches any box that is in state 0 with a 1 on Lin.  That will
    be all the boxes except for the first one.  So all boxes except the
    first will execute the action on line #1, which is to go to state 2,
    output 1 on both outputs, and make sure the LED is off.

    The first box will match line #2, which matches any box in state 0 that
    hasn't already been matched by a previous line.  Its action is to go to
    state 1, output 1 on Lout and 2 on Rout, and turn the LED on.

    Line #3 takes any box in state 1 and sends it back to state 0, turning
    off the LED.  The effect of line #2 and line #3 is to make box 1 blink
    its LED on and off as fast as it can.  Box #1 is sending a stream of 2
    pulses out its right output.

    State 2 is covered by lines #4 and #5.  These two lines say that if a
    box receives a 2 on Lin, it should go to state 3, and pass the 2 along
    on Rout, and turn on its LED.  Otherwise, the box should just state in
    state 2, and make sure its LED is off.

    State 3, covered by lines #6 and #7, is similar to state 2, in that when
    it receives a 2 on Lin, to goes to state 2.  However, it turns the LED
    off rather than on, and it does NOT pass the 2 along.

    The effect of this is that the boxes other than the first bounce between
    state 2 and 3, at a rate determined by the pulses of 2 that are coming
    from the box on their left, and they are passing half those pulses
    along.

    Thus, the net result of all this is that box 1 blinks its LED as fast as
    it can, box 2 is blinking half as fast as box 1, block 3 is blinking
    half as fast as box 2, and so on.  (Note that there is a phase shift
    between adjacent boxes, which makes it so the LEDs do not form a simple
    binary counting pattern.  Can you modify the state specification to
    eliminate the phase shift?  I am not able to).

SIMPLIFIED STATE MACHINE SPECIFICATION

    Some state machine problems involve arrays of machine where the machines
    inputs are simply the states of the neighboring machines.  Such an array
    can be handled by this simulator by simply setting both outputs to the
    state number for every state.

    To save typing, you may use an alternate format for the specification file
    when simulating this kind of array.  Specify four numbers instead of seven
    on each line.  The first three numbers specify the states of consecutive
    groups of three machines, in order (left, middle, right), and the fourth
    number specifies the new state for the middle machine.

    In this mode, the LED is off, except when the machine is in state 99, in
    which case it is on.

    This file format is simply a data entry convenience.  When the simulator
    reads the line

        a b c   d

    it simply treats it as if it were this seven number line:

        b a c   d d d  x

    where x=1 if d=99, x=2 otherwise.

    In this mode, the state state for each machine is 1, not 0.

DISPLAY

    Here is the result of running the simulator using the example state
    machine from the previous section, on 3 boxes, with explanations
    inserted:

                   0:  00 00 00
                       -  -  - 
                   1:  01 02 02
                       *  -  - 

    This is the first two cycles.  The number in front of the colon is the
    time.  That's followed by the state of each box.  All boxes were in
    state 0 at time 0.  At time 1, the first box was in state 1, and the
    others were in state 2.

    The '-' or '*' below each state is the LED status.  '-' means off and
    '*' means on.

    This continues.  Note that box 1 is blinking every time, box 2 at half
    that rate, and box 3 at half box 2's rate:

                   2:  00 03 02
                       -  *  - 
                       u                   3: uu1 03 03
                           *  *  * 
                   4:  00 02 03
                       -  -  * 
                   5:  01 02 03
                       *  -  * 
                   6:  00 03 03
                       -  *  * 
                   7:  01 03 02
                       *  *  - 
                   8:  00 02 02
                       -  -  - 
                   9:  01 02 02
                       *  -  - 
                  10:  00 03 02
                       -  *  - 
                  11:  01 03 03
                       *  *  * 
                  12:  00 02 03
                       -  -  * 
                  13:  01 02 03
                       *  -  * 
                  14:  00 03 03
                       -  *  * 
                  15:  01 03 02
                       *  *  - 
            Repeating: time 7 to 15

    The simulator has noticed that the system has entered a repeating cycle:
    it was the same at time 7 as it is at time 15, and so there is no point
    in continuing.  The simulation stops.

    The simulation is only considered to be repeating if the state of each
    box matches AND the inputs to each box match AND the outputs match
    (including the LED).  Because the non-verbose display format does not
    show the inputs and does not show all the outputs, you may see lines
    that appear to be repititions that are not actually repeating.  Also, in
    the case where there is a repetition, it may take a few repetitions of
    the repeating cycle for the simulator to notice.  (It does not store all
    previous states, but rather runs two simulations in parallel, one
        running twice as fast as the other, and so catches the repition when the
    difference in time between these two simulations is a multiple of the
    period of the repitition).

    It then prints a dump of the rule table, along with a count of how many
    times each rule was invoked.  The rule table is in a format that is
    meant to be easier to read than the state description file:

            00: --01->[00]        ==>  <-01--[02]--01-> -  used 2

    The number before the colon is simply the sequential number of the rule
    in the file.  The "--01->" means the rule requires a 1 on Lin to match.
    The "[00]" is the state.  If the rule had a requirement for Rin, there
    would be something like "<-01--" after the "[00]".  The lack of such a
    thing means Rin is "don't care".

    The action is to the right of the  "==>".  The "<-01--" means that the
    action sets Lout to 1.  The "[02]" is the next state.  The "--01->"
    means that Rout is set to 1.  The "-" means LED off.

    The "used 2" means this rule was invoked twice over the course of the
    simulation.

    Compare the rest of these to the example state description, and all
    should be clear.

            01:       [00]        ==>  <-01--[01]--02-> *  used 8

            02:       [01]        ==>  <-01--[00]--01-> -  used 8

            03: --02->[02]        ==>  <-01--[03]--02-> *  used 6
            04:       [02]        ==>  <-01--[02]--01-> -  used 8

            05: --02->[03]        ==>  <-01--[02]--01-> -  used 6
            06:       [03]        ==>  <-01--[03]--01-> *  used 10

    In verbose mode, the display looks like this:

        Time=0
               -         -         -  
        <-01--[  ]<-01--[  ]<-01--[  ]<-30--
              <00>      <00>      <00>
        --31->[  ]--01->[  ]--01->[  ]--01->
            rule=01   rule=00   rule=00

        00: --01->[00]        ==>  <-01--[02]--01-> -  used 0
        01:       [00]        ==>  <-01--[01]--02-> *  used 0

        02:       [01]        ==>  <-01--[00]--01-> -  used 0

        03: --02->[02]        ==>  <-01--[03]--02-> *  used 0
        04:       [02]        ==>  <-01--[02]--01-> -  used 0

        05: --02->[03]        ==>  <-01--[02]--01-> -  used 0
        06:       [03]        ==>  <-01--[03]--01-> *  used 0

    It gives the current time, then a representation of the line of boxes,
    and then the rules.  Each box looks like this:

                   - 
            <-01--[  ]<-01--
                  <00>
            --31->[  ]--01->
                rule=01

    The "-" means the LED is off.  "*" would mean it is on.  The "<00>" is
    the current state.  The "<-01--[" is the Lout, and the "]--01->" is the
    Rout.  The "--31->[" is the Lin, and the "]<-01--" is the Rin.  The
    "rule=01" means that rule 1 matches.

    In verbose mode, the simulation stops after every cycle.  Press ENTER to
    step to the next cycle.

    As it stands in the source, it attempts to clear the screen before
    displaying the results for each cycle.  If you don't like that (or you
    cannot figure out how to reasonably do it on your platform), simply find
    the

        system("clear");

    in the source and comment it out.  (Yes, I know I should have used
    curses).

